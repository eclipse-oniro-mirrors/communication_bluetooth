/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_play_helper"
#endif

#include <algorithm>

#include "refuse_play_helper.h"

#include "adapter_device_config.h"
#include "a2dp_service.h"
#include "avrcp_tg_service_manager.h"
#include "avrc_defs.h"
#include "bluetooth_dialog.h"
#include "bt_chr_ue_manager.h"
#include "common_util.h"
#include "log.h"
#include "raw_address.h"
#include "remote_device_properties.h"

namespace OHOS {
namespace bluetooth {
constexpr int64_t RESUME_PLAY_MAX_WAIT_TIME_MS = 30 * 1000;
constexpr int64_t DROP_KEY_WITHIN_SCO_END_INTERVAL_MS = 6000;
constexpr int64_t SCO_BEGIN_PAUSE_TIME = 2500; /* ms, between sco connectted and last avrcp pause time 2.5s */
constexpr int64_t AFTER_HANG_UP_SHOW_CAPSULE_TIME_DIFF_MS = 1500;
const std::list<std::string> NOT_DROP_PAUSE_AFTER_SCO_DEVICES{ "BT_CITROEN", "BT_PEUGEOT" };
const std::string CONTINENTAL_ADDRESS = "00:54:AF"; // CITROEN PEUGEOT

RefusePlayHelper* RefusePlayHelper::GetInstance()
{
    static RefusePlayHelper instance;
    return &instance;
}

int RefusePlayHelper::GetEffectiveAutoPlayDuration(const std::string &device)
{
    int learnedDuration = GetLearnedAutoPlayDuration(device);
    // 将时间窗限制为6-12s.
    int64_t windowDuration = std::max<int64_t>(learnedDuration, DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS);
    windowDuration = std::min<int64_t>(windowDuration, MAX_ADAPTIVE_AUTOPLAY_DURATION_MS);
    HILOGD("learnedDuration: %{public}d, windowDuration: %{public}lld, device: %{public}s",
        learnedDuration, windowDuration, GET_ENCRYPT_STR_ADDR(device));
    return static_cast<int>(windowDuration);
}

void RefusePlayHelper::SetLastAvrcpConnectTime(const std::string &device)
{
    int64_t currentTime = GetTimeStamp();
    int windowDuration = GetEffectiveAutoPlayDuration(device);
    HILOGI("currentTime: %{public}lld, windowDuration: %{public}d, device: %{public}s",
        currentTime, windowDuration, GET_ENCRYPT_STR_ADDR(device));

    TimeRecord timeRecord;
    if (mTimeRecordMap_.Find(device, timeRecord)) {
        timeRecord.lastAvrcpConnectTime = currentTime;
        timeRecord.windowDuration = windowDuration;
        timeRecord.pendingPlayTime = DEFAULT_VALUE;
        timeRecord.pendingPlayOffset = DEFAULT_VALUE;
        mTimeRecordMap_.EnsureInsert(device, timeRecord);
    } else {
        timeRecord.lastAvrcpConnectTime = currentTime;
        timeRecord.windowDuration = windowDuration;
        timeRecord.pendingPlayTime = DEFAULT_VALUE;
        timeRecord.pendingPlayOffset = DEFAULT_VALUE;
        mTimeRecordMap_.Insert(device, timeRecord);
    }
}

int64_t RefusePlayHelper::GetLastAvrcpConnectTime(const std::string &device)
{
    TimeRecord timeRecord;
    if (!mTimeRecordMap_.Find(device, timeRecord)) {
        return DEFAULT_VALUE;
    }
    return timeRecord.lastAvrcpConnectTime;
}

bool RefusePlayHelper::IsAutoPlay(const std::string &device, int64_t currentTime)
{
    TimeRecord timeRecord;
    if (!mTimeRecordMap_.Find(device, timeRecord)) {
        HILOGW("no avrcp connect record");
        return false;
    }
    int64_t lastAvrcpConnectTime = timeRecord.lastAvrcpConnectTime;
    if (lastAvrcpConnectTime == DEFAULT_VALUE) {
        HILOGW("last connect time is default time");
        return false;
    }
    if (currentTime < lastAvrcpConnectTime) {
        HILOGW("time error");
        return false;
    }
    int duration = DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS;
    GetRestrictedDuration(device, duration);
    long timeDiff = currentTime - lastAvrcpConnectTime;
    HILOGI("timeDiff: %{public}ld, windowDuration: %{public}lld", timeDiff, timeRecord.windowDuration);
    // 超过本次连接锁存的窗口，自动播放
    if (timeDiff > timeRecord.windowDuration) {
        return false;
    }
    return true;
}

void RefusePlayHelper::WriteAutoPlayRefuseUe(const std::string &device)
{
    TimeRecord timeRecord;
    int windowDuration = 0;
    if (mTimeRecordMap_.Find(device, timeRecord)) {
        windowDuration = static_cast<int>(timeRecord.windowDuration);
    }
    BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, RawAddress(device), AVRC_ID_PLAY,
        UE_COMMON_SCENE_CASE6, std::to_string(windowDuration));
}

bool RefusePlayHelper::IsRefusePlayAfterConnect(const std::string &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_STR_ADDR(device));

    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(device));
    if (remoteDevice == nullptr) {
        return false;
    }
    // 音箱不拦截
    if (remoteDevice->GetIoCapability() == GAP_IO_NOINPUTNOOUTPUT) {
        HILOGW("allow: Io cap is none");
        return false;
    }

    int64_t currentTime = GetTimeStamp();
    if (!IsAutoPlay(device, currentTime)) {
        HILOGI("allow: is not auto play");
        return false;
    }

    int allowValue = AUTO_PLAY_UNKNOWN;
    GetAutoPlayValue(device, allowValue);
    HILOGI("allowValue: %{public}d", allowValue);
    // 判断用户选择记录
    if (allowValue == AUTO_PLAY_ALLLOW) {
        return false;
    } else if (allowValue == AUTO_PLAY_NOT_ALLLOW) {
        WriteAutoPlayRefuseUe(device);
        return true;
    } else if (allowValue == AUTO_PLAY_UNKNOWN && !mWatingForAuthDevices_.Find(device, currentTime)) {
        DialogInfo dialog{device, AUTO_PLAY_AUTH_DIALOG, 0, nullptr};
        BluetoothDialog::RequestAuthDialog(dialog);
        mWatingForAuthDevices_.Insert(device, currentTime);
    } else {
        HILOGE("not auth device");
    }

    return true;
}

