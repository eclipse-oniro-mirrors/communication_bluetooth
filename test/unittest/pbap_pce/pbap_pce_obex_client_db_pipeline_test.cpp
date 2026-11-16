/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "bluetooth_errorcode.h"
#include "pbap_pce_datashare_helper.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_obex_client.h"
#include "pbap_pce_service_impl.h"
#include "thread_util.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

constexpr int DB_PIPELINE_POLL_INTERVAL_MS = 20;
constexpr int DB_PIPELINE_TASK_DURATION_MS = 100;
constexpr int DB_PIPELINE_WAIT_TIMEOUT_MS = 5000;
constexpr int DB_PIPELINE_TASK_NUM = 8;

class PbapPceObexClientDbPipelineTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // 本套例验证 DB 入库串行队列的真实 FIFO/背压语义，须走真实 ffrt 队列
        // （NOT_SWITCH_THREAD 会改为调用线程内联执行，背压永远无法触发）
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_LOW_PRIORITY, ThreadUtil::ENABLED);
    }

    static void TearDownTestCase() {}

    void SetUp()
    {
        serviceImpl_ = std::make_unique<PbapPceServiceImpl>();
        PbapPceObexConfig config;
        config.addr = "00:11:22:33:44:55";
        client_ = std::make_shared<PbapPceObexClient>(config, *serviceImpl_);
    }

    void TearDown()
    {
        // 防御性 drain：等待在飞任务清零（超时不阻塞用例结束）
        WaitFor([this]() { return client_->inFlightDbTasks_.load() == 0; }, DB_PIPELINE_WAIT_TIMEOUT_MS);
        client_->inFlightDbTasks_ = 0;
        client_ = nullptr;
        serviceImpl_ = nullptr;
    }

    static bool WaitFor(const std::function<bool()> &pred, int timeoutMs)
    {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            if (pred()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(DB_PIPELINE_POLL_INTERVAL_MS));
        }
        return pred();
    }

    std::unique_ptr<PbapPceServiceImpl> serviceImpl_ = nullptr;
    std::shared_ptr<PbapPceObexClient> client_ = nullptr;
};

/*
 * @tc.number: Pbap_Pce_ObexClient_DbPipeline_00100
 * @tc.name: DispatchDbInsert blocks under backpressure instead of inline execution
 * @tc.desc: 在飞任务达到上限时，DispatchDbInsert 不得在调用线程同步执行（回归
 *           验证：同步插队会破坏入库 FIFO 顺序，导致 och 批次被 isFinish 清库误删），
 *           应阻塞等待空位后入队，且任务恰好执行一次。
 */
HWTEST_F(PbapPceObexClientDbPipelineTest, Pbap_Pce_ObexClient_DbPipeline_00100, TestSize.Level1)
{
    // 手动注满在飞额度，模拟 4 个慢任务正在串行队列中执行
    client_->inFlightDbTasks_ = PBAP_PCE_MAX_INFLIGHT_DB_TASKS;

    std::atomic<int> executed {0};
    std::thread dispatcher([this, &executed]() {
        client_->DispatchDbInsert([&executed]() { executed++; });
    });

    // 旧实现（同步插队）会在 150ms 内直接执行任务；新实现应阻塞等待
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_EQ(executed.load(), 0);

    // 释放一个额度，任务应被唤醒并入队执行
    {
        std::lock_guard<std::mutex> lock(client_->dbDrainMutex_);
        client_->inFlightDbTasks_--;
    }
    client_->dbDrainCv_.notify_all();

    dispatcher.join();
    EXPECT_TRUE(WaitFor([&executed]() { return executed.load() == 1; }, DB_PIPELINE_WAIT_TIMEOUT_MS));
    EXPECT_EQ(executed.load(), 1);

    // 任务收尾 fetch_sub 后计数应为 4-1=3（3 为注入的幻影额度，用例结束前清除）
    EXPECT_TRUE(WaitFor([this]() {
        return client_->inFlightDbTasks_.load() == PBAP_PCE_MAX_INFLIGHT_DB_TASKS - 1;
    }, DB_PIPELINE_WAIT_TIMEOUT_MS));
    client_->inFlightDbTasks_ = 0;
}

