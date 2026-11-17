/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "obexserversession_fuzzer.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <future>

#include "adapter_device_config.h"
#include "permission_manager.h"
#include "bluetooth_log.h"
#include "obex_def.h"
#include "obex_server_session.h"
#include "../../bt_utils/fuzztest_utils.h"
#include "fuzzer/FuzzedDataProvider.h"

#undef private
#define private public
#include "map_mse_service_impl.h"
#include "pbap_pse_service_impl.h"
#include "thread_util.h"

static std::shared_ptr<OHOS::bluetooth::MapMseServiceImpl> g_mapMseInstance {nullptr};
static std::shared_ptr<OHOS::bluetooth::PbapPseServiceImpl> g_pbapPseInstance {nullptr};

static const int WAIT_THREAD_TIME_SECOND = 10;
static const int WAIT_READ_TIME_MILLISECOND = 200;
const int FUZZ_DATA_LEN = 10;

using ObexTestInitFunc = std::function<void(std::shared_ptr<OHOS::bluetooth::ObexSocketDevice>)>;
using ObexTestDeInitFunc = std::function<void(std::shared_ptr<OHOS::bluetooth::ObexSocketDevice>)>;
using ObexTestDataFunc = std::function<void(int)>;
struct ObtexTestFuncs {
    ObexTestInitFunc initFunc {nullptr};
    ObexTestDeInitFunc deinitFunc {nullptr};
    ObexTestDataFunc dataFunc {nullptr};
};