bool RefusePlayHelper::IsNeedToSendPlay(const std::string &device)
{
    int64_t authStartTime;
    if (mWatingForAuthDevices_.Find(device, authStartTime)) {
        mWatingForAuthDevices_.Erase(device);
        A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG_RET(service != nullptr, false, "a2dp service nullptr");
        int64_t currentTime = GetTimeStamp();
#ifdef MULTI_A2DP_ENABLED
        bool isConnectedPeer = false;
        for (const auto &peer : service->GetActivePeers()) {
            if (peer.GetAddress() == device) {
                isConnectedPeer = true;
                break;
            }
        }
        if (isConnectedPeer && currentTime - authStartTime < RESUME_PLAY_MAX_WAIT_TIME_MS) {
            HILOGI("after user auth, auto play");
            return true;
        }
#else
        if (device == service->GetActiveSinkDevice().GetAddress()
            && currentTime - authStartTime < RESUME_PLAY_MAX_WAIT_TIME_MS) {
            HILOGI("after user auth, auto play");
            return true;
        }
#endif
    }
    return false;
}

// 记录本次 AVRCP 连接中每一轮 PLAY 的候选：同一轮取最大值，暂停后由下一轮 PLAY 重新建立。
void RefusePlayHelper::RecordPlayOffset(const std::string &device)
{
    BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, RawAddress(device), AVRC_ID_PLAY,
        UE_COMMON_SCENE_CASE1);
    TimeRecord timeRecord;
    int64_t currentTime = GetTimeStamp();
    // 无连接记录或连接时间无效时不处理
    if (!mTimeRecordMap_.Find(device, timeRecord) ||
        timeRecord.lastAvrcpConnectTime == DEFAULT_VALUE || currentTime < timeRecord.lastAvrcpConnectTime) {
        return;
    }

    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(RawAddress(device));
    if (remoteDevice == nullptr) {
        HILOGI("skip learn play duration, device not found");
        return;
    }

    // 用户已允许自动播放时不学习
    int allowValue = AUTO_PLAY_UNKNOWN;
    GetAutoPlayValue(device, allowValue);
    if (allowValue != AUTO_PLAY_NOT_ALLLOW) {
        HILOGI("skip learn play duration, allowValue: %{public}d", allowValue);
        return;
    }

    // 只接受 6s~12s 的候选，且必须大于当前窗口，避免把窗口学小
    int64_t offset = currentTime - timeRecord.lastAvrcpConnectTime;
    if (offset < DEFAULT_CONNECT_AUTOPLAY_MAX_TIMEDIFF_MS || offset > MAX_ADAPTIVE_AUTOPLAY_DURATION_MS) {
        HILOGI("skip learn play duration, offset: %{public}lld", offset);
        return;
    }
    int64_t effectiveLearned = GetEffectiveAutoPlayDuration(device);
    if (offset <= effectiveLearned) {
        HILOGI("skip learn play duration, offset: %{public}lld, learned: %{public}lld", offset, effectiveLearned);
        return;
    }

    timeRecord.pendingPlayTime = currentTime;
    if (offset > timeRecord.pendingPlayOffset) {
        timeRecord.pendingPlayOffset = offset;
    }

    mTimeRecordMap_.EnsureInsert(device, timeRecord);
    HILOGI("pending learn auto play duration: offset: %{public}lld, allowValue: %{public}d, device: %{public}s",
        timeRecord.pendingPlayOffset, allowValue, GET_ENCRYPT_STR_ADDR(device));
}

