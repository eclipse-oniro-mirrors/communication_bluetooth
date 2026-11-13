/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KSTACK, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "bt_service_hdf_dynamic_library_loader"
#endif

#include "hdf_dynamic_library_loader.h"
#include "hitrace_meter.h"
#include "log.h"
#include "remote_device_properties.h"

namespace OHOS {
namespace bluetooth {

constexpr uint16_t HDF_RELOAD_DELAY_MS = 300;
constexpr int8_t MAX_LOAD_RETRY_TIMES = 3;
const char *AUDIO_BLUETOOTH_SERVICE_NAME = "audio_bluetooth_hdi_service";
const char *AUDIO_HDI_SERVICE_NAME = "bluetooth_audio_session_service";
static HdfRemoteService *g_hdfRemoteService = nullptr;
static void AddHdiDeathRecipient()
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    if (serviceMgr == nullptr) {
        HILOGE("HDIServiceManagerGet failed!");
        return;
    }
    g_hdfRemoteService = serviceMgr->GetService(serviceMgr, AUDIO_BLUETOOTH_SERVICE_NAME);
    HDIServiceManagerRelease(serviceMgr);
    if (g_hdfRemoteService == nullptr) {
        HILOGE("Remote GetService [g_hdfRemoteService] failed!");
        return;
    }

    auto func = [](HdfDeathRecipient *deathRecipient, HdfRemoteService *service) {
        HILOGI("Hdi service died! reload service delay ");
        auto delayReload = []() {
            auto devmgr = OHOS::HDI::DeviceManager::V1_0::IDeviceManager::Get();
            if (devmgr != nullptr) {
                devmgr->LoadDevice(AUDIO_HDI_SERVICE_NAME);
                devmgr->LoadDevice(AUDIO_BLUETOOTH_SERVICE_NAME);
            }
            AddHdiDeathRecipient();
        };
        ThreadUtil::GetInstance().PostTask(
            THREAD_ID_A2DP, delayReload, HDF_RELOAD_DELAY_MS, "RELOAD_A2DP_HOST_TASK");
    };
    static HdfDeathRecipient recipient = {.OnRemoteDied = func};

    HdfRemoteServiceAddDeathRecipient(g_hdfRemoteService, &recipient);
    HILOGI("Hdi service add death recipient success!");
}

static void RemoveHdiDeathRecipient()
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    if (serviceMgr == nullptr) {
        HILOGE("HDIServiceManagerGet failed!");
        return;
    }
    HDIServiceManagerRelease(serviceMgr);
    HdfRemoteServiceRemoveDeathRecipient(g_hdfRemoteService, nullptr);
    HILOGI("Hdi service remove death recipient success!");
}

HdfDynamicLibraryLoader& HdfDynamicLibraryLoader::GetInstance()
{
    static HdfDynamicLibraryLoader instance;
    return instance;
}

bool HdfDynamicLibraryLoader::LoadTask()
{
    HITRACE_METER(HITRACE_TAG_ZAUDIO);
    auto devmgr = OHOS::HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr != nullptr) {
        int32_t resultHdi = HDF_SUCCESS;
        int32_t resultSession = HDF_SUCCESS;
        for (int retryCount = 0; retryCount < MAX_LOAD_RETRY_TIMES; retryCount++) {
            int64_t currentTime = GetTimeStamp();
            HILOGI("start LoadDevice of bluetooth audio HDF, currentTime: %{public}lld", currentTime);
            resultHdi = devmgr->LoadDevice(AUDIO_HDI_SERVICE_NAME);
            resultSession = devmgr->LoadDevice(AUDIO_BLUETOOTH_SERVICE_NAME);
            bool hdiFailed = (resultHdi != HDF_SUCCESS && resultHdi != HDF_ERR_DEVICE_BUSY);
            bool sessionFailed = (resultSession != HDF_SUCCESS && resultSession != HDF_ERR_DEVICE_BUSY);
            if (!hdiFailed || !sessionFailed) {
                break;
            }
            currentTime = GetTimeStamp();
            HILOGE("LoadDevice both failed, retrying... count: %{public}d, currentTime: %{public}lld",
                retryCount + 1, currentTime);
        }
    }
    AddHdiDeathRecipient();
    return true;
};

bool HdfDynamicLibraryLoader::UnloadTask()
{
    auto devmgr = OHOS::HDI::DeviceManager::V1_0::IDeviceManager::Get();
    RemoveHdiDeathRecipient();
    if (devmgr != nullptr) {
        HILOGI("unloadDevice of bluetooth audio HDF");
        devmgr->UnloadDevice(AUDIO_BLUETOOTH_SERVICE_NAME);
        devmgr->UnloadDevice(AUDIO_HDI_SERVICE_NAME);
    }
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