namespace OHOS {
namespace bluetooth {
    static void WaitThreadClosed(int threadId)
    {
        auto promise = std::make_shared<std::promise<void>>();
        std::future<void> future = promise->get_future();

        auto func = [promise]() {
            promise->set_value();
        };

        ThreadUtil::GetInstance().ChangeThreadState(threadId, ThreadUtil::ENABLED);
        ThreadUtil::GetInstance().PostTask(threadId, func);
        ThreadUtil::GetInstance().ChangeThreadState(threadId, ThreadUtil::DISABLED);

        if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIME_SECOND)) != std::future_status::ready) {
            HILOGE("timeout");
            return;
        }
    }

    void ObexSocketFuzzTest(FuzzedDataProvider& provider, ObtexTestFuncs funcs)
    {
        int fds[2] = {-1, -1};
        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) == -1) {
            HILOGE("error creating socketpair: %s", strerror(errno));
            return;
        }
        int fdReader = fds[0];
        int fdWriter = fds[1];
        std::shared_ptr<ObexSocketDevice> socketDevice = std::make_shared<ObexSocketDevice>(
            fdReader, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, "11:22:33:44:55:66");

        funcs.initFunc(socketDevice);

        // 构造业务OpCode
        std::vector<uint8_t> opcodeVec {
            OBEX_OPCODE_CONNECT, OBEX_OPCODE_DISCONNECT, OBEX_OPCODE_PUT, OBEX_OPCODE_PUT_FINAL, OBEX_OPCODE_GET,
            OBEX_OPCODE_GET_FINAL, OBEX_OPCODE_RESERVED, OBEX_OPCODE_RESERVED_FINAL, OBEX_OPCODE_SETPATH,
            OBEX_OPCODE_SESSION, OBEX_OPCODE_ABORT, OBEX_OPCODE_UNIMPLEMENTED
        };
        uint8_t idx = provider.ConsumeIntegral<uint8_t>() % opcodeVec.size();
        send(fdWriter, &opcodeVec[idx], 1, 0);

        // 构造长度字段
        if (provider.remaining_bytes() >= 2) {  // 2 is the length field
            uint16_t len = provider.ConsumeIntegral<uint16_t>();
            len = len % (MAX_PACKET_SIZE + 1);

            // 发送长度字段
            uint8_t lenHigh = len >> 8;
            uint8_t lenLow = len & 0xFF;
            send(fdWriter, &lenHigh, 1, 0);
            send(fdWriter, &lenLow, 1, 0);
            // 发送剩余数据字段
            std::vector<uint8_t> remainingData = provider.ConsumeRemainingBytes<uint8_t>();
            send(fdWriter, remainingData.data(), remainingData.size(), 0);
        }

        funcs.dataFunc(fdWriter);

        // 等待业务线程从socket里读取数据
        int sleepMs = 5;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));

        WaitThreadClosed(THREAD_ID_MAP);
        WaitThreadClosed(THREAD_ID_PBAP);
        shutdown(fdWriter, SHUT_RDWR);
        close(fdWriter);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_READ_TIME_MILLISECOND));
        funcs.deinitFunc(socketDevice);
    }

    void MapFuzzTestInit(std::shared_ptr<ObexSocketDevice> socketDevice)
    {
        HILOGI("============= map_fuzz_test start ================");
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_MAP, ThreadUtil::ENABLED);
        MapMnsSdpFoundRecord sdpRecord;
        g_mapMseInstance = std::make_shared<MapMseServiceImpl>();
        g_mapMseInstance->StartUp(sdpRecord, socketDevice, false);
    }

    void MapFuzzTestDeinit(std::shared_ptr<ObexSocketDevice> socketDevice)
    {
        g_mapMseInstance->masInstanceMap_.Iterate([](int32_t key, std::shared_ptr<MapMseMasInstance> &instance) {
            instance->StopObexServerSession();
        });
        g_mapMseInstance = nullptr;
        HILOGI("============= map_fuzz_test end ================");
    }

    void PbapFuzzTestInit(std::shared_ptr<ObexSocketDevice> socketDevice)
    {
        HILOGI("============= pbap_fuzz_test start ================");
        ThreadUtil::GetInstance().ChangeThreadState(THREAD_ID_PBAP, ThreadUtil::ENABLED);
        SetPbapPermission(socketDevice->mDeviceAddress, static_cast<int>(BTPermissionType::ACCESS_ALLOWED));
        g_pbapPseInstance = std::make_shared<PbapPseServiceImpl>();
        g_pbapPseInstance->HandlerConnectEvt(socketDevice);
    }

    void PbapFuzzTestDeinit(std::shared_ptr<ObexSocketDevice> socketDevice)
    {
        RawAddress addr(socketDevice->mDeviceAddress);
        g_pbapPseInstance->Disconnect(addr);
        g_pbapPseInstance = nullptr;
        HILOGI("============= pbap_fuzz_test end ================");
    }

    bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider& provider)
    {
        ObtexTestFuncs mapFuncs = {
            .initFunc = MapFuzzTestInit,
            .deinitFunc = MapFuzzTestDeinit,
            .dataFunc = [](int fdWriter) {},
        };
        ObexSocketFuzzTest(provider, mapFuncs);

        ObtexTestFuncs pbapFuncs = {
            .initFunc = PbapFuzzTestInit,
            .deinitFunc = PbapFuzzTestDeinit,
            .dataFunc = [](int fdWriter) {},
        };
        ObexSocketFuzzTest(provider, pbapFuncs);
        return true;
    }

    bool PbapPpseServiceImplTest(FuzzedDataProvider& provider)
    {
        std::shared_ptr<PbapPseServiceImpl> pbapPseServiceImpl = std::make_shared<PbapPseServiceImpl>();
        std::shared_ptr<IPbapPseObserver> observer;
        pbapPseServiceImpl->RegisterObserver(observer);
        std::vector<int32_t> states = {0, 1, 2};
        pbapPseServiceImpl->GetDevicesByStates(states);
        auto addr = Bluetooth::BuildAddressString(provider);
        RawAddress device(addr);
        pbapPseServiceImpl->GetDeviceState(device);
        pbapPseServiceImpl->GetConnectState();
        int32_t accessAuthorization = static_cast<int>(BTPermissionType::ACCESS_FORBIDDEN);
        pbapPseServiceImpl->SetPhoneBookAccessAuthorization(device, accessAuthorization);
        accessAuthorization = static_cast<int>(BTPermissionType::ACCESS_ALLOWED);
        pbapPseServiceImpl->SetPhoneBookAccessAuthorization(device, accessAuthorization);
        PbapPseMessage event(PBAP_PSE_DISCONNECT_EVT);
        event.dev_ = "test";
        pbapPseServiceImpl->ProcessReqPermissionTimeOut(event);
        pbapPseServiceImpl->DeregisterObserver(observer);
        return true;
    }

    bool MapMseServiceImplTest(FuzzedDataProvider& provider)
    {
        std::shared_ptr<MapMseServiceImpl> mapMseServiceImpl = std::make_shared<MapMseServiceImpl>();
        std::shared_ptr<IMapMseObserver> observer;
        mapMseServiceImpl->RegisterObserver(observer);
        auto addr = Bluetooth::BuildAddressString(provider);
        RawAddress device(addr);
        mapMseServiceImpl->Disconnect(device);
        mapMseServiceImpl->GetConnectDevices();
        mapMseServiceImpl->GetDeviceState(device);
        mapMseServiceImpl->GetConnectState();
        mapMseServiceImpl->DeregisterObserver(observer);
        return true;
    }
}  // namespace bluetooth
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // 数据大小至少为1
    if (size < 1) {
        return 0;
    }
    /* Run your code on data */
    FuzzedDataProvider provider(data, size);
    OHOS::Bluetooth::PermissionManager::SetAllGranted();
    OHOS::bluetooth::DoSomethingInterestingWithMyAPI(provider);
    OHOS::bluetooth::PbapPpseServiceImplTest(provider);
    OHOS::bluetooth::MapMseServiceImplTest(provider);
    return 0;
}