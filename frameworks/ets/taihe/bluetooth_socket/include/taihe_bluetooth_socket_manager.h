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

#ifndef TAIHE_BLUETOOTH_SOCKET_MANAGER_H_
#define TAIHE_BLUETOOTH_SOCKET_MANAGER_H_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "bluetooth_socket.h"
#include "bluetooth_socket_inputstream.h"
#include "bluetooth_socket_outputstream.h"
#include "bluetooth_log.h"
#include "taihe/array.hpp"
#include "taihe/callback.hpp"
#include "taihe/optional.hpp"

namespace OHOS {
namespace Bluetooth {

constexpr int RFCOMM_SOCKET_BUFFER_SIZE = 1024;
constexpr int L2CAP_SOCKET_BUFFER_SIZE = 65535;
constexpr int SPP_ACCEPT_TIMEOUT = 20;

struct TaiheSppClient {
    std::shared_ptr<ClientSocket> client_;
    std::atomic<bool> readFlag_{false};
    std::thread readThread_;
    std::vector<::taihe::optional<::taihe::callback<
        void(::taihe::array_view<uint8_t> data)>>> sppReadVec_ = {};
    std::shared_mutex sppReadLock_;

    ~TaiheSppClient()
    {
        StopReadThread();
    }

    void StartReadThread()
    {
        // OnSppRead guarantees readFlag_ was false before re-starting, join the
        // previous thread (if any) to keep at most one read thread per client.
        if (readThread_.joinable()) {
            readThread_.join();
        }
        readThread_ = std::thread([this]() {
            ReadLoop();
        });
    }

    void ReadLoop()
    {
        if (client_ == nullptr) {
            HILOGE("thread start failed, client_ is nullptr");
            readFlag_ = false;
            return;
        }
        auto inputStream = client_->GetInputStream();
        if (inputStream == nullptr) {
            HILOGE("thread start failed, inputStream is nullptr");
            readFlag_ = false;
            return;
        }
        BtSocketType type = client_->GetType();
        std::vector<uint8_t> buf;
        if (type == TYPE_RFCOMM) {
            buf.resize(RFCOMM_SOCKET_BUFFER_SIZE);
        } else {
            buf.resize(client_->GetMaxReceivePacketSize());
        }
        while (readFlag_) {
            std::fill(buf.begin(), buf.end(), 0);
            ssize_t ret = inputStream->Read(buf.data(), buf.size());
            if (ret <= 0) {
                HILOGE("inputStream.Read failed, ret = %{public}zd", ret);
                readFlag_ = false;
                return;
            }
            NotifyReadData(buf, ret);
        }
    }

    void NotifyReadData(std::vector<uint8_t> &buf, ssize_t ret)
    {
        if (!readFlag_) {
            return;
        }
        // Copy the callbacks out of the lock, the callback may re-enter
        // OnSppRead/OffSppRead which requires the unique lock.
        std::vector<::taihe::optional<::taihe::callback<
            void(::taihe::array_view<uint8_t> data)>>> callbacks;
        {
            std::shared_lock<std::shared_mutex> guard(sppReadLock_);
            callbacks = sppReadVec_;
        }
        auto data = ::taihe::array_view<uint8_t>(buf.data(), static_cast<size_t>(ret));
        for (auto callback : callbacks) {
            if (callback) {
                (*callback)(data);
            }
        }
    }

    void StopReadThread()
    {
        readFlag_ = false;
        if (readThread_.joinable()) {
            readThread_.join();
        }
    }
};

class TaiheSocketManager {
public:
    static TaiheSocketManager& GetInstance()
    {
        static TaiheSocketManager instance;
        return instance;
    }

    int32_t AddServer(std::shared_ptr<ServerSocket> server)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        int32_t id = serverCount_++;
        serverMap_[id] = server;
        return id;
    }

    bool HasServer(int32_t id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return serverMap_.find(id) != serverMap_.end();
    }

    std::shared_ptr<ServerSocket> GetServer(int32_t id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = serverMap_.find(id);
        if (it != serverMap_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void RemoveServer(int32_t id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        serverMap_.erase(id);
    }

    int32_t AddClient(std::shared_ptr<ClientSocket> client)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        int32_t id = clientCount_++;
        auto sppClient = std::make_shared<TaiheSppClient>();
        sppClient->client_ = client;
        clientMap_[id] = sppClient;
        return id;
    }

    bool HasClient(int32_t id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return clientMap_.find(id) != clientMap_.end();
    }

    std::shared_ptr<TaiheSppClient> GetClient(int32_t id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = clientMap_.find(id);
        if (it != clientMap_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void RemoveClient(int32_t id)
    {
        std::shared_ptr<TaiheSppClient> client;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = clientMap_.find(id);
            if (it == clientMap_.end()) {
                return;
            }
            client = it->second;
            clientMap_.erase(it);
        }
        // Out of the manager lock: joining the read thread must not hold
        // mutex_, the read callbacks may re-enter OnSppRead/OffSppRead.
        client->StopReadThread();
        {
            std::unique_lock<std::shared_mutex> guard(client->sppReadLock_);
            client->sppReadVec_.clear();
        }
    }

private:
    TaiheSocketManager() = default;
    ~TaiheSocketManager() = default;
    TaiheSocketManager(const TaiheSocketManager&) = delete;
    TaiheSocketManager& operator=(const TaiheSocketManager&) = delete;

    std::mutex mutex_;
    std::map<int32_t, std::shared_ptr<ServerSocket>> serverMap_;
    std::map<int32_t, std::shared_ptr<TaiheSppClient>> clientMap_;
    int32_t serverCount_ = 0;
    int32_t clientCount_ = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_BLUETOOTH_SOCKET_MANAGER_H_
