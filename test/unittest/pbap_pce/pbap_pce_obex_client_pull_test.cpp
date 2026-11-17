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
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <vector>
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "obex_def.h"
#include "obex_header.h"
#include "obex_client_session.h"
#include "pbap_pce_obex_client.h"
#include "pbap_pce_service_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

static constexpr const char *PULL_TEST_ADDR = "11:22:33:44:55:66";
static constexpr uint32_t PULL_TEST_MTU = 0xFFFF;

static const std::string CALLLOG_CARD_1 =
    "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000001\r\n"
    "X-IRMC-CALL-DATETIME:20260911T114500\r\nEND:VCARD\r\n";
static const std::string CALLLOG_CARD_2 =
    "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000002\r\n"
    "X-IRMC-CALL-DATETIME:20260911T120000\r\nEND:VCARD\r\n";
static const std::string CONTACT_CARD_1 =
    "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Alice\r\nTEL:13800000001\r\nEND:VCARD\r\n";
static const std::string CONTACT_CARD_2 =
    "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Bob\r\nTEL:13800000002\r\nEND:VCARD\r\n";

static std::vector<uint8_t> ToBytes(const std::string &s)
{
    return std::vector<uint8_t>(s.begin(), s.end());
}

// 按 OBEX 协议追加 3 字节前缀（HIId + HIlen）的可变长度头（0x48/0x49 等）
static void AppendVarLenHeader(std::vector<uint8_t> &packet, uint8_t headerId, const std::vector<uint8_t> &data)
{
    packet.push_back(headerId);
    uint16_t len = static_cast<uint16_t>(data.size() + HEADER_PREFIX_LENGTH);
    packet.push_back(static_cast<uint8_t>(len >> 8));
    packet.push_back(static_cast<uint8_t>(len & 0xFF));
    packet.insert(packet.end(), data.begin(), data.end());
}

// 构造一条 OBEX 响应包：[respCode][pktLen(2B BE)][headers...]
static std::vector<uint8_t> MakeObexResponse(uint8_t code, const std::vector<uint8_t> &headers)
{
    std::vector<uint8_t> pkt;
    pkt.push_back(code);
    uint16_t len = static_cast<uint16_t>(headers.size() + HEADER_PREFIX_LENGTH);
    pkt.push_back(static_cast<uint8_t>(len >> 8));
    pkt.push_back(static_cast<uint8_t>(len & 0xFF));
    pkt.insert(pkt.end(), headers.begin(), headers.end());
    return pkt;
}

class PbapPceObexClientPullTest : public testing::Test {
public:
    PbapPceObexClientPullTest() {}
    ~PbapPceObexClientPullTest() {}

    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        service_ = std::make_unique<PbapPceServiceImpl>();
        serverFd_ = -1;
        clientFd_ = -1;
    }
    void TearDown() override
    {
        client_ = nullptr;
        service_ = nullptr;
        if (serverFd_ >= 0) {
            close(serverFd_);
            serverFd_ = -1;
        }
        clientFd_ = -1;
    }

    // 进程内回环：socketpair 模拟 PSE 侧 socket，注入 clientSession_，
    // 并将脚本化的 PSE 响应预写入 socket（client 按序消费，无需服务线程）。
    void SetupLoopbackClient(const std::vector<std::vector<uint8_t>> &pseResponses)
    {
        int fds[2] = {-1, -1};
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
        serverFd_ = fds[0];
        clientFd_ = fds[1];
        // RFCOMM 类型：SRM 不支持，走经典 GET 轮询续传路径（与实车 RFCOMM 场景一致）
        auto device = std::make_shared<ObexSocketDevice>(clientFd_, PULL_TEST_MTU, PULL_TEST_MTU,
            SocketType::TYPE_RFCOMM, PULL_TEST_ADDR);
        auto transport = std::make_shared<ObexServerTransport>(device);
        PbapPceObexConfig config;
        config.addr = PULL_TEST_ADDR;
        config.mtu = PULL_TEST_MTU;
        client_ = std::make_shared<PbapPceObexClient>(config, *service_);
        // 批量下载模式：跳过电源 BUSY 与状态机消息上报（单测环境无 BT 栈）
        client_->isBatchDownloading_ = true;
        client_->clientSession_ = std::make_shared<ObexClientSession>(transport, client_);
        for (const auto &resp : pseResponses) {
            ssize_t n = write(serverFd_, resp.data(), resp.size());
            ASSERT_EQ(n, static_cast<ssize_t>(resp.size()));
        }
    }

