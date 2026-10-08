/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_taihe_socket_impl"
#endif

#include "ohos.bluetooth.socket.impl.hpp"
#include "ohos.bluetooth.socket.proj.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_socket.h"
#include "bluetooth_socket_inputstream.h"
#include "bluetooth_socket_outputstream.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"
#include "uuid.h"
#include "taihe_bluetooth_socket_manager.h"

#include <algorithm>

using namespace OHOS::Bluetooth;


namespace {
struct SppOptionValues {
    std::string uuid;
    bool secure;
    BtSocketType type;
    int psm;
    bool valid;
};

SppOptionValues ConvertSppOptions(const ::ohos::bluetooth::socket::SppOptions& options)
{
    SppOptionValues v;
    v.uuid = std::string(options.uuid);
    v.secure = options.secure;
    v.type = static_cast<BtSocketType>(options.type.get_value());
    v.psm = options.psm.has_value() ? options.psm.value() : -1;
    v.valid = true;
    if (!v.uuid.empty() && !IsValidUuid(v.uuid)) {
        HILOGE("Invalid uuid: %{public}s", v.uuid.c_str());
        v.valid = false;
    }
    if ((v.type == TYPE_L2CAP || v.type == TYPE_L2CAP_LE) && v.psm <= 0) {
        HILOGE("L2CAP/L2CAP_LE type but psm <= 0");
        v.valid = false;
    }
    if (v.type == TYPE_RFCOMM && v.uuid.empty()) {
        HILOGE("RFCOMM type but uuid is empty");
        v.valid = false;
    }
    return v;
}

int32_t SppAcceptSync(int32_t serverSocket)
{
    HILOGI("enter, serverSocket: %{public}d", serverSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppAccept", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasServer(serverSocket),
        BT_ERR_INVALID_PARAM, -1);
    auto server = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetServer(serverSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server != nullptr, BT_ERR_INVALID_PARAM, -1);

    // 本函数由 STS 层 taskpool 包装调用（阻塞在 taskpool 线程，不卡 JS 线程）；
    // 失败时 set_business_error 抛出的 BusinessError 经 taskpool catch 送达 callback
    auto client = server->Accept(OHOS::Bluetooth::SPP_ACCEPT_TIMEOUT);
    if (client == nullptr) {
        HILOGE("Operation failed. Please wait for SPP connection"
               " before other API calls.");
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(false, BT_ERR_INTERNAL_ERROR, -1);
    }

    return OHOS::Bluetooth::TaiheSocketManager::GetInstance().AddClient(client);
}

int32_t SppListenSync(::taihe::string_view name, ::ohos::bluetooth::socket::SppOptions const& options)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppListen", validErrCodes);

    auto opt = ConvertSppOptions(options);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(opt.valid, BT_ERR_INVALID_PARAM, -1);

    std::string nameStr = std::string(name);
    auto server = std::make_shared<OHOS::Bluetooth::ServerSocket>(nameStr,
        UUID::FromString(opt.uuid), opt.type, opt.secure, opt.psm);
    int32_t errorCode = server->Listen();
    HILOGI("Listen errorCode: %{public}d", errorCode);
    // Listen 返回成功但GetStringTag为空时兜底为内部错误，避免静默失败
    if (errorCode == BT_NO_ERROR && server->GetStringTag() == "") {
        errorCode = BT_ERR_INTERNAL_ERROR;
    }
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server->GetStringTag() != "", errorCode, -1);

    HILOGI("SppListen success");
    return OHOS::Bluetooth::TaiheSocketManager::GetInstance().AddServer(server);
}

void SppCloseServerSocket(int32_t socket)
{
    HILOGI("enter, socket: %{public}d", socket);
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppCloseServerSocket", {});
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(socket);
    if (client && client->client_) {
        client->client_->Close();
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().RemoveClient(socket);
    } else {
        HILOGW("no such key in clientMap.");
    }
    auto server = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetServer(socket);
    if (server) {
        server->Close();
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().RemoveServer(socket);
    } else {
        HILOGW("no such key in serverMap.");
    }
}

int32_t SppConnectSync(::taihe::string_view deviceId, ::ohos::bluetooth::socket::SppOptions const& options)
{
    HILOGI("enter");
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE, BT_ERR_PROFILE_DISABLED,
                                           BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppConnect", validErrCodes);

    auto opt = ConvertSppOptions(options);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(opt.valid, BT_ERR_INVALID_PARAM, -1);

    std::string addr = std::string(deviceId);
    BluetoothRemoteDevice device(addr, BT_TRANSPORT_BREDR);
    auto client = std::make_shared<OHOS::Bluetooth::ClientSocket>(device,
        UUID::FromString(opt.uuid), opt.type, opt.secure);
    int32_t errorCode = client->Connect(opt.psm);
    HILOGI("Connect errorCode: %{public}d", errorCode);
    if (errorCode == BT_ERR_SPP_CONNECT_FAILED) {
        errorCode = BT_ERR_INTERNAL_ERROR;
    }
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(errorCode == BT_NO_ERROR, errorCode, -1);

    HILOGI("SppConnect successfully");
    return OHOS::Bluetooth::TaiheSocketManager::GetInstance().AddClient(client);
}