// 校验暂停事件是否可以确认本轮 PLAY 候选：成功则返回更新后的拦截时间窗，否则返回 DEFAULT_VALUE。
// 每次暂停后清空本轮候选，由下一次 PLAY 重新建立；连接结束由 ClearRecord 清理。
int64_t RefusePlayHelper::TryLearnFromUserPause(const std::string &device, TimeRecord &timeRecord, int64_t pauseTime)
{
    // 没有待确认的本轮 PLAY 候选
    if (timeRecord.pendingPlayTime == DEFAULT_VALUE || timeRecord.pendingPlayOffset == DEFAULT_VALUE) {
        return DEFAULT_VALUE;
    }

    int allowValue = AUTO_PLAY_UNKNOWN;
    GetAutoPlayValue(device, allowValue);

    if (timeRecord.lastAvrcpConnectTime == DEFAULT_VALUE || allowValue != AUTO_PLAY_NOT_ALLLOW) {
        timeRecord.ResetPendingPlay();
        return DEFAULT_VALUE;
    }

    // PAUSE确认窗从 AVRCP 连接时刻起算，持续 12s
    int64_t windowEnd = timeRecord.lastAvrcpConnectTime + MAX_ADAPTIVE_AUTOPLAY_DURATION_MS;
    bool isAfterPlay = pauseTime >= timeRecord.pendingPlayTime;
    bool isInWindow = pauseTime <= windowEnd;
    int64_t learnOffset = DEFAULT_VALUE;
    if (!isAfterPlay || !isInWindow) {
        HILOGI("skip learn after pause, pauseTime: %{public}lld, pendingPlayTime: %{public}lld, "
            "windowEnd: %{public}lld", pauseTime, timeRecord.pendingPlayTime, windowEnd);
        timeRecord.ResetPendingPlay();
        return DEFAULT_VALUE;
    }
    // 只有 T 大于该车当前窗口才学习，保证窗口只增不减
    int64_t currentWindow = GetEffectiveAutoPlayDuration(device);
    if (timeRecord.pendingPlayOffset > currentWindow) {
        learnOffset = timeRecord.pendingPlayOffset;
        HILOGI("confirm learn after pause: %{public}lld -> %{public}lld, device: %{public}s",
            currentWindow, learnOffset, GET_ENCRYPT_STR_ADDR(device));
    }
    // 本轮暂停已处理，清空候选，等待下一次 PLAY 重新建立
    timeRecord.ResetPendingPlay();
    return learnOffset;
}

void RefusePlayHelper::ClearRecord(const std::string &device)
{
    mTimeRecordMap_.Erase(device);
    mWatingForAuthDevices_.Erase(device);
    mKeyPushedMap_.Erase(device);
    mScoPauseMap_.Erase(device);
    HILOGI("ClearRecord, device: %{public}s", GET_ENCRYPT_STR_ADDR(device));
}

