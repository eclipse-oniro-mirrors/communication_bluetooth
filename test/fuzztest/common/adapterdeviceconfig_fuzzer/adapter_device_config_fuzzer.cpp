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

#include "adapter_device_config_fuzzer.h"
#include "permission_manager.h"
#include "adapter_device_config.h"
#include "bluetooth_log.h"
#include "securec.h"

using namespace std;
using namespace OHOS::Bluetooth;
const int FUZZ_DATA_LEN = 6;
const int FUZZ_MAX_STR_LEN = 6;
namespace OHOS {
    void TestForAdapterDeviceConfig()
    {
        std::string addr = "00:00:00:00:00:00";
        int32_t shareType = 0;

        bluetooth::SetPbapShareType(addr, shareType);
        bluetooth::GetPbapShareType(addr, shareType);

        int32_t permission = 0;
        bluetooth::SetPbapPermission(addr, permission);
        bluetooth::GetPbapPermission(addr, permission);

        int32_t rejectCount = 0;
        bluetooth::SetPbapRejectCount(addr, rejectCount);
        bluetooth::GetPbapRejectCount(addr, rejectCount);

        bluetooth::SetMessagePermission(addr, permission);
        bluetooth::GetMessagePermission(addr, permission);

        int autoPlayValue = 0;
        bluetooth::SaveAutoPlayValue(addr, autoPlayValue);
        bluetooth::GetAutoPlayValue(addr, autoPlayValue);

        int duration = 0;
        bluetooth::SetRestrictedDuration(addr, duration);
        bluetooth::GetRestrictedDuration(addr, duration);

        bool isTrue = true;
        bluetooth::SetHfpAgVgsSupport(addr, isTrue);
        bluetooth::GetHfpAgVgsSupport(addr, isTrue);
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        if ((data == nullptr) || (size < FUZZ_DATA_LEN)) {
            return false;
        }
        bluetooth::IAdapterDeviceConfig *adapterDeviceConfig = bluetooth::AdapterDeviceConfig::GetInstance();
        int len = FUZZ_MAX_STR_LEN;
        std::string str(reinterpret_cast<const char*>(data), len);
        for (int i = 0; i < len; i++) {
            char c = str[i];
            bool ret = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
            if (!ret) {
                str[i] = 'a';
            }
        }

        int value = 0;
        bool isTrue = true;
        adapterDeviceConfig->GetValue(str, str, str, value);
        adapterDeviceConfig->GetValue(str, str, str, str);
        adapterDeviceConfig->GetValue(str, str, str, isTrue);
        adapterDeviceConfig->SetValue(str, str, str, value, isTrue);
        adapterDeviceConfig->SetValue(str, str, str, str, isTrue);
        adapterDeviceConfig->SetValue(str, str, str, isTrue, isTrue);

        std::vector<std::string> columns = {"contact_id"};
        adapterDeviceConfig->GetSubSections(str, columns);
        adapterDeviceConfig->RemoveSection(str, str, isTrue);

        adapterDeviceConfig->GetValue(str, str, value);
        adapterDeviceConfig->GetValue(str, str, str);
        adapterDeviceConfig->GetValue(str, str, isTrue);
        adapterDeviceConfig->SetValue(str, str, value, isTrue);
        TestForAdapterDeviceConfig();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