protected:
    std::unique_ptr<PbapPceServiceImpl> service_ = nullptr;
    std::shared_ptr<PbapPceObexClient> client_ = nullptr;
    int serverFd_ = -1;
    int clientFd_ = -1;
};

/*
 * @tc.number: PbapPce_PullCalllog_NoDup_00100
 * @tc.name: multi-packet GET body not duplicated
 * @tc.desc: 0x90+BODY 与 0xA0+END_OF_BODY 两包下载通话记录，
 *           body 由 ObexClientSession::Get 全权写入 writer（恰一份），返回数据必须与
 *           PSE 下发内容逐字节一致，最终包体不得二次写入（修复前 END_OF_BODY 双写）
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullCalllog_NoDup_00100, TestSize.Level1)
{
    auto part1 = ToBytes(CALLLOG_CARD_1);
    auto part2 = ToBytes(CALLLOG_CARD_2);
    std::vector<uint8_t> hdr1;
    AppendVarLenHeader(hdr1, BODY, part1);
    std::vector<uint8_t> hdr2;
    AppendVarLenHeader(hdr2, END_OF_BODY, part2);
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_CONTINUE, hdr1),
        MakeObexResponse(ResponseCodes::OBEX_HTTP_OK, hdr2),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullCalllog(PATH_MCH, 0, 100, outBodyData);
    EXPECT_EQ(ret, RET_NO_ERROR);

    std::vector<uint8_t> expected = part1;
    expected.insert(expected.end(), part2.begin(), part2.end());
    EXPECT_EQ(outBodyData.size(), expected.size());
    EXPECT_EQ(outBodyData, expected);

    auto callLogs = client_->ParseCallLogsBody(outBodyData);
    EXPECT_EQ(callLogs.size(), static_cast<size_t>(2));
}

/*
 * @tc.number: PbapPce_PullCalllog_SinglePacket_00110
 * @tc.name: single-packet GET body not duplicated
 * @tc.desc: 单包 0xA0+END_OF_BODY（小体积响应路径）：数据恰一份、条数不翻倍
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullCalllog_SinglePacket_00110, TestSize.Level1)
{
    auto all = ToBytes(CALLLOG_CARD_1 + CALLLOG_CARD_2);
    std::vector<uint8_t> hdr;
    AppendVarLenHeader(hdr, END_OF_BODY, all);
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_OK, hdr),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullCalllog(PATH_ICH, 0, 100, outBodyData);
    EXPECT_EQ(ret, RET_NO_ERROR);

    EXPECT_EQ(outBodyData.size(), all.size());
    EXPECT_EQ(outBodyData, all);

    auto callLogs = client_->ParseCallLogsBody(outBodyData);
    EXPECT_EQ(callLogs.size(), static_cast<size_t>(2));
}

/*
 * @tc.number: PbapPce_PullCalllog_EmptyEob_00120
 * @tc.name: empty END_OF_BODY yields BODY-only data
 * @tc.desc: 末包 END_OF_BODY 长度为 0：仅保留中间包 BODY 数据，不丢包、不重复
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullCalllog_EmptyEob_00120, TestSize.Level1)
{
    auto part1 = ToBytes(CALLLOG_CARD_1);
    std::vector<uint8_t> hdr1;
    AppendVarLenHeader(hdr1, BODY, part1);
    std::vector<uint8_t> hdr2;
    AppendVarLenHeader(hdr2, END_OF_BODY, {});
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_CONTINUE, hdr1),
        MakeObexResponse(ResponseCodes::OBEX_HTTP_OK, hdr2),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullCalllog(PATH_OCH, 0, 100, outBodyData);
    EXPECT_EQ(ret, RET_NO_ERROR);

    EXPECT_EQ(outBodyData.size(), part1.size());
    EXPECT_EQ(outBodyData, part1);

    auto callLogs = client_->ParseCallLogsBody(outBodyData);
    EXPECT_EQ(callLogs.size(), static_cast<size_t>(1));
}

/*
 * @tc.number: PbapPce_PullCalllog_Fail_00200
 * @tc.name: GET failure returns error and empty body
 * @tc.desc: PSE 返回 0xC3 Forbidden：PullCalllog 返回错误码，不产出 body
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullCalllog_Fail_00200, TestSize.Level1)
{
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_FORBIDDEN, {}),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullCalllog(PATH_MCH, 0, 100, outBodyData);
    EXPECT_NE(ret, RET_NO_ERROR);
    EXPECT_TRUE(outBodyData.empty());
}

/*
 * @tc.number: PbapPce_PullCalllog_NoSession_00210
 * @tc.name: PullCalllog without client session
 * @tc.desc: 会话未建立（clientSession_ 为空）：返回 RET_BAD_STATUS，不产出 body
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullCalllog_NoSession_00210, TestSize.Level1)
{
    PbapPceObexConfig config;
    config.addr = PULL_TEST_ADDR;
    config.mtu = PULL_TEST_MTU;
    auto client = std::make_shared<PbapPceObexClient>(config, *service_);

    std::vector<uint8_t> outBodyData;
    int ret = client->PullCalllog(PATH_MCH, 0, 100, outBodyData);
    EXPECT_EQ(ret, RET_BAD_STATUS);
    EXPECT_TRUE(outBodyData.empty());
}

/*
 * @tc.number: PbapPce_PullPhoneBookBatch_NoDup_00300
 * @tc.name: PullPhoneBookBatch body not duplicated
 * @tc.desc: PullPhoneBookBatch：多包下载 body 恰一份，
 *           最终包（END_OF_BODY）不得因二次提取而双写
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullPhoneBookBatch_NoDup_00300, TestSize.Level1)
{
    auto part1 = ToBytes(CONTACT_CARD_1);
    auto part2 = ToBytes(CONTACT_CARD_2);
    std::vector<uint8_t> hdr1;
    AppendVarLenHeader(hdr1, BODY, part1);
    std::vector<uint8_t> hdr2;
    AppendVarLenHeader(hdr2, END_OF_BODY, part2);
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_CONTINUE, hdr1),
        MakeObexResponse(ResponseCodes::OBEX_HTTP_OK, hdr2),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullPhoneBookBatch(PATH_PB, 500, 500, false, outBodyData);
    EXPECT_EQ(ret, RET_NO_ERROR);

    std::vector<uint8_t> expected = part1;
    expected.insert(expected.end(), part2.begin(), part2.end());
    EXPECT_EQ(outBodyData.size(), expected.size());
    EXPECT_EQ(outBodyData, expected);

    auto contacts = client_->ParseContactsBody(outBodyData);
    EXPECT_EQ(contacts.size(), static_cast<size_t>(2));
}

/*
 * @tc.number: PbapPce_PullCalllog_IchMissed_00130
 * @tc.name: ICH mixed MISSED entries filtered before insert
 * @tc.desc: 部分手机 ICH 响应混入 MISSED 条目：Pull -> Parse -> Filter 链路后
 *           入库集仅剩 RECEIVED 条目（MISSED 由后续 MCH 拉取补齐，不丢记录）
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullCalllog_IchMissed_00130, TestSize.Level1)
{
    static const std::string ICH_CARD_RECEIVED =
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000001\r\n"
        "X-IRMC-CALL-DATETIME;RECEIVED:20260911T114500\r\nEND:VCARD\r\n";
    static const std::string ICH_CARD_MISSED =
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000002\r\n"
        "X-IRMC-CALL-DATETIME;MISSED:20260911T120000\r\nEND:VCARD\r\n";

    auto all = ToBytes(ICH_CARD_RECEIVED + ICH_CARD_MISSED);
    std::vector<uint8_t> hdr;
    AppendVarLenHeader(hdr, END_OF_BODY, all);
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_OK, hdr),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullCalllog(PATH_ICH, 0, 100, outBodyData);
    EXPECT_EQ(ret, RET_NO_ERROR);

    auto callLogs = client_->ParseCallLogsBody(outBodyData);
    ASSERT_EQ(callLogs.size(), static_cast<size_t>(2));

    size_t missedDropped = 0;
    auto insertSet = PbapPceObexClient::FilterCallLogsForInsert(callLogs, PATH_ICH, missedDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(1));
    EXPECT_EQ(missedDropped, static_cast<size_t>(1));
    ASSERT_EQ(insertSet[0].phones.size(), static_cast<size_t>(1));
    EXPECT_EQ(insertSet[0].phones[0], "13800000001");
}

// 在原始 OBEX 请求字节中定位应用参数头（0x4C）并提取指定 tag 的 2 字节值（大端）
static bool FindAppParamValue(const std::vector<uint8_t> &req, uint8_t tag, uint16_t &value)
{
    for (size_t i = 0; i + HEADER_PREFIX_LENGTH <= req.size(); i++) {
        if (req[i] != APPLICATION_PARAMETER) {
            continue;
        }
        size_t hdrLen = (static_cast<size_t>(req[i + 1]) << 8) | req[i + 2];
        if (hdrLen < HEADER_PREFIX_LENGTH || i + hdrLen > req.size()) {
            continue;
        }
        size_t end = i + hdrLen;
        size_t p = i + HEADER_PREFIX_LENGTH;
        while (p + 2 <= end) {
            uint8_t t = req[p];
            uint8_t len = req[p + 1];
            if (p + 2 + len > end) {
                break;
            }
            if (t == tag && len == 2) {
                value = (static_cast<uint16_t>(req[p + 2]) << 8) | req[p + 3];
                return true;
            }
            p += 2 + len;
        }
    }
    return false;
}

/*
 * @tc.number: PbapPce_PullPhoneBookBatch_FirstBatchReq_00400
 * @tc.name: first batch GET carries ListStartOffset=1 and MaxListCount=size-1
 * @tc.desc: 请求级名片跳过（PBAP v1.2.3 §3.1.5）：位置 0 为 0.vcf 名片槽位不请求，
 *           首批 GET 应用参数必须携带 MaxListCount=size-1 与 ListStartOffset=1。
 *           从回环 socket 读出客户端实际发出的请求字节，校验应用参数编码真实上线
 */