void RefusePlayHelper::ClearAuthingDevice(const std::string &device)
{
    mWatingForAuthDevices_.Erase(device);
}

bool RefusePlayHelper::IsPauseAllowedByWhiteList()
{
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG_RET(service != nullptr, false, "a2dp service nullptr");
    std::vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
    std::vector<RawAddress> devices = service->GetDevicesByStates(states);
    for (auto device : devices) {
        std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
        if ((remoteDevice != nullptr &&
            std::find(NOT_DROP_PAUSE_AFTER_SCO_DEVICES.begin(), NOT_DROP_PAUSE_AFTER_SCO_DEVICES.end(),
            remoteDevice->GetRemoteName()) != NOT_DROP_PAUSE_AFTER_SCO_DEVICES.end()) ||
            device.GetAddress().find(CONTINENTAL_ADDRESS) == 0) {
            return true;
        }
    }
    return false;
}

bool RefusePlayHelper::IsShowCapsuleAfterHangUp(const std::string &device)
{
    int64_t lastHangUpTime = GetLastHangUpTime(device);
    if (lastHangUpTime == DEFAULT_VALUE) {
        HILOGW("last hang up time is default time");
        return false;
    }
    int64_t currentTime = GetTimeStamp();
    if (currentTime - lastHangUpTime <= AFTER_HANG_UP_SHOW_CAPSULE_TIME_DIFF_MS) {
        return true;
    }
    return false;
}

int64_t RefusePlayHelper::GetLastHangUpTime(const std::string &device)
{
    TimeRecord timeRecord;
    if (!mTimeRecordMap_.Find(device, timeRecord)) {
        return DEFAULT_VALUE;
    }
    return timeRecord.lastHangUpTime;
}

void RefusePlayHelper::SetLastHangUpTime(const std::string &device)
{
    int64_t currentTime = GetTimeStamp();
    HILOGD("currentTime: %{public}lld, device: %{public}s", currentTime, GET_ENCRYPT_STR_ADDR(device));
    TimeRecord timeRecord;
    if (mTimeRecordMap_.Find(device, timeRecord)) {
        timeRecord.lastHangUpTime = currentTime;
        mTimeRecordMap_.EnsureInsert(device, timeRecord);
    } else {
        timeRecord.lastHangUpTime = currentTime;
        mTimeRecordMap_.Insert(device, timeRecord);
    }
}

bool RefusePlayHelper::NeedIgnoreSetActive()
{
    int64_t lastCallTime = 0;
    mTimeRecordMap_.Iterate([&lastCallTime](const std::string &addr, const TimeRecord &timeRecord) {
        lastCallTime = (timeRecord.lastHangUpTime > lastCallTime) ? timeRecord.lastHangUpTime : lastCallTime;
    });
    int64_t nowTime = GetTimeStamp();
    if (nowTime < lastCallTime + DROP_KEY_WITHIN_SCO_END_INTERVAL_MS) {
        HILOGI("WITHIN_SCO_END_INTERVAL_MS");
        return true;
    }

    return false;
}

bool RefusePlayHelper::NeedDropDueScoEnd(const std::string &device, bool isPushed)
{
    bool pushed = false;
    if (mKeyPushedMap_.Find(device, pushed)) {
        HILOGI("NeedDropDueScoEnd, mKeyPushedMap_ drop device:%{public}s", GET_ENCRYPT_STR_ADDR(device));
        mKeyPushedMap_.Erase(device);
        return true;
    }

    int64_t lastCallTime = 0;
    mTimeRecordMap_.Iterate([&lastCallTime](const std::string &addr, const TimeRecord &timeRecord) {
        lastCallTime = (timeRecord.lastHangUpTime > lastCallTime) ? timeRecord.lastHangUpTime : lastCallTime;
    });
    int64_t nowTime = GetTimeStamp();
    bool scoPause = false;
    if (nowTime < lastCallTime + DROP_KEY_WITHIN_SCO_END_INTERVAL_MS) {
        HILOGI("nowTime - lastCallTime is less than DROP_KEY_WITHIN_SCO_END_INTERVAL_MS.");
        if (mScoPauseMap_.Find(device, scoPause)) {
            // push and release we all need jugde, only remove in release
            CHECK_AND_RETURN_LOG_RET(isPushed == false, false, "");
            mScoPauseMap_.Erase(device);
            HILOGI("this device send pause before sco, no drop");
            return false;
        }
        CHECK_AND_RETURN_LOG_RET(isPushed == true, true, "");
        mKeyPushedMap_.Insert(device, isPushed);
        return true;
    }
    // play coming in other time, clear map
    if (mScoPauseMap_.Find(device, scoPause)) {
        mScoPauseMap_.Erase(device);
    }
    return false;
}

