/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_gatt_server"
#endif

#include "gatt_server_service.h"

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <mutex>
#include <set>

#include "bluetooth_gatt_interface.h"
#include "igatt_api.h"
#include "gatt_server_application.h"
#include "gatt_service_base.h"
#include "log.h"
#include "securec.h"
#include "thread_util.h"
#include "bluetooth_resource_manager.h"
#include "permission_manager.h"
#include "bluetooth_state_manager.h"
#include "control_intercept_plugin.h"
#include "bluetooth_errorcode.h"
#include "ipc_skeleton.h"
#include "btif_gatt.h"

namespace OHOS {
namespace bluetooth {
static constexpr uint32_t WAIT_TIMEOUT_MS = 1000; // 1000ms
class GattServerService::GattServerServiceRegisterObserver : public BluetoothGattInterface::GattServerObserver {
public:
    using RegisterCallback = std::function<void(int status, int serverIf)>;

    explicit GattServerServiceRegisterObserver(RegisterCallback cb)
        : callback_(cb)
    {
    }

    ~GattServerServiceRegisterObserver() override
    {
    }

    void RegisterServerCallback(int status, int serverIf, const STACK::bluetooth::Uuid &appUuid) override
    {
        callback_(status, serverIf);
    }

private:
    RegisterCallback callback_;
};

struct GattServerService::impl : public GattServiceBase {
    GattServerService &self;
    std::list<std::shared_ptr<GattServerApplication>> servers = {};
    // appId mutex, used with server thread (read) and inner thread (write, read)
    std::mutex appIdMutex = {};
    // appId <-> caller pid, used to prevent stale appId (e.g. appId registered before bluetooth restart)
    // from removing services registered by another process after bluetooth restart.
    // !!! servers and appIds shall be used in pairs when adding or removeing
    std::map<int, int32_t> appIds = {};

    // For sync
    static constexpr int SYNC_TIMEOUT = BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT;
    std::mutex syncMutex = {};
    std::condition_variable cv = {};
    // used for sync [un]register application
    static constexpr int INVALID_APP_ID = 0;
    static constexpr int REGISTER_APP_ID_FAILED = -1;
    int tempAppId = INVALID_APP_ID;

    // used in notify no confirm lock
    std::mutex notifyNoCfmMutex;

    explicit impl(GattServerService &service)
        : self(service), servers(), appIdMutex(), appIds(),
          syncMutex(), cv(), tempAppId(INVALID_APP_ID)
    {}
    ~impl() override {}

    bool IsValidApplication(int appId, int32_t pid);
    void NotifyServiceChanged(int appId, const Service &service);
    bool WaitForRegisterCallback(int &outAppId);
    void RegisterServerCallback(int status, int serverIf);
    void RegisterApplicationInThread(std::weak_ptr<IGattServerCallback> callback,
        std::shared_ptr<std::promise<int>> promise, uint32_t tokenId, int32_t pid);
    void NotifyClient(const GattDevice &device, uint16_t handle, std::vector<uint8_t> value);
    void NotifyClientNoCfm(const GattDevice &device, uint16_t handle, std::vector<uint8_t> value);

    std::list<std::shared_ptr<GattServerApplication>>::iterator GetApplicationIter(int appId)
    {
        return std::find_if(servers.begin(), servers.end(),
            [appId](const std::shared_ptr<GattServerApplication> &app) { return app->GetAppId() == appId; });
    }
    std::list<std::shared_ptr<GattServerApplication>>::iterator GetApplicationIter(uint16_t handle)
    {
        return std::find_if(servers.begin(), servers.end(),
            [handle](const std::shared_ptr<GattServerApplication> &app) { return app->IsValidHandle(handle); });
    }
};

GattServerService::GattServerService()
    : utility::Context("GattServerService", "5.0.0"), btIfGattServer_(BtGetGattServerInterface()),
      pimpl(std::make_unique<GattServerService::impl>(*this))
{}

GattServerService::~GattServerService()
{}

void GattServerService::SetBtifInterface(const btgatt_server_interface_t *interface)
{
    btIfGattServer_ = interface;
}

utility::Context *GattServerService::GetContext(void)
{
    return this;
}

std::list<RawAddress> GattServerService::GetConnectDevices(void)
{
    return std::list<RawAddress>();
}

int GattServerService::GetConnectState(void)
{
    auto connectState = BluetoothStateManager::GetInstance()->GetProfileConnectState(PROFILE_NAME_GATT_SERVER);
    return (connectState == BTConnectState::CONNECTED ? PROFILE_STATE_CONNECTED : PROFILE_STATE_DISCONNECTED);
}

int GattServerService::GetMaxConnectNum(void)
{
    return 0;
}

int GattServerService::Connect(const RawAddress &device)
{
    return 0;
}

int GattServerService::Disconnect(const RawAddress &device)
{
    return 0;
}

void GattServerService::Enable(void)
{
    if (pimpl->InRunningState()) {
        HILOGE("GattServerService is already start up.");
        return;
    }
    // Switch to inner thread
    DoInGattThread(
        [this]() {
            pimpl->Start();
            GetContext()->OnEnable(PROFILE_NAME_GATT_SERVER, true);
        });
}

void GattServerService::Disable(void)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is already shut down.");
        return;
    }

