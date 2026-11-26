/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef REFUSE_PLAY_HELPER_H
#define REFUSE_PLAY_HELPER_H

#include "safe_map.h"
#include "safe_vector.h"

namespace OHOS {
namespace bluetooth {
constexpr int AUTO_PLAY_UNKNOWN = -1;
constexpr int AUTO_PLAY_NOT_ALLLOW = 0;
constexpr int AUTO_PLAY_ALLLOW = 1;
constexpr int DEFAULT_VALUE = -1;
constexpr int64_t DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS = 6 * 1000;
constexpr int64_t MAX_ADAPTIVE_AUTOPLAY_DURATION_MS = 12 * 1000;
typedef struct {
    /** 存储最后一次AVRCP连接的时间 */
    int64_t lastAvrcpConnectTime = DEFAULT_VALUE;
    /** 存储最后一次挂断电话的时间 */
    int64_t lastHangUpTime = DEFAULT_VALUE;
    /** 存储上一次暂停的时间 */
    int64_t lastPauseTime = DEFAULT_VALUE;
    /** 存储本次AVRCP连接锁存的自动播放拦截窗口 */
    int64_t windowDuration = DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS;
    /** 存储本轮 PLAY 候选的最大时间偏移，等待用户暂停确认 */
    int64_t pendingPlayOffset = DEFAULT_VALUE;
    /** 存储本轮最近一次 PLAY 的时间，用于判断暂停是否在 PLAY 之后 */
    int64_t pendingPlayTime = DEFAULT_VALUE;

    void ResetPendingPlay()
    {
        pendingPlayOffset = DEFAULT_VALUE;
        pendingPlayTime = DEFAULT_VALUE;
    }
} TimeRecord;

class RefusePlayHelper {
public:
    static RefusePlayHelper* GetInstance();
    /**
     * 设置最后一次AVRCP连接的时间
     */
    void SetLastAvrcpConnectTime(const std::string &device);
    /**
     * 获取最后一次AVRCP连接的时间
     */
    int64_t GetLastAvrcpConnectTime(const std::string &device);
    /**
     * play指令拦截策略
     */
    bool IsRefusePlayAfterConnect(const std::string &device);
    /**
     * 是否需要发送play指令
     */
    bool IsNeedToSendPlay(const std::string &device);
    /**
     * 获取设备当前实际生效的自动播放拦截时长
     */
    int GetEffectiveAutoPlayDuration(const std::string &device);
    /**
     * 记录本次连接PLAY命令的时间偏移，满足条件时记录学习候选
     */
    void RecordPlayOffset(const std::string &device);

    void ClearRecord(const std::string &device);
    void ClearAuthingDevice(const std::string &device);
    bool IsShowCapsuleAfterHangUp(const std::string &device);
    int64_t GetLastHangUpTime(const std::string &device);
    void SetLastHangUpTime(const std::string &device);
    bool NeedIgnoreSetActive();
    bool NeedDropDueScoEnd(const std::string &device, bool isPushed);
    void ProcessPushed(const std::string &device, bool isPushed);
    bool IsPauseAllowedByWhiteList();
    void SavePauseTime(const std::string &device);
    void JudgeScoPause(const std::string &device);
    /**
     * 本机播放状态暂停确认学习
     */
    void HandlePauseEvent(const std::string &device);
private:
    bool IsAutoPlay(const std::string &device, int64_t currentTime);
    /**
     * 用户暂停后确认是否学习候选时长
     */
    int64_t TryLearnFromUserPause(const std::string &device, TimeRecord &timeRecord, int64_t pauseTime);
    /**
     * 校验暂停候选，成功时保存该车学习窗口
     */
    void TryLearnAndSave(const std::string &device, TimeRecord &timeRecord, int64_t pauseTime);
    /**
     * 上报自动播放拦截CHR，携带本次连接锁存的拦截窗口
     */
    void WriteAutoPlayRefuseUe(const std::string &device);
    // 时间记录
    SafeMap<std::string, TimeRecord> mTimeRecordMap_;
    // 已弹窗等待用户授权的设备
    SafeMap<std::string, int64_t> mWatingForAuthDevices_;
    SafeMap<std::string, bool> mKeyPushedMap_;
    SafeMap<std::string, bool> mScoPauseMap_;
};
} // namespace bluetooth
} // namespace OHOS
#endif