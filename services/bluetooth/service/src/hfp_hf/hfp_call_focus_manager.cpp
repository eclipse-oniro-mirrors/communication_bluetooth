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

#ifndef LOG_TAG
#define LOG_TAG "bt_service_hfp_call_focus"
#endif

#include "log.h"
#include "hfp_call_focus_manager.h"

namespace OHOS {
namespace bluetooth {

namespace {
class CallFocusCallbackImpl : public AudioStandard::AudioInterruptCallback {
public:
    ~CallFocusCallbackImpl() override = default;
    void OnInterrupt(const AudioStandard::InterruptEventInternal &interruptEvent) override
    {
        if (callback_) {
            callback_(interruptEvent.hintType);
        }
    }
    void SetCallback(std::function<void(AudioStandard::InterruptHint)> cb) { callback_ = std::move(cb); }
private:
    std::function<void(AudioStandard::InterruptHint)> callback_;
};
}

HfpCallFocusManager::~HfpCallFocusManager()
{
    Deinit();
}

int32_t HfpCallFocusManager::Init(std::function<void(AudioStandard::InterruptHint)> callback)
{
    auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
    if (mgr == nullptr) {
        HILOGE("[HFP_CALL_FOCUS]AudioSystemManager is nullptr");
        return -1;
    }

    int32_t ret = mgr->GenerateSessionId(sessionId_);
    if (ret != 0 || sessionId_ == 0) {
        HILOGE("[HFP_CALL_FOCUS]GenerateSessionId failed ret=%{public}d sessionId=%{public}u", ret, sessionId_);
        sessionId_ = 0;
        return ret;
    }

    auto cb = std::make_shared<CallFocusCallbackImpl>();
    cb->SetCallback(std::move(callback));
    ret = mgr->SetAudioInterruptCallback(sessionId_, cb, static_cast<uint32_t>(getuid()), 0);
    if (ret != 0) {
        HILOGE("[HFP_CALL_FOCUS]SetAudioInterruptCallback failed ret=%{public}d", ret);
        sessionId_ = 0;
        return ret;
    }

    callback_ = cb;
    callbackRegistered_ = true;
    HILOGI("[HFP_CALL_FOCUS]Init success sessionId=%{public}u", sessionId_);
    return 0;
}

AudioStandard::AudioInterrupt HfpCallFocusManager::CreateCallAudioInterrupt()
{
    AudioStandard::AudioInterrupt interrupt;
    interrupt.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    interrupt.contentType = AudioStandard::ContentType::CONTENT_TYPE_SPEECH;
    interrupt.audioFocusType.streamType = AudioStandard::AudioStreamType::STREAM_VOICE_CALL;
    interrupt.audioFocusType.isPlay = true;
    interrupt.sessionId = sessionId_;
    interrupt.pid = getpid();
    interrupt.uid = getuid();
    interrupt.mode = AudioStandard::InterruptMode::SHARE_MODE;
    return interrupt;
}

int32_t HfpCallFocusManager::ActivateCallFocus()
{
    if (activated_) {
        HILOGI("[HFP_CALL_FOCUS]Already activated, skip sessionId=%{public}u", sessionId_);
        return 0;
    }
    if (!callbackRegistered_ || sessionId_ == 0) {
        HILOGE("[HFP_CALL_FOCUS]Not initialized, cannot activate");
        return -1;
    }

    auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
    if (mgr == nullptr) {
        HILOGE("[HFP_CALL_FOCUS]AudioSystemManager is nullptr");
        return -1;
    }

    AudioStandard::AudioInterrupt interrupt = CreateCallAudioInterrupt();
    int32_t ret = mgr->ActivateAudioInterrupt(interrupt);
    if (ret != 0) {
        HILOGE("[HFP_CALL_FOCUS]ActivateAudioInterrupt failed ret=%{public}d sessionId=%{public}u",
            ret, sessionId_);
        return ret;
    }

    activated_ = true;
    HILOGI("[HFP_CALL_FOCUS]ActivateCallFocus success sessionId=%{public}u", sessionId_);
    return 0;
}

void HfpCallFocusManager::DeactivateCallFocus()
{
    if (!activated_) {
        return;
    }

    auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
    if (mgr != nullptr) {
        AudioStandard::AudioInterrupt interrupt = CreateCallAudioInterrupt();
        int32_t ret = mgr->DeactivateAudioInterrupt(interrupt);
        if (ret != 0) {
            HILOGE("[HFP_CALL_FOCUS]DeactivateAudioInterrupt failed ret=%{public}d sessionId=%{public}u",
                ret, sessionId_);
        }
    }

    activated_ = false;
    HILOGI("[HFP_CALL_FOCUS]DeactivateCallFocus done sessionId=%{public}u", sessionId_);
}

void HfpCallFocusManager::Deinit()
{
    if (sessionId_ == 0) {
        return;
    }

    if (activated_) {
        DeactivateCallFocus();
    }

    if (callbackRegistered_) {
        auto *mgr = AudioStandard::AudioSystemManager::GetInstance();
        if (mgr != nullptr) {
            int32_t ret = mgr->UnsetAudioInterruptCallback(static_cast<uint32_t>(getuid()), sessionId_);
            if (ret != 0) {
                HILOGE("[HFP_CALL_FOCUS]UnsetAudioInterruptCallback failed ret=%{public}d", ret);
            }
        }
        callbackRegistered_ = false;
    }

    callback_.reset();
    sessionId_ = 0;
    HILOGI("[HFP_CALL_FOCUS]Deinit done");
}

bool HfpCallFocusManager::IsActivated() const
{
    return activated_;
}

uint32_t HfpCallFocusManager::GetSessionId() const
{
    return sessionId_;
}

}  // namespace bluetooth
}  // namespace OHOS