void SppWrite(int32_t clientSocket, ::taihe::array_view<uint8_t> data)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SPP_IO, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppWrite", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INTERNAL_ERROR);
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client->client_ != nullptr, BT_ERR_INTERNAL_ERROR);

    auto outputStream = client->client_->GetOutputStream();
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(outputStream != nullptr, BT_ERR_INTERNAL_ERROR);
    uint8_t* buf = data.data();
    size_t totalSize = data.size();
    while (totalSize) {
        int result = outputStream->Write(buf, totalSize);
        // The framework must not write more than requested, guard the size_t wrap-around.
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
            result > 0 && static_cast<size_t>(result) <= totalSize, BT_ERR_SPP_IO);
        totalSize -= static_cast<size_t>(result);
        buf += static_cast<size_t>(result);
    }
}

void SppCloseClientSocket(int32_t socket)
{
    HILOGI("enter, socket: %{public}d", socket);
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppCloseClientSocket", {});
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(socket);
    if (client && client->client_) {
        client->client_->Close();
    } else {
        HILOGW("no such key in clientMap.");
    }
    OHOS::Bluetooth::TaiheSocketManager::GetInstance().RemoveClient(socket);
}

int32_t GetMaxReceiveDataSize(int32_t clientSocket)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.GetMaxReceiveDataSize", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM, -1);
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM, -1);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM, -1);

    return static_cast<int32_t>(client->client_->GetMaxReceivePacketSize());
}

void OnSppRead(int32_t clientSocket, ::taihe::callback_view<void(::taihe::array_view<uint8_t> data)> callback)
{
    HILOGI("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SPP_IO, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.OnSppRead", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM);
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM);

    bool expected = false;
    if (!client->readFlag_.compare_exchange_strong(expected, true)) {
        HILOGE("client is reading, please off first");
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(false, BT_ERR_INTERNAL_ERROR);
    }
    {
        std::unique_lock<std::shared_mutex> guard(client->sppReadLock_);
        auto eventCb = ::taihe::optional<::taihe::callback<
            void(::taihe::array_view<uint8_t> data)>>{std::in_place_t{}, callback};
        if (std::find(client->sppReadVec_.begin(), client->sppReadVec_.end(), eventCb)
            != client->sppReadVec_.end()) {
            client->readFlag_ = false;
            return;
        }
        client->sppReadVec_.emplace_back(eventCb);
    }
    client->StartReadThread();
}

void OffSppRead(int32_t clientSocket,
    ::taihe::optional_view<::taihe::callback<void(::taihe::array_view<uint8_t> data)>> callback)
{
    HILOGI("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.OffSppRead", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM);
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM);

    client->readFlag_ = false;
    {
        std::unique_lock<std::shared_mutex> guard(client->sppReadLock_);
        if (callback.has_value()) {
            client->sppReadVec_.erase(
                std::remove_if(client->sppReadVec_.begin(), client->sppReadVec_.end(),
                    [callback](const ::taihe::optional<::taihe::callback<
                        void(::taihe::array_view<uint8_t> data)>>& it) {
                        return it == callback;
                    }),
                client->sppReadVec_.end());
        } else {
            client->sppReadVec_.clear();
        }
    }
}

bool IsConnected(int32_t clientSocket)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.IsConnected", {});
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    if (!client || !client->client_) {
        return false;
    }
    return client->client_->IsConnected();
}

int32_t GetL2capPsm(int32_t serverSocket)
{
    HILOGD("enter, serverSocket: %{public}d", serverSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.GetL2capPsm", validErrCodes);

    bool checkRet = CheckSppFdParam(serverSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM, -1);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasServer(serverSocket),
        BT_ERR_INVALID_PARAM, -1);
    auto server = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetServer(serverSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(server != nullptr, BT_ERR_INVALID_PARAM, -1);

    return server->GetL2capPsm();
}

::taihe::string GetDeviceId(int32_t clientSocket)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.GetDeviceId", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM, ::taihe::string{});
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM, ::taihe::string{});
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM, ::taihe::string{});

    BluetoothRemoteDevice remoteDevice = client->client_->GetRemoteDevice();
    return ::taihe::string(remoteDevice.GetDeviceAddr());
}

