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

#include "car_profile_service.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_pms"
#endif
#include "log.h"

#include "a2dp_snk_service.h"
#include "bluetooth_def.h"
#include "hfp_hf_service.h"
#include "raw_address.h"
#include "pbap_pce_service.h"
#include "common_util.h"
#include "interface_profile.h"
#include <vector>

namespace OHOS {
namespace bluetooth {

namespace {
// ProfileType → IProfile 单点分发。
// 延迟调用 GetService:profile 服务随蓝牙开关动态注册/注销,不可缓存指针。
IProfile *GetProfileService(ProfileType profile)
{
    switch (profile) {
        case ProfileType::HFP_HF:
            return HfpHfService::GetService();
        case ProfileType::A2DP_SINK:
            return A2dpSnkService::GetService();
        case ProfileType::PBAP_PCE:
            return PbapPceService::GetService();
        default:
            HILOGW("GetProfileService: unsupported profile=%{public}d", static_cast<int>(profile));
            return nullptr;
    }
}
}  // namespace

void CarProfileService::Connect(ProfileType profile, const std::string &addr)
{
    HILOGI("CarProfileService::Connect: profile=%{public}d, addr=%{public}s", profile, GetEncryptAddr(addr).c_str());
    auto *svc = GetProfileService(profile);
    if (svc != nullptr) {
        svc->Connect(RawAddress(addr));
    }
}

void CarProfileService::Disconnect(ProfileType profile, const std::string &addr)
{
    HILOGI("CarProfileService::Disconnect: profile=%{public}d, addr=%{public}s", profile, GetEncryptAddr(addr).c_str());
    auto *svc = GetProfileService(profile);
    if (svc != nullptr) {
        svc->Disconnect(RawAddress(addr));
    }
}

void CarProfileService::NotifyA2dpConnected()
{
    auto *svc = A2dpSnkService::GetService();
    if (svc != nullptr) {
        svc->NotifyA2dpConnected(true);
    }
}

void CarProfileService::NotifyPlaybackDisallowed()
{
    auto *svc = A2dpSnkService::GetService();
    if (svc != nullptr) {
        svc->NotifyPlaybackDisallowed();
    }
}

bool CarProfileService::HasHfpConnectingOrConnected()
{
    auto *svc = HfpHfService::GetService();
    if (svc == nullptr) {
        return false;
    }
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTING),
                               static_cast<int>(BTConnectState::CONNECTED)};
    return !svc->GetDevicesByStates(states).empty();
}

int CarProfileService::GetHfpDeviceState(const std::string &addr)
{
    auto *svc = HfpHfService::GetService();
    if (svc == nullptr) {
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    return svc->GetDeviceState(RawAddress(addr));
}

int CarProfileService::GetA2dpDeviceState(const std::string &addr)
{
    auto *svc = A2dpSnkService::GetService();
    if (svc == nullptr) {
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    return svc->GetDeviceState(RawAddress(addr));
}

}  // namespace bluetooth
}  // namespace OHOS
