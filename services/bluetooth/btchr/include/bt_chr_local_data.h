/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_LOCAL_DATA_H
#define BT_CHR_LOCAL_DATA_H

#include <cstdint>
#include <mutex>
#include <string>

namespace OHOS {
namespace bluetooth {
/**
 * Local device data channel used by CHR events. Currently it caches the local
 * country information (ISO country code, MCC and MNC) and refreshes the cache
 * when the SIM or network state changes.
 *
 * Telephony types are only used in the source file, the observer registration
 * state is guarded by BLUETOOTH_TELEPHONY_EXIST there, so this header has no
 * dependency on the telephony subsystem.
 */
class BtChrLocalData {
public:
    static BtChrLocalData &GetInstance();

    // 获取本端运营商信息合并字符串，格式: "ISO国家码_MCC_MNC"
    // 示例：中国移动 "CN_460_00"、中国电信 "CN_460_11"、美国 T-Mobile "US_310_260"
    // useCache=true 读缓存（懒加载，失败 30s 冷却）；false 实时获取并刷新缓存，失败回退缓存
    std::string GetCountryInfo(bool useCache = true);

    // 主动刷新缓存，供状态变化回调调用
    void RefreshCountryInfo();

    // 设置服务运行状态（Start 置 true，Stop 置 false），注册与取数的同源判据
    static void SetServiceRunning(bool running);

    // 服务启动：置运行态并订阅 SIM/网络状态变化。生命周期由蓝牙服务进程驱动，
    // 不跟随蓝牙开关，保证开关切换全程可取到国家码。
    void Start();

    // 服务停止：置停服态并取消订阅、复位缓存
    void Stop();

    // 订阅 SIM/网络状态变化，变化时自动刷新缓存（Start 时调用）
    // 返回 0（TELEPHONY_SUCCESS）表示订阅已建立；正值透传 telephony 错误码；
    // -1（REGISTER_RET_NOT_ATTEMPTED）表示本次未发起注册（停服/节流/未编译 telephony 依赖）
    int32_t RegisterCountryInfoObserver();

    // 取消订阅并复位缓存（Stop 时调用）
    void UnregisterCountryInfoObserver();

private:
    BtChrLocalData() = default;
    ~BtChrLocalData() = default;
    BtChrLocalData(const BtChrLocalData &) = delete;
    BtChrLocalData &operator=(const BtChrLocalData &) = delete;

    // 复位国家码缓存
    void ResetCountryInfo();

    // 实时获取并合并本端运营商信息；调用方不得持有 countryInfoMutex_
    bool FetchCountryInfo(std::string &countryInfo);

    static int64_t GetCurrentTimeMs();

    // 保护 countryInfo_ 及 isLoaded_/时间戳/gen_/refreshSeq_/lastRefreshCommitSeq_
    std::mutex countryInfoMutex_;
    std::string countryInfo_ = "";
    bool isLoaded_ = false;
    int64_t lastFetchFailTime_ = 0; // 上报路径取数失败时间戳（0 表示未发生）
    int64_t lastRefreshFailTime_ = 0; // 回调路径取数失败时间戳（0 表示未发生）
    uint64_t gen_ = 0; // 复位代际，停服复位后拒绝在飞取数的陈旧提交
    uint64_t refreshSeq_ = 0; // 回调取数起飞序号，单调递增
    uint64_t lastRefreshCommitSeq_ = 0; // 回调取数已提交的最高序号，只允许更新的结果提交
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BT_CHR_LOCAL_DATA_H