    DoInGattThread(
        [this]() {
            pimpl->Stop();
            GetContext()->OnDisable(PROFILE_NAME_GATT_SERVER, true);
            for (std::shared_ptr<GattServerApplication> server : pimpl->servers) {
                BluetoothGattInterface::GetInstance()->RemoveGattServerObserver(server);
            }
            // Clear application
            pimpl->servers.clear();
            std::lock_guard<std::mutex> lock(pimpl->appIdMutex);
            pimpl->appIds.clear();
        });
}

// called in jni thread
void GattServerService::impl::RegisterServerCallback(int status, int serverIf)
{
    HILOGI("status: %{public}d, appId: %{public}d", status, serverIf);
    // serverIf now is in (0, 0xFF)
    int ret = serverIf;
    if (status != GATT_SUCCESS) {
        ret = REGISTER_APP_ID_FAILED;
    }
    std::unique_lock<std::mutex> lock(syncMutex);
    tempAppId = ret;
    cv.notify_all();
}

bool GattServerService::impl::WaitForRegisterCallback(int &outAppId)
{
    // wait RegisterServerCallback called by stack
    std::unique_lock<std::mutex> lock(syncMutex);
    if (!cv.wait_for(lock, std::chrono::seconds(SYNC_TIMEOUT), [this]() { return tempAppId != INVALID_APP_ID; })) {
        HILOGE("register server timeout");
        return false;
    }
    // Check callback report error
    if (tempAppId == REGISTER_APP_ID_FAILED) {
        HILOGE("register server callback report error");
        tempAppId = INVALID_APP_ID; // clear appId status for next register application
        return false;
    }

    outAppId = tempAppId;
    tempAppId = INVALID_APP_ID; // clear appId status for next register application
    return true;
}

// called in inner thread
void GattServerService::impl::RegisterApplicationInThread(std::weak_ptr<IGattServerCallback> callback,
    std::shared_ptr<std::promise<int>> promise, uint32_t tokenId, int32_t pid)
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    std::shared_ptr<GattServerServiceRegisterObserver> obs = std::make_shared<GattServerServiceRegisterObserver>(
        [this](int status, int serverIf) { RegisterServerCallback(status, serverIf); });
    BluetoothGattInterface::GetInstance()->AddGattServerObserver(obs);

    auto appUuid = STACK::bluetooth::Uuid::GetRandom();
    if (self.btIfGattServer_) {
        int ret = self.btIfGattServer_->register_server(appUuid, false);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("register server failed, ret: %{public}d", ret);
            promise->set_value(GattStatus::GATT_FAILURE);
            return;
        }

        int appId = 0;
        if (!WaitForRegisterCallback(appId)) {
            HILOGI("failed");
            promise->set_value(GattStatus::GATT_FAILURE);
            return;
        }
        // Create Gatt server application
        std::shared_ptr<GattServerApplication> server = std::make_shared<GattServerApplication>(appId,
            self.btIfGattServer_, tokenId, callback, [this](int appId, const Service &service) {
                NotifyServiceChanged(appId, service);
            });
        BluetoothGattInterface::GetInstance()->AddGattServerObserver(server);
        servers.emplace(servers.end(), server);

        std::lock_guard<std::mutex> appIdLock(appIdMutex);
        appIds.insert_or_assign(appId, pid);

        // Sync
        promise->set_value(appId);
    }
}

