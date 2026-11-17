/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef HFP_AG_INTERFACE_MOCK_H
#define HFP_AG_INTERFACE_MOCK_H

#include <gmock/gmock.h>
#include "bluetooth_headset_interface.h"

namespace bluetooth {
namespace headset {
class HfpAgInterfaceMock : public ::bluetooth::headset::Interface {
public:
    MOCK_METHOD3(Init, bt_status_t(Callbacks*, int, bool));
    MOCK_METHOD1(Connect, bt_status_t(RawAddress*));
    MOCK_METHOD1(Disconnect, bt_status_t(RawAddress*));
    MOCK_METHOD1(ConnectAudio, bt_status_t(RawAddress*));
    MOCK_METHOD1(DisconnectAudio, bt_status_t(RawAddress*));
    MOCK_METHOD1(isNoiseReductionSupported, bt_status_t(RawAddress*));
    MOCK_METHOD1(isVoiceRecognitionSupported, bt_status_t(RawAddress*));
    MOCK_METHOD1(StartVoiceRecognition, bt_status_t(RawAddress*));
    MOCK_METHOD1(StopVoiceRecognition, bt_status_t(RawAddress*));
    MOCK_METHOD3(VolumeControl, bt_status_t(bthf_volume_type_t, int, RawAddress*));
    MOCK_METHOD5(DeviceStatusNotification, bt_status_t(bthf_network_state_t,
        bthf_service_type_t, int, int, RawAddress*));
    MOCK_METHOD2(CopsResponse, bt_status_t(const char*, RawAddress*));
    MOCK_METHOD8(CindResponse, bt_status_t(int, int, int, bthf_call_state_t, int, int, int, RawAddress*));
    MOCK_METHOD2(FormattedAtResponse, bt_status_t(const char*, RawAddress*));
    MOCK_METHOD3(AtResponse, bt_status_t(bthf_at_response_t, int, RawAddress*));
    MOCK_METHOD8(ClccResponse, bt_status_t(int, bthf_call_direction_t, bthf_call_state_t,
        bthf_call_mode_t, bthf_call_mpty_type_t, const char*, bthf_call_addrtype_t, RawAddress*));
    MOCK_METHOD7(PhoneStateChange, bt_status_t(int, int, bthf_call_state_t, const char*,
        bthf_call_addrtype_t, const char*, RawAddress*));
    MOCK_METHOD0(Cleanup, void());
    MOCK_METHOD1(SetScoAllowed, bt_status_t(bool));
    MOCK_METHOD2(SendBsir, bt_status_t(bool, RawAddress*));
    MOCK_METHOD1(SetActiveDevice, bt_status_t(RawAddress*));
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // HFP_AG_INTERFACE_MOCK_H