void RefusePlayHelper::ProcessPushed(const std::string &device, bool isPushed)
{
    bool pushed = false;
    if (mKeyPushedMap_.Find(device, pushed) && !isPushed) {
        mKeyPushedMap_.Erase(device);
        return;
    }
    if (isPushed) { // this condition is to prevent case: pushed/not in sco -> released/sco
        mKeyPushedMap_.Insert(device, isPushed);
    }
}

void RefusePlayHelper::SavePauseTime(const std::string &device)
{
    int64_t currentTime = GetTimeStamp();
    TimeRecord timeRecord;
    if (mTimeRecordMap_.Find(device, timeRecord)) {
        timeRecord.lastPauseTime = currentTime;
        mTimeRecordMap_.EnsureInsert(device, timeRecord);
    } else {
        timeRecord.lastPauseTime = currentTime;
        mTimeRecordMap_.Insert(device, timeRecord);
    }
}

// 校验暂停候选，成功时保存该车学习窗口
void RefusePlayHelper::TryLearnAndSave(const std::string &device, TimeRecord &timeRecord, int64_t pauseTime)
{
    int64_t learnOffset = TryLearnFromUserPause(device, timeRecord, pauseTime);
    mTimeRecordMap_.EnsureInsert(device, timeRecord);
    if (learnOffset == DEFAULT_VALUE) {
        return;
    }
    int learnedDuration = GetLearnedAutoPlayDuration(device);
    int effectiveDuration = std::max(learnedDuration, static_cast<int>(learnOffset));
    if (!SetLearnedAutoPlayDuration(device, effectiveDuration)) {
        HILOGE("save learned auto play duration failed, device: %{public}s", GET_ENCRYPT_STR_ADDR(device));
        return;
    }
}

// 处理本机播放状态暂停：仅当存在本轮 PLAY 候选、暂停时间有效且 T 能延长窗口时更新配置。
void RefusePlayHelper::HandlePauseEvent(const std::string &device)
{
    if (device.empty()) {
        return;
    }
    TimeRecord timeRecord;
    // 没有PLAY 候选时不学习
    if (!mTimeRecordMap_.Find(device, timeRecord) ||
        timeRecord.pendingPlayTime == DEFAULT_VALUE || timeRecord.pendingPlayOffset == DEFAULT_VALUE) {
        return;
    }

    int64_t currentTime = GetTimeStamp();
    TryLearnAndSave(device, timeRecord, currentTime);
}

void RefusePlayHelper::JudgeScoPause(const std::string &device)
{
    std::string judgeDevice = device;
#ifdef MULTI_A2DP_ENABLED
    // Multi A2DP has no single active sink; judge by the device passed in.
#else
    A2dpService *service = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (service != nullptr) {
        judgeDevice = service->GetActiveSinkDevice().GetAddress();
    }
#endif
    int64_t scoBeginTime = GetTimeStamp();
    TimeRecord record;
    if (!mTimeRecordMap_.Find(judgeDevice, record)) {
        HILOGI("judgeScoPause, no pause rcd. device:%{public}s", GET_ENCRYPT_STR_ADDR(judgeDevice));
        return;
    }
    if (scoBeginTime - record.lastPauseTime < 0) {
        HILOGI("judgeScoPause, no pause rcd. device:%{public}s", GET_ENCRYPT_STR_ADDR(judgeDevice));
        return;
    }
    if (scoBeginTime - record.lastPauseTime < SCO_BEGIN_PAUSE_TIME) {
        HILOGI("pause key before sco in 2s");
        mScoPauseMap_.Insert(judgeDevice, true);
    }
}
}
}