// called in server thread
int GattServerService::RegisterApplication(std::weak_ptr<IGattServerCallback> callback)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(GATT_REGISTER_APPLICATION_PKGNAME,
            SensingInfo(Bluetooth::PermissionManager::GetCallingName(), true));
    }

    auto promise = std::make_shared<std::promise<int>>();
    std::future<int> future = promise->get_future();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t pid = IPCSkeleton::GetCallingPid();

    DoInGattThread([this, callback, promise, tokenId, pid]() {
        pimpl->RegisterApplicationInThread(callback, promise, tokenId, pid);
    });

    if (future.wait_for(std::chrono::seconds(impl::SYNC_TIMEOUT)) != std::future_status::ready) {
        HILOGE("timeout");
        return GattStatus::GATT_FAILURE;
    }
    return future.get();
}

int GattServerService::DeregisterApplication(int appId)
{
    return DeregisterApplication(appId, IPCSkeleton::GetCallingPid());
}

int GattServerService::DeregisterApplication(int appId, int32_t pid)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    if (!pimpl->IsValidApplication(appId, pid)) {
        HILOGE("Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
        return GattStatus::INVALID_PARAMETER;
    }
    HILOGI("appId: %{public}d", appId);
    auto promise = std::make_shared<std::promise<int>>();
    std::future<int> future = promise->get_future();

    DoInGattThread([this, appId, promise]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->servers.end()) {
            pimpl->servers.erase(iter);
            std::lock_guard<std::mutex> lock(pimpl->appIdMutex);
            pimpl->appIds.erase(appId);
        }

        promise->set_value(GattStatus::GATT_SUCCESS);
    });

    if (future.wait_for(std::chrono::seconds(impl::SYNC_TIMEOUT)) != std::future_status::ready) {
        HILOGE("timeout");
        return GattStatus::GATT_FAILURE;
    }
    return future.get();
}