/*
 * @tc.number: Pbap_Pce_ObexClient_DbPipeline_00200
 * @tc.name: DispatchDbInsert keeps strict FIFO order under natural backpressure
 * @tc.desc: 连续投递 8 个慢任务（超过在飞上限 4），自然触发背压；投递线程阻塞等待
 *           空位而非插队，所有任务在串行队列上按投递顺序执行，最终计数归零。
 *           旧实现下第 5 个任务起会同步插队执行，顺序断言必然失败。该用例同时
 *           回归历史缺陷"batchId 不按顺序入库导致插入失败"（末批 isFinish=true
 *           插队先于低 batchId 的 isFinish=false 批次到达 contactsdataability，
 *           服务端会话提前闭合/校验拒绝）：batchId 分配序 == 派发序，本用例断言
 *           执行序 == 派发序，故 batchId 严格递增、isFinish=true 恒最后到达。
 */
HWTEST_F(PbapPceObexClientDbPipelineTest, Pbap_Pce_ObexClient_DbPipeline_00200, TestSize.Level1)
{
    std::mutex orderMutex;
    std::vector<int> order;
    std::atomic<int> done {0};

    for (int i = 0; i < DB_PIPELINE_TASK_NUM; i++) {
        client_->DispatchDbInsert([i, &order, &orderMutex, &done]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(DB_PIPELINE_TASK_DURATION_MS));
            {
                std::lock_guard<std::mutex> lock(orderMutex);
                order.push_back(i);
            }
            done++;
        });
    }

    EXPECT_TRUE(WaitFor([&done]() { return done.load() == DB_PIPELINE_TASK_NUM; },
        DB_PIPELINE_WAIT_TIMEOUT_MS));

    // 执行顺序 == 投递顺序 == batchId 分配顺序（顺序契约）
    ASSERT_EQ(order.size(), static_cast<size_t>(DB_PIPELINE_TASK_NUM));
    for (size_t k = 0; k < order.size(); k++) {
        EXPECT_EQ(order[k], static_cast<int>(k));
    }
    EXPECT_TRUE(WaitFor([this]() { return client_->inFlightDbTasks_.load() == 0; },
        DB_PIPELINE_WAIT_TIMEOUT_MS));
}

/*
 * @tc.number: Pbap_Pce_ObexClient_DbPipeline_00300
 * @tc.name: AllocDbBatchId sub-batch numbering semantics
 * @tc.desc: 空集占 1 个号（totalSize==0 的 commit-only 批次依赖该语义）；按 250 条
 *           子批数跳号；序号单调递增且不重叠。
 */
HWTEST_F(PbapPceObexClientDbPipelineTest, Pbap_Pce_ObexClient_DbPipeline_00300, TestSize.Level1)
{
    EXPECT_EQ(client_->AllocDbBatchId(0), 0u);    // 空集占 1 号
    EXPECT_EQ(client_->AllocDbBatchId(600), 1u);  // 600 条 = 3 子批，基号跳 3
    EXPECT_EQ(client_->AllocDbBatchId(0), 4u);
    EXPECT_EQ(client_->AllocDbBatchId(251), 5u);  // 251 条 = 2 子批
    EXPECT_EQ(client_->AllocDbBatchId(250), 7u);  // 恰好 1 子批
    EXPECT_EQ(client_->nextDbBatchId_, 8u);
}

/*
 * @tc.number: Pbap_Pce_ObexClient_DbPipeline_00400
 * @tc.name: InsertContactsToDb skips empty set without finish
 * @tc.desc: 空集 + isFinish=false 直接跳过返回 RET_NO_ERROR，不触碰 DataShare
 *           （dataShareHelper_ 为 null 时不应崩溃）。
 */
HWTEST_F(PbapPceObexClientDbPipelineTest, Pbap_Pce_ObexClient_DbPipeline_00400, TestSize.Level1)
{
    EXPECT_EQ(client_->dataShareHelper_, nullptr);
    int ret = client_->InsertContactsToDb({}, false, "trace-unittest", 0);
    EXPECT_EQ(ret, RET_NO_ERROR);
}

/*
 * @tc.number: Pbap_Pce_ObexClient_DbPipeline_00500
 * @tc.name: BatchInsertContacts early return for empty set without finish
 * @tc.desc: 空集 + isFinish=false 在 helper 层早退返回 0，不创建 DataShare helper
 *           （无需 DataShare 运行环境）。空集 + isFinish=true 的 commit-only 下发
 *           依赖真实 DataShare 环境，由真机验证覆盖。
 */
HWTEST_F(PbapPceObexClientDbPipelineTest, Pbap_Pce_ObexClient_DbPipeline_00500, TestSize.Level1)
{
    PbapPceDataShareHelper helper;
    int32_t ret = helper.BatchInsertContacts({}, false, "trace-unittest", 0);
    EXPECT_EQ(ret, 0);
}

}  // namespace bluetooth
}  // namespace OHOS
