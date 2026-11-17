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

#include "sensorhub_collaboration_fuzzer.h"
#include "permission_manager.h"
#include "sensorhub_collaboration_service.h"
#include "bluetooth_hw_interface.h"
#include "ble_scanner_state_machine.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::bluetooth;
namespace OHOS {

    // Constants for fuzzer testing
    static constexpr int MIN_SCREEN_STATE = 0;
    static constexpr int MAX_SCREEN_STATE = 2;
    static constexpr int MIN_SCAN_TYPE = 0;
    static constexpr int MAX_SCAN_TYPE = 2;

    static struct SensorhubCollaborationTestResult {
        bool setBufferSizeAndCount;
        char sendToSensorhubType;
        std::vector<char> sendToSensorhubParam;

        int scanMsgWhat;
        int scanMsgArg1;
        Bluetooth::BleScannerStateMachine::ScannerAppInfo sensorScanner;
    } g_shCollTestResult;

    static void SendParamsToSensorhubMock(const char *params, char type, int length)
    {
        HILOGI("save sensorhub msg, type: %{public}d, len: %{public}d", type, length);
        g_shCollTestResult.sendToSensorhubType = type;
        g_shCollTestResult.sendToSensorhubParam = std::vector<char>(params, params + length);
    }

    static void HwConnAttrSetMock(uint8_t transport, const STACK::RawAddress &addr, const HwConnAttr *attr)
    {
        HILOGI("set attr, transport: %{public}hhu, type: %{public}hhu", transport, attr->type);
    }

    static const bthwif_interface_t g_shCollHwifTestInterface = {
        .sendParamsToSensorhub = SendParamsToSensorhubMock,
        .hwConnAttrSet = HwConnAttrSetMock,
    };

    // Helper class to access private impl
    class SensorhubCollaborationServiceAccessor {
    public:
        static auto GetImpl(SensorhubCollaborationService* service)
        {
            return service->pimpl.get();
        }
    };

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        BluetoothHwInterface::GetInstance()->SetBtHwInterface(&g_shCollHwifTestInterface);
        SensorhubCollaborationService::GetInstance()->Init();
        SensorhubCollaborationService::GetInstance()->IsScanCollborationEnabled();
        uint8_t type = provider.ConsumeIntegral<uint8_t>();
        std::vector<SensorhubScannerInfo> appInfos;
        SensorhubCollaborationService::GetInstance()->SendScanCollaborationMsg(type, appInfos);
        SensorhubCollaborationService::GetInstance()->RegisterScreenStateCallback();
        ScreenState state = static_cast<ScreenState>(provider.ConsumeIntegralInRange<int>(MIN_SCREEN_STATE,
            MAX_SCREEN_STATE));
        SensorhubCollaborationService::GetInstance()->OnScreenStateChanged(state);
        int32_t scannerId = provider.ConsumeIntegral<int32_t>();
        bool enable = true;
        SensorhubCollaborationService::GetInstance()->SetScanReportChannelToLpDevice(scannerId, enable);
        SensorhubCollaborationService::GetInstance()->QueryCapability();
        std::string connName = "testName";
        SensorhubCollaborationService::GetInstance()->AllConnectionComplete(connName);
        SensorhubCollaborationService::GetInstance()->StartConnCollaboration(connName);

        // Test Register/DeregisterShCollaborationObserver
        class TestShCollaborationObserver : public ISensorhubCollaborationObserver {
        public:
            void OnSensorhubStateChanged(SensorhubState state) override {}
            void OnSensorhubScanCollaborationMsgRecved(const SensorhubScannerInfo &info) override {}
            void OnSensorhubConnCollaborationCompleted() override {}
        };
        TestShCollaborationObserver observer;
        SensorhubCollaborationService::GetInstance()->RegisterShCollaborationObserver(observer);
        SensorhubCollaborationService::GetInstance()->DeregisterShCollaborationObserver(observer);

        // Test OnScreenStateChanged
        SensorhubCollaborationService::GetInstance()->OnScreenStateChanged(state);

        // Test SendScanCollaborationMsg with different types
        uint8_t scanType = provider.ConsumeIntegralInRange<uint8_t>(MIN_SCAN_TYPE, MAX_SCAN_TYPE);
        uint16_t scanMode = provider.ConsumeIntegral<uint16_t>();
        uint16_t scanInterval = provider.ConsumeIntegral<uint16_t>();
        uint16_t scanWindow = provider.ConsumeIntegral<uint16_t>();
        SensorhubScannerInfo scannerInfo(scannerId, scanMode, scanInterval, scanWindow);
        std::vector<SensorhubScannerInfo> appInfosWithScanner = {scannerInfo};
        SensorhubCollaborationService::GetInstance()->SendScanCollaborationMsg(scanType, appInfosWithScanner);

        // Test SendScanCollaborationMsg with empty appInfos
        std::vector<SensorhubScannerInfo> emptyAppInfos;
        SensorhubCollaborationService::GetInstance()->SendScanCollaborationMsg(scanType, emptyAppInfos);

        // Test additional SendScanCollaborationMsg scenarios
        SensorhubScannerInfo scannerInfo2(scanInterval, scanWindow);
        std::vector<SensorhubScannerInfo> appInfos2 = {scannerInfo2};
        SensorhubCollaborationService::GetInstance()->SendScanCollaborationMsg(scanType, appInfos2);

        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    /* Run your code on data */
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}