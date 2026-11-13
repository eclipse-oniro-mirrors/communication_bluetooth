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

#undef private
#define private public

#include "sdp_found_event_publish_fuzzer.h"
#include "permission_manager.h"
#include "sdp_found_event_publish.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "classic_adapter.h"
#include "class_creator.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace std;
using namespace OHOS::bluetooth;
namespace OHOS {

    static void *ClassicAdapterCreate(void)
    {
        return new ClassicAdapter();
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        HILOGI("start");
        FuzzedDataProvider provider(data, size);
        bluetooth::ClassFactory::RegisterClass("ClassicAdapter", ClassicAdapterCreate);
        int32_t status = provider.ConsumeIntegral<int32_t>();
        std::string addr = "11:22:33:44:55:66";
        std::string uuid = "00000000-0000-1000-8000-00805F9B34FB";
        bool moreResults = provider.ConsumeBool();
        bluetooth_sdp_record record = {};
        SdpFoundEventPublishHelper::PublishMapMasSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishMapMnsSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishPbapPseSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishObexObjectPushSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishSapSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishDipSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishSdpFoundEvent(status, addr, uuid, moreResults, &record);
        SdpFoundEventPublishHelper::PublishCommonSdpFoundEvent(status, addr, uuid, moreResults, &record);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        HILOGI("invalid data");
        return false;
    }
    /* Run your code on data */
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}