HWTEST_F(PbapPceObexClientPullTest, PbapPce_PullPhoneBookBatch_FirstBatchReq_00400, TestSize.Level1)
{
    auto body = ToBytes(CONTACT_CARD_1);
    std::vector<uint8_t> hdr;
    AppendVarLenHeader(hdr, END_OF_BODY, body);
    SetupLoopbackClient({
        MakeObexResponse(ResponseCodes::OBEX_HTTP_OK, hdr),
    });
    ASSERT_TRUE(client_ != nullptr);

    std::vector<uint8_t> outBodyData;
    int ret = client_->PullPhoneBookBatch(PATH_PB, 1, 8, false, outBodyData);
    EXPECT_EQ(ret, RET_NO_ERROR);

    // 读出客户端实际发出的 OBEX GET 请求字节（socketpair 回环另一端）
    std::vector<uint8_t> req(2048);
    ssize_t n = read(serverFd_, req.data(), req.size());
    ASSERT_GT(n, static_cast<ssize_t>(0));
    req.resize(static_cast<size_t>(n));

    uint16_t maxListCount = 0;
    ASSERT_TRUE(FindAppParamValue(req, PbapPceAppParams::MAX_LIST_COUNT, maxListCount));
    EXPECT_EQ(maxListCount, static_cast<uint16_t>(8));
    uint16_t listStartOffset = 0;
    ASSERT_TRUE(FindAppParamValue(req, PbapPceAppParams::LIST_START_OFFSET, listStartOffset));
    EXPECT_EQ(listStartOffset, static_cast<uint16_t>(1));
}

}  // namespace bluetooth
}  // namespace OHOS