int32_t GetMaxTransmitDataSize(int32_t clientSocket)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.GetMaxTransmitDataSize", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM, -1);
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM, -1);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM, -1);

    return static_cast<int32_t>(client->client_->GetMaxTransmitPacketSize());
}

void SppWriteAsyncSync(int32_t clientSocket, ::taihe::array_view<uint8_t> data)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SPP_IO, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppWriteAsync", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM);
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM);
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM);

    auto outputStream = client->client_->GetOutputStream();
    TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(outputStream != nullptr, BT_ERR_INTERNAL_ERROR);
    uint8_t* buf = data.data();
    size_t totalSize = data.size();
    while (totalSize) {
        int result = outputStream->Write(buf, totalSize);
        // The framework must not write more than requested, guard the size_t wrap-around.
        TAIHE_BT_HA_ASSERT_RETURN_VOID_VERIFY(
            result > 0 && static_cast<size_t>(result) <= totalSize, BT_ERR_SPP_IO);
        totalSize -= static_cast<size_t>(result);
        buf += static_cast<size_t>(result);
    }
}

::taihe::array<uint8_t> SppReadAsyncSync(int32_t clientSocket)
{
    HILOGD("enter, clientSocket: %{public}d", clientSocket);
    std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                           BT_ERR_SPP_IO, BT_ERR_INTERNAL_ERROR };
    TAIHE_BT_CONTEXT_HA(taihe::get_env(), "socket.SppReadAsync", validErrCodes);

    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(
        OHOS::Bluetooth::TaiheSocketManager::GetInstance().HasClient(clientSocket),
        BT_ERR_INVALID_PARAM, ::taihe::array<uint8_t>{});
    auto client = OHOS::Bluetooth::TaiheSocketManager::GetInstance().GetClient(clientSocket);
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client != nullptr, BT_ERR_INVALID_PARAM, ::taihe::array<uint8_t>{});
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(client->client_ != nullptr, BT_ERR_INVALID_PARAM,
        ::taihe::array<uint8_t>{});

    bool expected = false;
    if (!client->readFlag_.compare_exchange_strong(expected, true)) {
        HILOGE("client is reading, please off first");
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(false, BT_ERR_INVALID_PARAM,
            ::taihe::array<uint8_t>{});
    }
    auto inputStream = client->client_->GetInputStream();
    if (inputStream == nullptr) {
        client->readFlag_ = false;
        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(false, BT_ERR_INTERNAL_ERROR,
            ::taihe::array<uint8_t>{});
    }
    BtSocketType type = client->client_->GetType();
    std::vector<uint8_t> buf;
    if (type == TYPE_RFCOMM) {
        buf.resize(RFCOMM_SOCKET_BUFFER_SIZE);
    } else {
        buf.resize(client->client_->GetMaxReceivePacketSize());
    }
    std::fill(buf.begin(), buf.end(), 0);
    ssize_t ret = inputStream->Read(buf.data(), buf.size());
    client->readFlag_ = false;
    TAIHE_BT_HA_ASSERT_RETURN_VERIFY(ret > 0, BT_ERR_SPP_IO, ::taihe::array<uint8_t>{});

    return ::taihe::array<uint8_t>(::taihe::copy_data, buf.data(), static_cast<size_t>(ret));
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_SppAcceptSync(SppAcceptSync);
TH_EXPORT_CPP_API_SppListenSync(SppListenSync);
TH_EXPORT_CPP_API_SppCloseServerSocket(SppCloseServerSocket);
TH_EXPORT_CPP_API_SppConnectSync(SppConnectSync);
TH_EXPORT_CPP_API_SppWrite(SppWrite);
TH_EXPORT_CPP_API_SppCloseClientSocket(SppCloseClientSocket);
TH_EXPORT_CPP_API_GetMaxReceiveDataSize(GetMaxReceiveDataSize);
TH_EXPORT_CPP_API_OnSppRead(OnSppRead);
TH_EXPORT_CPP_API_OffSppRead(OffSppRead);
TH_EXPORT_CPP_API_IsConnected(IsConnected);
TH_EXPORT_CPP_API_GetL2capPsm(GetL2capPsm);
TH_EXPORT_CPP_API_GetDeviceId(GetDeviceId);
TH_EXPORT_CPP_API_GetMaxTransmitDataSize(GetMaxTransmitDataSize);
TH_EXPORT_CPP_API_SppWriteAsyncSync(SppWriteAsyncSync);
TH_EXPORT_CPP_API_SppReadAsyncSync(SppReadAsyncSync);
// NOLINTEND
