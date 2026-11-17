/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved.
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

#undef private
#define private public

#include "bluetooth_hw_interface_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "permission_manager.h"
#include "bluetooth_hw_interface.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "hiecho.h"

using namespace std;
using namespace OHOS::Bluetooth;

namespace OHOS {

    static constexpr uint8_t QUERY_BUSINESS_LEN = 2;

    void BluetoothHwInterfaceTest(const uint8_t* data, size_t size)
    {
        FuzzedDataProvider provider(data, size);
        int rawStatus = provider.ConsumeIntegralInRange<int>(0, 14);
        bt_status_t status = static_cast<bt_status_t>(rawStatus);
        std::string addr = "11:22:33:44:55:66";
        bluetooth::RawAddress rawAddr(addr);
        STACK::RawAddress address = bluetooth::ServiceUtil::AddrToStack(rawAddr);
        int btStackErrno = provider.ConsumeIntegralInRange<int>(0, 701);
        BtStackErrno state = static_cast<BtStackErrno>(btStackErrno);
        bluetooth::StackErrnoCallback(status, &address, state);
        bool isConnected = provider.ConsumeBool();
        uint8_t featureBit = provider.ConsumeIntegral<uint8_t>();
        HdapConfigCallback config;
        bluetooth::HwHdapConnectCallback(&address, isConnected, featureBit, config);
        uint8_t a2dpState = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        uint8_t hfpState = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        uint8_t a2dpServiceType = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        bluetooth::HwProfileStateCallback(&address, a2dpState, hfpState, addr, a2dpServiceType);
        int reason = provider.ConsumeIntegral<int>();
        bluetooth::HwAclDisconnReasonCb(&address, reason);
        uint8_t buffer = provider.ConsumeIntegral<uint8_t>();
        int length = provider.ConsumeIntegral<int>();
        bluetooth::HwGetSensorhubDevInfoCb(&buffer, length);
        uint32_t sensorhubStatetate = provider.ConsumeIntegral<uint32_t>();
        bluetooth::HwSensorhubResetCb(sensorhubStatetate);
        uint8_t echoType = provider.ConsumeIntegralInRange<uint8_t>(1, 2);
        uint8_t payload[256] = {0};
        uint16_t len = provider.ConsumeIntegralInRange<uint16_t>(0, sizeof(payload) - 1);
        uint8_t args[256] = {0};
        uint16_t argsLen = provider.ConsumeIntegralInRange<uint16_t>(0, sizeof(args) - 1);
        bluetooth::HwHiechoIndCallCb(echoType, payload, len, args, argsLen);
        StackCallbackParam param;
        bluetooth::HwBluetoothConnCb(param);
        int deviceType = provider.ConsumeIntegralInRange<int>(-1, 3);
        bluetooth::HwDeviceCallback(&address, deviceType);
        A2dpOffloadConfigCallback a2dpOffloadConfig;
        bluetooth::HwA2dpOffloadCodecConfigCallback(&address, a2dpOffloadConfig);
        bluetooth::HwHiechoDeviceAclEncryptionChangedCb(address);
        int32_t errCode = provider.ConsumeIntegral<int32_t>();
        bluetooth::HwReportA2dpAbnormalStatusCb(address, errCode);

        bluetooth::ProcessA2dpProfileState(rawAddr, a2dpState);
        bluetooth::ProcessHfpProfileState(rawAddr, hfpState);
        bluetooth::CapsuleParam capsuleParam;
        capsuleParam.isAudioOutputToA2dp = provider.ConsumeBool();
        capsuleParam.isMusicActive = provider.ConsumeBool();
        capsuleParam.isA2dpActiveDeviceEqual = provider.ConsumeBool();
        capsuleParam.isStreamEmptyAduio = provider.ConsumeBool();
        bluetooth::IsOtherA2dpDeviceInPlayingMusic(capsuleParam);
        bluetooth::IsOtherScoDeviceInCalling(capsuleParam);
        uint8_t profileState = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        bluetooth::ReportCapsuleInfo(rawAddr, profileState, addr);
        bluetooth::NotifyAudioManagerShowEnableCapsule(capsuleParam);
        bluetooth::NotifyAudioManagerShowDisableCapsule(capsuleParam);
        bluetooth::PrintCapsuleParam(capsuleParam);
        bluetooth::NotifyAudioManagerShowCapsule(rawAddr, a2dpState, hfpState, addr, capsuleParam.isStreamEmptyAduio);
        uint8_t cap = provider.ConsumeIntegral<uint8_t>();
        bluetooth::HwHiechoSendPerServiceCapReqData(cap, address);
        bluetooth::HwIsSupportMlinkDevice();
        bluetooth::HwHiechoSendHwAccountHashToTwsCb(address);
        bluetooth::HwHiechoDtsCap(echoType, args);
        std::vector<uint8_t> payloads = {0x01, 0x02, 0x03};
        bluetooth::HwHiechoAudioCapReq(payloads, len, args);
        bluetooth::HwHiechoWearState(rawAddr, payloads, len, args);
        bluetooth::HwHiechoQueryConn(echoType, payloads, QUERY_BUSINESS_LEN, args);
        bluetooth::HwHiechoAutoConnSwitch(echoType, payloads, len, args);
        bluetooth::HwHiechoParseDescriptionRsp(echoType, payloads, length, args);
        bluetooth::HwHiechoDeviceFoundConn(echoType, payloads, length, args);
        bluetooth::HwHiechoDeviceFoundState(echoType, payloads, length, args);
        bluetooth::HwHiechoIndCallBack(echoType, payload, len, args, argsLen);
        bluetooth::HwHiechoDtsReport(echoType, payload, len, args, argsLen);
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        bluetooth::BluetoothHwInterface* bluetoothHwInterface = bluetooth::BluetoothHwInterface::GetInstance();
        FuzzedDataProvider provider(data, size);
        const bthwif_interface_t *bthwif = bluetoothHwInterface->GetBtHwInterface();
        bluetoothHwInterface->InitBtHwInterface(bthwif);
        bluetoothHwInterface->SetBtHwInterface(bthwif);

        std::string addr = "11:22:33:44:55:66";
        bluetooth::RawAddress rawAddr(addr);
        bluetoothHwInterface->CleanHfpScoOccupied(rawAddr);

        uint16_t feature = provider.ConsumeIntegral<uint16_t>();
        bluetoothHwInterface->InteropMatch(feature, rawAddr);

        int uid = provider.ConsumeIntegral<int>();
        uint8_t transport = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        STACK::RawAddress address = bluetooth::ServiceUtil::AddrToStack(rawAddr);
        std::string pkgName = "com.test.pkgname";
        bluetoothHwInterface->KeepBleScanInConn(pkgName, uid, transport, address);

        uint8_t a2dpState = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        uint8_t hfpState = provider.ConsumeIntegralInRange<uint8_t>(0, 3);
        bluetoothHwInterface->ReportProfileStatus(rawAddr, a2dpState, hfpState);

        bluetooth::BluetoothHwInterface::SensorhubObserver observer;
        bluetoothHwInterface->AddSensorhubObserver(observer);
        std::shared_ptr<bluetooth::BluetoothHwInterface::SocketConnectionObserver> socketObserver;
        bluetoothHwInterface->AddSocketObserver(socketObserver);
        uint8_t outHashArray[32];
        uint32_t arrayLen = HW_ACCOUNT_HASH_LEN;
        bluetoothHwInterface->GetSha256EncryptHwHashAccount(outHashArray, arrayLen);
        uint8_t remoteAddr[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        uint32_t controlType = provider.ConsumeIntegralInRange<uint32_t>(0, 4);
        uint32_t controlTypeVal = provider.ConsumeIntegralInRange<uint32_t>(0, 4);
        uint32_t controlObject = provider.ConsumeIntegral<uint32_t>();
        bluetoothHwInterface->HwHiechoDeviceCtrl(remoteAddr, controlType, controlTypeVal, controlObject);
        char irk = provider.ConsumeIntegral<char>();
        char hbk = provider.ConsumeIntegral<char>();
        uint8_t version = provider.ConsumeIntegral<uint8_t>();
        char advFeature = provider.ConsumeIntegral<char>();
        bluetoothHwInterface->HwSetAdvKey(rawAddr, irk, hbk, version, advFeature);
        bluetoothHwInterface->HwRemoveAdvKey(rawAddr);
        std::vector<uint8_t> irkData;
        bluetoothHwInterface->HwGetAdvIrk(rawAddr, irkData);
        bluetoothHwInterface->CheckDeviceBonded(rawAddr);
        bluetoothHwInterface->RemoveSocketObserver(socketObserver);
        bluetoothHwInterface->RemoveSensorhubObserver(observer);
        bluetoothHwInterface->ClearSocketInValidObserver();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::BluetoothHwInterfaceTest(data, size);
    return 0;
}