int GattServerService::AddService(int appId, Service &service)
{
    ControlInterceptMessage msg {
        .isSystemHap = Bluetooth::PermissionManager::IsSystemHap(),
        .isNativeCaller = Bluetooth::PermissionManager::IsNativeCaller(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattAddService(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    if (!pimpl->IsValidApplication(appId, msg.pid)) {
        HILOGE("Application is invalid, appId: %{public}d, pid: %{public}d", appId, msg.pid);
        return GattStatus::INVALID_PARAMETER;
    }
    HILOGI("appId: %{public}d", appId);
    DoInGattThread(
        [this, appId, service]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->servers.end()) {
                (*iter)->AddService(service);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::RemoveService(int appId, const Service &service)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    int32_t pid = IPCSkeleton::GetCallingPid();
    if (!pimpl->IsValidApplication(appId, pid)) {
        HILOGE("Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
        return GattStatus::INVALID_PARAMETER;
    }
    HILOGI("appId: %{public}d, pid: %{public}d, handle: %{public}#x", appId, pid, service.handle_);
    DoInGattThread(
        [this, appId, handle = service.handle_]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->servers.end()) {
                (*iter)->RemoveService(handle);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::ClearServices(int appId)
{
    return ClearServices(appId, IPCSkeleton::GetCallingPid());
}

int GattServerService::ClearServices(int appId, int32_t pid)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return Bluetooth::BT_ERR_SERVICE_DISCONNECTED;
    }
    if (!pimpl->IsValidApplication(appId, pid)) {
        HILOGE("Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    HILOGI("appId: %{public}d", appId);
    DoInGattThread(
        [this, appId]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->servers.end()) {
                (*iter)->ClearServices();
            }
        });

    return GattStatus::GATT_SUCCESS;
}

bool GattServerService::impl::IsValidApplication(int appId, int32_t pid)
{
    std::lock_guard<std::mutex> lock(appIdMutex);
    auto iter = appIds.find(appId);
    return iter != appIds.end() && iter->second == pid;
}

void GattServerService::impl::NotifyServiceChanged(int appId, const Service &service)
{
    for (auto iter = servers.begin(); iter != servers.end(); iter++) {
        if ((*iter)->GetAppId() == appId) {
            continue;
        }
        (*iter)->ReportServiceChanged(service);
    }
}

int GattServerService::Connect(int appId, const GattDevice &device, bool isDirect)
{
    CHECK_AND_RETURN_LOG_RET(pimpl->InRunningState(), REQUEST_NOT_SUPPORT, "GattServerService is not start up.");
    int32_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_LOG_RET(pimpl->IsValidApplication(appId, pid), INVALID_PARAMETER,
        "Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
    ControlInterceptMessage msg {
        .addr = device.addr_.GetAddress(),
        .isSystemHap = Bluetooth::PermissionManager::IsSystemHap(),
        .isNativeCaller = Bluetooth::PermissionManager::IsNativeCaller(),
    };
    if (!ControlInterceptIsAllowedGattConn(msg)) {
        return REQUEST_NOT_SUPPORT;
    }
    HILOGI("appId: %{public}d", appId);
    DoInGattThread(
        [this, appId, isDirect, addr = device.addr_]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->servers.end()) {
                (*iter)->Connect(addr, isDirect);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::CancelConnection(int appId, const GattDevice &device)
{
    CHECK_AND_RETURN_LOG_RET(pimpl->InRunningState(), REQUEST_NOT_SUPPORT, "GattServerService is not start up.");
    int32_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_LOG_RET(pimpl->IsValidApplication(appId, pid), INVALID_PARAMETER,
        "Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
    HILOGI("appId: %{public}d", appId);
    DoInGattThread(
        [this, appId, addr = device.addr_]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->servers.end()) {
                (*iter)->CancelConnection(addr);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::RespondCharacteristicRead(const GattDevice &device, Characteristic &characteristic, int ret)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = device.addr_.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattWrite(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    HILOGI("handle: %{public}#x, ret: %{public}d, length: %{public}zu",
        characteristic.handle_, ret, characteristic.length_);
    const uint8_t *p = characteristic.value_.get();
    std::vector<uint8_t> value(p, p + characteristic.length_);
    DoInGattThread(
        [this, ret, addr = device.addr_, handle = characteristic.handle_, value = std::move(value)]() {
            auto iter = pimpl->GetApplicationIter(handle);
            if (iter != pimpl->servers.end()) {
                (*iter)->RespondCharacteristicRead(addr, handle, std::move(value), ret);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::RespondCharacteristicReadByUuid(const GattDevice &device, Characteristic &characteristic,
    int ret)
{
    HILOGI("Unsupported");
    return GattStatus::GATT_FAILURE;
}

int GattServerService::RespondDescriptorRead(const GattDevice &device, Descriptor &descriptor, int ret)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = device.addr_.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattWrite(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    HILOGI("handle: %{public}#x, ret: %{public}d, length: %{public}zu", descriptor.handle_, ret, descriptor.length_);
    const uint8_t *p = descriptor.value_.get();
    std::vector<uint8_t> value(p, p + descriptor.length_);
    DoInGattThread(
        [this, ret, addr = device.addr_, handle = descriptor.handle_, value = std::move(value)]() {
            auto iter = pimpl->GetApplicationIter(handle);
            if (iter != pimpl->servers.end()) {
                (*iter)->RespondDescriptorRead(addr, handle, std::move(value), ret);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::RespondCharacteristicWrite(const GattDevice &device, const Characteristic &characteristic,
    int ret)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = device.addr_.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattWrite(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    HILOGI("handle: %{public}#x, ret: %{public}d", characteristic.handle_, ret);
    DoInGattThread(
        [this, ret, addr = device.addr_, handle = characteristic.handle_]() {
            auto iter = pimpl->GetApplicationIter(handle);
            if (iter != pimpl->servers.end()) {
                (*iter)->RespondCharacteristicWrite(addr, handle, ret);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::RespondDescriptorWrite(const GattDevice &device, const Descriptor &descriptor, int ret)
{
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = device.addr_.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattWrite(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    HILOGI("handle: %{public}#x, ret: %{public}d", descriptor.handle_, ret);
    DoInGattThread(
        [this, ret, addr = device.addr_, handle = descriptor.handle_]() {
            auto iter = pimpl->GetApplicationIter(handle);
            if (iter != pimpl->servers.end()) {
                (*iter)->RespondDescriptorWrite(addr, handle, ret);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

void GattServerService::impl::NotifyClient(const GattDevice &device, uint16_t handle, std::vector<uint8_t> value)
{
    DoInGattThread(
        [this, handle, device, value = std::move(value)]() {
            auto iter = GetApplicationIter(handle);
            if (iter != servers.end()) {
                (*iter)->NotifyClient(device.addr_, handle, std::move(value));
            }
        });
}

void GattServerService::impl::NotifyClientNoCfm(const GattDevice &device, uint16_t handle, std::vector<uint8_t> value)
{
    // It need lock because this function will be called in multi thread.
    std::lock_guard<std::mutex> lock(notifyNoCfmMutex);

    auto semaphore = std::make_shared<utility::Semaphore>(0);
    DoInGattThread(
        [this, handle, device, semaphore, value = std::move(value)]() {
            auto iter = GetApplicationIter(handle);
            if (iter != servers.end()) {
                (*iter)->NotifyClientNoCfm(device.addr_, handle, std::move(value), semaphore);
            }
        });

    if (!semaphore->WaitForMs(WAIT_TIMEOUT_MS)) {
        HILOGE("timeout");
    }
}

int GattServerService::NotifyClient(const GattDevice &device, Characteristic &characteristic, bool needConfirm)
{
    HILOGD("handle: %{public}#x, needConfirm: %{public}d", characteristic.handle_, needConfirm);
    if (!pimpl->InRunningState()) {
        HILOGE("GattServerService is not start up.");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    const uint8_t *p = characteristic.value_.get();
    std::vector<uint8_t> value(p, p + characteristic.length_);
    if (needConfirm) {
        pimpl->NotifyClient(device, characteristic.handle_, std::move(value));
    } else {
        pimpl->NotifyClientNoCfm(device, characteristic.handle_, std::move(value));
    }

    return GattStatus::GATT_SUCCESS;
}

int GattServerService::GetConnectedState(const RawAddress &address)
{
    const std::string deviceId = address.GetAddress();
    std::vector<std::string> clientDevices = BluetoothStateManager::GetInstance()->GetClientDeviceList();
    for (const std::string &clientDevice : clientDevices) {
        if (clientDevice == deviceId) {
            return static_cast<int>(BTConnectState::CONNECTED);
        }
    }
    return static_cast<int>(BTConnectState::DISCONNECTED);
}

int GattServerService::SetPhy(int32_t appId,
    const RawAddress &address, int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    const std::string deviceId = address.GetAddress();
    if (!bluetooth::IsValidAddr(deviceId)) {
        HILOGE("Invalid deviceId: %{public}s", GetEncryptAddr(deviceId).c_str());
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (GetConnectedState(address) == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGE("Request not supported");
        return Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED;
    }

    HILOGI("appId: %{public}d, addr: %{public}s", appId, GetEncryptAddr(deviceId).c_str());
    if (phyOptions < static_cast<int32_t>(BLE_PHY_CODED_NO_PREFERRED) ||
        phyOptions > static_cast<int32_t>(BLE_PHY_CODED_S8)) {
        HILOGE("Invalid parameters");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    if ((txPhy < static_cast<int32_t>(BLE_PHY_1M) || txPhy > static_cast<int32_t>(BLE_PHY_CODED)) ||
        (rxPhy < static_cast<int32_t>(BLE_PHY_1M) || rxPhy > static_cast<int32_t>(BLE_PHY_CODED))) {
        HILOGE("Invalid parameters");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }

    CHECK_AND_RETURN_LOG_RET(pimpl->InRunningState(), REQUEST_NOT_SUPPORT, "GattServerService is not start up.");
    int32_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_LOG_RET(pimpl->IsValidApplication(appId, pid), INVALID_PARAMETER,
        "Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
    DoInGattThread([this, appId, address, txPhy, rxPhy, phyOptions]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->servers.end()) {
            (*iter)->SetPhy(address, txPhy, rxPhy, phyOptions);
        }
    });
    return GattStatus::GATT_SUCCESS;
}
 
int GattServerService::ReadPhy(int32_t appId, const RawAddress &address)
{
    const std::string deviceId = address.GetAddress();
    if (!bluetooth::IsValidAddr(deviceId)) {
        HILOGE("Invalid deviceId: %{public}s", GetEncryptAddr(deviceId).c_str());
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (GetConnectedState(address) == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGE("Request not supported");
        return Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED;
    }

    HILOGI("appId: %{public}d, addr: %{public}s", appId, GetEncryptAddr(deviceId).c_str());
    CHECK_AND_RETURN_LOG_RET(pimpl->InRunningState(), REQUEST_NOT_SUPPORT, "GattServerService is not start up.");
    int32_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_LOG_RET(pimpl->IsValidApplication(appId, pid), INVALID_PARAMETER,
        "Application is invalid, appId: %{public}d, pid: %{public}d", appId, pid);
    DoInGattThread([this, appId, address]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->servers.end()) {
            (*iter)->ReadPhy(address);
        }
    });
    return GattStatus::GATT_SUCCESS;
}

REGISTER_CLASS_CREATOR(GattServerService);
}  // namespace bluetooth
}  // namespace OHOS
