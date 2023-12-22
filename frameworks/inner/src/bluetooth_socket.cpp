/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_utils.h"
#include "bluetooth_socket_proxy.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "raw_address.h"
#include "bluetooth_socket.h"
#include "bluetooth_socket_observer_stub.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
const int LENGTH = 18;
const int MIN_BUFFER_SIZE_TO_SET = 4 * 1024; // 4KB
const int MAX_BUFFER_SIZE_TO_SET = 50 * 1024; // 50KB
const int ADDR_OFFSET = 1; // state(1)
const int TX_OFFSET = 7; // state(1)+addr(6)
const int RX_OFFSET = 9; // state(1)+addr(6)+tx(2)
const int SOCKET_RECV_ADDR_SIZE = 6;
const int SOCKET_RECV_TXRX_SIZE = 2;
const int SOCKET_RECV_CHANNEL_SIZE = 4;
const int SOCKET_RECV_FD_SIZE = 14;
const int SOCKET_RECV_FD_SIGNAL = 11; // state(1)+addr(6)+tx(2)+rx(2)

constexpr char BLUETOOTH_UE_DOMAIN[] = "BLUETOOTH_UE";
std::mutex g_socketProxyMutex;

struct ClientSocket::impl {
    impl(const BluetoothRemoteDevice &addr, UUID uuid, BtSocketType type, bool auth);
    impl(int fd, std::string address, BtSocketType type);
    impl(const BluetoothRemoteDevice &addr, UUID uuid, BtSocketType type, bool auth,
        std::shared_ptr<BluetoothConnectionObserver> observer);
    ~impl()
    {
        if (fd_ > 0) {
            shutdown(fd_, SHUT_RD);
            shutdown(fd_, SHUT_WR);
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
                HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "close", "ID", fd_, "ADDRESS", "empty",
                "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());
            HiSysEventWrite(BLUETOOTH_UE_DOMAIN, "SOCKET_DISCONN", HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "PNAMEID", "Bluetooth", "PVERSIONID", "1.0", "DEV_ADDRESS", GetEncryptAddr(address_),
                "SCENE_CODE", fd_);
            HILOGI("fd closed, fd_: %{public}d", fd_);
            close(fd_);
            fd_ = -1;
        }
    }

    void Close()
    {
        HILOGD("enter");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGW("The socketStatus_ is already SOCKET_CLOSED");
            return;
        } else {
            socketStatus_ = SOCKET_CLOSED;
            if (fd_ > 0) {
                shutdown(fd_, SHUT_RD);
                shutdown(fd_, SHUT_WR);
                HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
                    HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "close", "ID", fd_, "ADDRESS", "empty",
                    "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());
                HiSysEventWrite(BLUETOOTH_UE_DOMAIN, "SOCKET_DISCONN", HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                    "PNAMEID", "Bluetooth", "PVERSIONID", "1.0", "DEV_ADDRESS", GetEncryptAddr(address_),
                    "SCENE_CODE", fd_);
                HILOGI("fd closed, fd_: %{public}d", fd_);
                close(fd_);
                fd_ = -1;
                sptr<IBluetoothSocket> proxy = GetRemoteProxy<IBluetoothSocket>(PROFILE_SOCKET);
                CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
                bluetooth::Uuid tempUuid = bluetooth::Uuid::ConvertFrom128Bits(uuid_.ConvertTo128Bits());
                proxy->DeregisterClientObserver(BluetoothRawAddress(remoteDevice_.GetDeviceAddr()), tempUuid,
                    observerImp_);
            } else {
                HILOGE("socket not created");
                return;
            }
        }
    }

    uint16_t GetPacketSizeFromBuf(const uint8_t recvBuf[], int recvBufLen) const
    {
        uint16_t shortBuf;
        CHECK_AND_RETURN_LOG_RET(recvBuf, 0, "getpacketsize fail, invalid recvBuf");
        CHECK_AND_RETURN_LOG_RET(recvBufLen >= SOCKET_RECV_TXRX_SIZE, 0, "getpacketsize fail, invalid recvBufLen");
        CHECK_AND_RETURN_LOG_RET(memcpy_s(&shortBuf, sizeof(shortBuf), &recvBuf[0], sizeof(shortBuf)) == EOK, 0,
            "getpacketsize failed, memcpy_s fail");
        return shortBuf;
    }

    bool RecvSocketSignal()
    {
        uint8_t signalBuf[SOCKET_RECV_FD_SIGNAL] = {0};
#ifdef DARWIN_PLATFORM
        int recvBufSize = recv(fd_, signalBuf, sizeof(signalBuf), 0);
#else
        int recvBufSize = recv(fd_, signalBuf, sizeof(signalBuf), MSG_WAITALL);
#endif
        CHECK_AND_RETURN_LOG_RET(recvBufSize == SOCKET_RECV_FD_SIGNAL, false, "recv signal error, service closed");
        bool state = signalBuf[0];
        // remote addr has been obtained, no need obtain again
        maxTxPacketSize_ = GetPacketSizeFromBuf(signalBuf + TX_OFFSET, SOCKET_RECV_FD_SIGNAL - TX_OFFSET);
        maxRxPacketSize_ = GetPacketSizeFromBuf(signalBuf + RX_OFFSET, SOCKET_RECV_FD_SIGNAL - RX_OFFSET);

        return state;
    }

    int getSecurityFlags()
    {
        int flags = 0;
        if (auth_) {
            flags |= FLAG_AUTH;
            flags |= FLAG_ENCRYPT;
        }
        return flags;
    }

    InputStream &GetInputStream()
    {
        HILOGD("enter");
        if (inputStream_ == nullptr) {
            HILOGE("inputStream is NULL, failed. please Connect");
        }
        return *inputStream_;
    }

    OutputStream &GetOutputStream()
    {
        HILOGD("enter");
        if (outputStream_ == nullptr) {
            HILOGE("outputStream is NULL, failed. please Connect");
        }
        return *outputStream_;
    }

    BluetoothRemoteDevice &GetRemoteDevice()
    {
        HILOGD("enter");
        return remoteDevice_;
    }

    bool IsConnected()
    {
        HILOGD("enter");
        return socketStatus_ == SOCKET_CONNECTED;
    }

    int SetBufferSize(int bufferSize)
    {
        HILOGI("SetBufferSize bufferSize is %{public}d.", bufferSize);
        if (bufferSize < MIN_BUFFER_SIZE_TO_SET || bufferSize > MAX_BUFFER_SIZE_TO_SET) {
            HILOGE("SetBufferSize param is invalid.");
            return RET_BAD_PARAM;
        }

        if (fd_ <= 0) {
            HILOGE("SetBufferSize socket fd invalid.");
            return RET_BAD_STATUS;
        }

        const std::pair<const char*, int> sockOpts[] = {
            {"recvBuffer", SO_RCVBUF},
            {"sendBuffer", SO_SNDBUF},
        };
        for (auto opt : sockOpts) {
            int curSize = 0;
            socklen_t optlen = sizeof(curSize);
            if (getsockopt(fd_, SOL_SOCKET, opt.second, &curSize, &optlen) != 0) {
                HILOGE("SetBufferSize getsockopt %{public}s failed.", opt.first);
                return RET_BAD_STATUS;
            }
            HILOGI("SetBufferSize %{public}s before set size is %{public}d.", opt.first, curSize);

            if (curSize != bufferSize) {
                int setSize = bufferSize / 2;
                if (setsockopt(fd_, SOL_SOCKET, opt.second, &setSize, sizeof(setSize)) != 0) {
                    HILOGE("SetBufferSize setsockopt  %{public}s failed.", opt.first);
                    return RET_BAD_STATUS;
                }

                curSize = 0;
                if (getsockopt(fd_, SOL_SOCKET, opt.second, &curSize, &optlen) != 0) {
                    HILOGE("SetBufferSize after getsockopt %{public}s failed.", opt.first);
                    return RET_BAD_STATUS;
                }
                HILOGI("SetBufferSize %{public}s after set size is %{public}d.", opt.first, curSize);
            }
        }

        return RET_NO_ERROR;
    }

    bool RecvSocketPsmOrScn()
    {
        int channel = 0;
#ifdef DARWIN_PLATFORM
        int recvBufSize = recv(fd_, &channel, sizeof(channel), 0);
#else
        int recvBufSize = recv(fd_, &channel, sizeof(channel), MSG_WAITALL);
#endif
        CHECK_AND_RETURN_LOG_RET(recvBufSize == SOCKET_RECV_CHANNEL_SIZE, false,
            "recv psm or scn error, errno:%{public}d, fd_:%{public}d", errno, fd_);
        CHECK_AND_RETURN_LOG_RET(channel > 0, false, "recv channel error, invalid channel:%{public}d", channel);
        HILOGI("psm or scn = %{public}d, type = %{public}d", channel, type_);
        socketChannel_ = channel;
        return true;
    }

    // user register observer
    std::shared_ptr<BluetoothConnectionObserver> observer_ = nullptr;
    class BluetoothSocketObserverImp;

    // socket observer
    sptr<BluetoothSocketObserverImp> observerImp_ = nullptr;
    std::unique_ptr<InputStream> inputStream_ {
        nullptr
    };
    std::unique_ptr<OutputStream> outputStream_ {
        nullptr
    };
    BluetoothRemoteDevice remoteDevice_;
    UUID uuid_;
    BtSocketType type_;
    std::string address_;
    int fd_;
    bool auth_;
    int socketStatus_;
    int socketChannel_ = -1;
    uint32_t maxTxPacketSize_ = 0;
    uint32_t maxRxPacketSize_ = 0;
};

class ClientSocket::impl::BluetoothSocketObserverImp : public BluetoothClientSocketObserverStub {
public:
    explicit BluetoothSocketObserverImp(ClientSocket::impl &clientSocket) : clientSocket_(clientSocket)
    {
        HILOGD("enter");
    }
    ~BluetoothSocketObserverImp()
    {
        HILOGD("enter");
    }

    void OnConnectionStateChanged(const BluetoothRawAddress &dev, bluetooth::Uuid uuid, int status, int result) override
    {
        HILOGI("dev: %s, uuid: %s, status: %d, result: %d", GetEncryptAddr((dev).GetAddress()).c_str(),
            uuid.ToString().c_str(), status, result);
        BluetoothRemoteDevice device(dev.GetAddress(), BTTransport::ADAPTER_BREDR);
        UUID btUuid = UUID::ConvertFrom128Bits(uuid.ConvertTo128Bits());
        if (!clientSocket_.observer_) {
            HILOGE("clientSocket observer is nullptr.");
            return;
        }
        clientSocket_.observer_->OnConnectionStateChanged(device, btUuid, status, result);
    }

private:
    ClientSocket::impl &clientSocket_;
};

ClientSocket::impl::impl(const BluetoothRemoteDevice &addr, UUID uuid, BtSocketType type, bool auth)
    : inputStream_(nullptr),
      outputStream_(nullptr),
      remoteDevice_(addr),
      uuid_(uuid),
      type_(type),
      fd_(-1),
      auth_(auth),
      socketStatus_(SOCKET_INIT)
{
    HILOGD("enter 4 parameters");
    observerImp_ = new (std::nothrow) BluetoothSocketObserverImp(*this);
}

ClientSocket::impl::impl(int fd, std::string address, BtSocketType type)
    : inputStream_(std::make_unique<InputStream>(fd)),
      outputStream_(std::make_unique<OutputStream>(fd)),
      remoteDevice_(BluetoothRemoteDevice(address, 0)),
      type_(type),
      address_(address),
      fd_(fd),
      auth_(false),
      socketStatus_(SOCKET_CONNECTED)
{
    HILOGD("enter 3 parameters");
    observerImp_ = new (std::nothrow) BluetoothSocketObserverImp(*this);
}

ClientSocket::impl::impl(const BluetoothRemoteDevice &addr, UUID uuid, BtSocketType type, bool auth,
    std::shared_ptr<BluetoothConnectionObserver> observer)
    : observer_(observer),
      inputStream_(nullptr),
      outputStream_(nullptr),
      remoteDevice_(addr),
      uuid_(uuid),
      type_(type),
      fd_(-1),
      auth_(auth),
      socketStatus_(SOCKET_INIT)
{
    HILOGD("enter 5 parameters");
    observerImp_ = new (std::nothrow) BluetoothSocketObserverImp(*this);
}

ClientSocket::ClientSocket(const BluetoothRemoteDevice &bda, UUID uuid, BtSocketType type, bool auth)
    : pimpl(new ClientSocket::impl(bda, uuid, type, auth))
{}

ClientSocket::ClientSocket(int fd, std::string address, BtSocketType type)
    : pimpl(new ClientSocket::impl(fd, address, type))
{}

ClientSocket::ClientSocket(const BluetoothRemoteDevice &bda, UUID uuid, BtSocketType type, bool auth,
    std::shared_ptr<BluetoothConnectionObserver> observer)
    : pimpl(new ClientSocket::impl(bda, uuid, type, auth, observer))
{}

ClientSocket::~ClientSocket()
{}

int ClientSocket::Connect(int psm)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "BR is not TURN_ON");

    pimpl->address_ = pimpl->remoteDevice_.GetDeviceAddr();
    std::string tempAddress = pimpl->address_;
    CHECK_AND_RETURN_LOG_RET(tempAddress.size(), BtStatus::BT_FAILURE, "address size error");
    CHECK_AND_RETURN_LOG_RET(pimpl->socketStatus_ != SOCKET_CLOSED, BT_ERR_INVALID_STATE, "socket closed");
    sptr<IBluetoothSocket> proxy = GetRemoteProxy<IBluetoothSocket>(PROFILE_SOCKET);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_SERVICE_DISCONNECTED, "proxy is nullptr");

    bluetooth::Uuid tempUuid = bluetooth::Uuid::ConvertFrom128Bits(pimpl->uuid_.ConvertTo128Bits());
    int ret = proxy->RegisterClientObserver(BluetoothRawAddress(pimpl->address_), tempUuid,
        pimpl->observerImp_);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "regitser observer fail, ret = %d", ret);

    ConnectSocketParam param {
        .addr = tempAddress,
        .uuid = tempUuid,
        .securityFlag = (int32_t)pimpl->getSecurityFlags(),
        .type = (int32_t)pimpl->type_,
        .psm = psm
    };
    ret = proxy->Connect(param, pimpl->fd_);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "Connect error %{public}d", ret);

    HILOGI("fd_: %{public}d", pimpl->fd_);
    CHECK_AND_RETURN_LOG_RET(pimpl->fd_ != -1, BtStatus::BT_FAILURE, "connect failed!");
    CHECK_AND_RETURN_LOG_RET(pimpl->RecvSocketPsmOrScn(), BT_ERR_INVALID_STATE, "recv psm or scn failed");

    bool recvret = pimpl->RecvSocketSignal();
    HILOGI("recvret: %{public}d", recvret);
    pimpl->inputStream_ = std::make_unique<InputStream>(pimpl->fd_);
    pimpl->outputStream_ = std::make_unique<OutputStream>(pimpl->fd_);
    CHECK_AND_RETURN_LOG_RET(recvret, BtStatus::BT_FAILURE, "recvSocketSignal connect failed!");
    pimpl->socketStatus_ = SOCKET_CONNECTED;
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "connect", "ID", pimpl->fd_, "ADDRESS",
        GetEncryptAddr(tempAddress), "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());
    return BtStatus::BT_SUCCESS;
}

void ClientSocket::Close()
{
    HILOGD("enter");
    return pimpl->Close();
}

InputStream &ClientSocket::GetInputStream()
{
    HILOGD("enter");
    return pimpl->GetInputStream();
}

OutputStream &ClientSocket::GetOutputStream()
{
    HILOGD("enter");
    return pimpl->GetOutputStream();
}

BluetoothRemoteDevice &ClientSocket::GetRemoteDevice()
{
    HILOGD("enter");
    return pimpl->GetRemoteDevice();
}

bool ClientSocket::IsConnected() const
{
    HILOGD("enter");
    return pimpl->IsConnected();
}

int ClientSocket::SetBufferSize(int bufferSize)
{
    HILOGD("enter");
    return pimpl->SetBufferSize(bufferSize);
}

int ClientSocket::GetSocketFd()
{
    HILOGD("enter");
    return pimpl->fd_;
}

int ClientSocket::GetL2capPsm()
{
    HILOGI("psm:%{public}d", pimpl->socketChannel_);
    return pimpl->socketChannel_;
}

int ClientSocket::GetRfcommScn()
{
    HILOGI("scn:%{public}d", pimpl->socketChannel_);
    return pimpl->socketChannel_;
}

uint32_t ClientSocket::GetMaxTransmitPacketSize()
{
    HILOGI("MaxTransmitPacketSize:%{public}d", pimpl->maxTxPacketSize_);
    return pimpl->maxTxPacketSize_;
}

uint32_t ClientSocket::GetMaxReceivePacketSize()
{
    HILOGI("MaxReceivePacketSize:%{public}d", pimpl->maxRxPacketSize_);
    return pimpl->maxRxPacketSize_;
}

struct ServerSocket::impl {
    impl(const std::string &name, UUID uuid, BtSocketType type, bool encrypt);
    ~impl()
    {
        HILOGI("enter");
        if (fd_ > 0) {
            shutdown(fd_, SHUT_RD);
            shutdown(fd_, SHUT_WR);
            close(fd_);
            HILOGI("fd closed, fd_: %{public}d", fd_);
            fd_ = -1;
        }
    }

    int Listen()
    {
        HILOGD("enter");

        CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "BR is not TURN_ON");

        sptr<IBluetoothSocket> proxy = GetRemoteProxy<IBluetoothSocket>(PROFILE_SOCKET);
        if (!proxy) {
            HILOGE("failed, proxy is nullptr");
            socketStatus_ = SOCKET_CLOSED;
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        CHECK_AND_RETURN_LOG_RET(socketStatus_ != SOCKET_CLOSED, BT_ERR_INVALID_STATE,
            "failed, socketStatus_ is SOCKET_CLOSED");

        ListenSocketParam param {
            .name = name_,
            .uuid = bluetooth::Uuid::ConvertFrom128Bits(uuid_.ConvertTo128Bits()),
            .securityFlag = (int32_t)getSecurityFlags(),
            .type = (int32_t)type_,
            .observer = observer_
        };
        int ret = proxy->Listen(param, fd_);
        CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "Listen error %{public}d.", ret);

        if (fd_ == BT_INVALID_SOCKET_FD) {
            HILOGE("listen socket failed");
            socketStatus_ = SOCKET_CLOSED;
            return BT_ERR_INVALID_STATE;
        }

        CHECK_AND_RETURN_LOG_RET(RecvSocketPsmOrScn(), BT_ERR_INVALID_STATE, "recv psm or scn failed");

        if (socketStatus_ == SOCKET_INIT) {
            socketStatus_ = SOCKET_LISTENING;
        } else {
            HILOGE("failed, socketStatus_: %{public}d is not SOCKET_INIT", socketStatus_);
            close(fd_);
            socketStatus_ = SOCKET_CLOSED;
            return BT_ERR_INVALID_STATE;
        }

        return BT_NO_ERROR;
    }

    int getSecurityFlags()
    {
        int flags = 0;
        if (encrypt_) {
            flags |= FLAG_AUTH;
            flags |= FLAG_ENCRYPT;
        }
        return flags;
    }

    std::shared_ptr<ClientSocket> Accept(int timeout)
    {
        HILOGD("enter");
        if (socketStatus_ != SOCKET_LISTENING) {
            HILOGE("socket is not in listen state");
            return nullptr;
        }
        struct timeval time = {timeout, 0};
        setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&time, sizeof(time));
        setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));

        acceptFd_ = RecvSocketFd();
        HILOGI("RecvSocketFd acceptFd: %{public}d", acceptFd_);
        if (acceptFd_ <= 0) {
            return nullptr;
        }
        if (timeout > 0) {
            time = {0, 0};
            setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&time, sizeof(time));
            setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));
        }

        std::shared_ptr<ClientSocket> clientSocket = std::make_shared<ClientSocket>(acceptFd_, acceptAddress_, type_);

        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "connect", "ID", acceptFd_, "ADDRESS",
            GetEncryptAddr(acceptAddress_), "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());

        return clientSocket;
    }

    int RecvSocketFd()
    {
        HILOGD("enter");
        char ccmsg[CMSG_SPACE(sizeof(int))];
        char buffer[SOCKET_RECV_FD_SIZE];
        struct iovec io = {.iov_base = buffer, .iov_len = sizeof(buffer)};
        struct msghdr msg;
        (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
        msg.msg_control = ccmsg;
        msg.msg_controllen = sizeof(ccmsg);
        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

#ifdef DARWIN_PLATFORM
        int rv = recvmsg(fd_, &msg, 0);
#else
        int rv = recvmsg(fd_, &msg, MSG_NOSIGNAL);
#endif
        if (rv == -1) {
            HILOGE("[sock] recvmsg error  %{public}d, fd: %{public}d", errno, fd_);
            return BtStatus::BT_FAILURE;
        }
        struct cmsghdr *cmptr = CMSG_FIRSTHDR(&msg);
        CHECK_AND_RETURN_LOG_RET(cmptr != nullptr, BtStatus::BT_FAILURE, "cmptr error");
        CHECK_AND_RETURN_LOG_RET(cmptr->cmsg_len == CMSG_LEN(sizeof(int)) && cmptr->cmsg_level == SOL_SOCKET
            && cmptr->cmsg_type == SCM_RIGHTS, BtStatus::BT_FAILURE,
            "recvmsg error, len:%{public}d level:%{public}d type:%{public}d",
            cmptr->cmsg_len, cmptr->cmsg_level, cmptr->cmsg_type);
        int clientFd = *(reinterpret_cast<int *>(CMSG_DATA(cmptr)));

        uint8_t recvBuf[rv];
        (void)memset_s(&recvBuf, sizeof(recvBuf), 0, sizeof(recvBuf));
        CHECK_AND_RETURN_LOG_RET(memcpy_s(recvBuf, sizeof(recvBuf), (uint8_t *)msg.msg_iov[0].iov_base, rv) == EOK,
            BtStatus::BT_FAILURE, "RecvSocketFd, recvBuf memcpy_s fail");

        uint8_t buf[SOCKET_RECV_ADDR_SIZE] = {0};
        CHECK_AND_RETURN_LOG_RET(memcpy_s(buf, sizeof(buf), &recvBuf[ADDR_OFFSET], sizeof(buf)) == EOK,
            BtStatus::BT_FAILURE, "RecvSocketFd, buf memcpy_s fail");

        char token[LENGTH] = {0};
        (void)sprintf_s(token, sizeof(token), "%02X:%02X:%02X:%02X:%02X:%02X",
            buf[0x05], buf[0x04], buf[0x03], buf[0x02], buf[0x01], buf[0x00]);
        BluetoothRawAddress rawAddr {token};
        acceptAddress_ = rawAddr.GetAddress().c_str();

        maxTxPacketSize_ = GetPacketSizeFromBuf(recvBuf + TX_OFFSET, rv - TX_OFFSET);
        maxRxPacketSize_ = GetPacketSizeFromBuf(recvBuf + RX_OFFSET, rv - RX_OFFSET);
        return clientFd;
    }

    uint16_t GetPacketSizeFromBuf(uint8_t recvBuf[], int recvBufLen)
    {
        uint16_t shortBuf;
        CHECK_AND_RETURN_LOG_RET(recvBuf, 0, "getpacketsize fail, invalid recvBuf");
        CHECK_AND_RETURN_LOG_RET(recvBufLen >= SOCKET_RECV_TXRX_SIZE, 0, "getpacketsize fail, invalid recvBufLen");
        CHECK_AND_RETURN_LOG_RET(memcpy_s(&shortBuf, sizeof(shortBuf), &recvBuf[0], sizeof(shortBuf)) == EOK, 0,
            "getpacketsize failed, memcpy_s fail");
        return shortBuf;
    }

    bool RecvSocketPsmOrScn()
    {
        int channel = 0;
#ifdef DARWIN_PLATFORM
        int recvBufSize = recv(fd_, &channel, sizeof(channel), 0);
#else
        int recvBufSize = recv(fd_, &channel, sizeof(channel), MSG_WAITALL);
#endif
        CHECK_AND_RETURN_LOG_RET(recvBufSize == SOCKET_RECV_CHANNEL_SIZE, false,
            "recv psm or scn error, errno:%{public}d, fd_:%{public}d", errno, fd_);
        CHECK_AND_RETURN_LOG_RET(channel > 0, false,
            "recv channel error, errno:%{public}d, fd_:%{public}d", errno, fd_);
        HILOGI("psm or scn = %{public}d, type = %{public}d", channel, type_);
        socketChannel_ = channel;
        return true;
    }

    void Close()
    {
        HILOGD("enter");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGW("The socketStatus_ is already SOCKET_CLOSED");
            return;
        } else {
            socketStatus_ = SOCKET_CLOSED;
            if (fd_ > 0) {
                shutdown(fd_, SHUT_RD);
                shutdown(fd_, SHUT_WR);
                close(fd_);
                HILOGI("fd closed, fd_: %{public}d", fd_);
                fd_ = -1;
                sptr<IBluetoothSocket> proxy = GetRemoteProxy<IBluetoothSocket>(PROFILE_SOCKET);
                CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
                proxy->DeregisterServerObserver(observer_);
                return;
            } else {
                HILOGE("socket not created");
                return;
            }
        }
    }

    const std::string &GetStringTag()
    {
        HILOGD("enter");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGE("socketStatus_ is SOCKET_CLOSED");
            socketServiceType_ = "";
        } else {
            socketServiceType_ = "ServerSocket:";
            socketServiceType_.append(" Type: ").append(ConvertTypeToString(type_))
                .append(" ServerName: ").append(name_);
        }
        return socketServiceType_;
    }

    static std::string ConvertTypeToString(BtSocketType type)
    {
        std::string retStr;
        if (type == TYPE_RFCOMM) {
            retStr = "TYPE_RFCOMM";
        } else if (type == TYPE_L2CAP) {
            retStr = "TYPE_L2CAP";
        } else if (type == TYPE_L2CAP_LE) {
            retStr = "TYPE_L2CAP_LE";
        } else {
            retStr = "TYPE_UNKNOW";
        }
        return retStr;
    }

    sptr<BluetoothServerSocketObserverStub> observer_ = nullptr;

    sptr<IBluetoothSocket> proxy;
    UUID uuid_;
    BtSocketType type_;
    bool encrypt_;
    int fd_;
    int socketStatus_;
    std::string name_ {
        ""
        };
    int acceptFd_ = 0;
    std::string acceptAddress_;
    std::string socketServiceType_ {
        ""
    };
    int socketChannel_ = -1;
    uint32_t maxTxPacketSize_ = 0;
    uint32_t maxRxPacketSize_ = 0;
};

ServerSocket::impl::impl(const std::string &name, UUID uuid, BtSocketType type, bool encrypt)
    : uuid_(uuid), type_(type), encrypt_(encrypt), fd_(-1), socketStatus_(SOCKET_INIT), name_(name)
{
    HILOGI("(4 parameters) starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgr, "samgr is nullptr.");

    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    CHECK_AND_RETURN_LOG(hostRemote, "failed: no hostRemote");

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SOCKET);
    CHECK_AND_RETURN_LOG(remote, "failed: no remote");

    HILOGI("remote obtained");

   sptr<IBluetoothSocket> proxy = GetRemoteProxy<IBluetoothSocket>(PROFILE_SOCKET);
    CHECK_AND_RETURN_LOG(proxy, "failed: no proxy");
    observer_ = new BluetoothServerSocketObserverStub();
}

ServerSocket::ServerSocket(const std::string &name, UUID uuid, BtSocketType type, bool encrypt)
    : pimpl(new ServerSocket::impl(name, uuid, type, encrypt))
{
    HILOGI("type:%{public}d encrypt:%{public}d", type, encrypt);
}

ServerSocket::~ServerSocket()
{}

int ServerSocket::Listen()
{
    HILOGD("enter");
    return pimpl->Listen();
}

std::shared_ptr<ClientSocket> ServerSocket::Accept(int timeout)
{
    HILOGD("enter");
    return pimpl->Accept(timeout);
}

void ServerSocket::Close()
{
    HILOGD("enter");
    return pimpl->Close();
}

const std::string &ServerSocket::GetStringTag()
{
    HILOGD("enter");
    return pimpl->GetStringTag();
}

int ServerSocket::GetL2capPsm()
{
    HILOGI("psm:%{public}d", pimpl->socketChannel_);
    return pimpl->socketChannel_;
}

int ServerSocket::GetRfcommScn()
{
    HILOGI("scn:%{public}d", pimpl->socketChannel_);
    return pimpl->socketChannel_;
}

uint32_t ServerSocket::GetMaxTransmitPacketSize()
{
    HILOGI("MaxTransmitPacketSize:%{public}d", pimpl->maxTxPacketSize_);
    return pimpl->maxTxPacketSize_;
}

uint32_t ServerSocket::GetMaxReceivePacketSize()
{
    HILOGI("MaxReceivePacketSize:%{public}d", pimpl->maxRxPacketSize_);
    return pimpl->maxRxPacketSize_;
}

int ServerSocket::GetSocketFd()
{
    return pimpl->fd_;
}

std::shared_ptr<ClientSocket> SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord(
    const BluetoothRemoteDevice &device, const UUID &uuid)
{
    HILOGD("enter");
    if (device.IsValidBluetoothRemoteDevice()) {
        return std::make_shared<ClientSocket>(device, uuid, TYPE_RFCOMM, false);
    } else {
        HILOGE("[sock] Device is not valid.");
        return nullptr;
    }
}

std::shared_ptr<ClientSocket> SocketFactory::BuildRfcommDataSocketByServiceRecord(
    const BluetoothRemoteDevice &device, const UUID &uuid)
{
    HILOGD("enter");
    if (device.IsValidBluetoothRemoteDevice()) {
        return std::make_shared<ClientSocket>(device, uuid, TYPE_RFCOMM, true);
    } else {
        HILOGE("[sock] Device is not valid.");
        return nullptr;
    }
}

std::shared_ptr<ServerSocket> SocketFactory::DataListenInsecureRfcommByServiceRecord(
    const std::string &name, const UUID &uuid)
{
    HILOGD("enter");
    return std::make_shared<ServerSocket>(name, uuid, TYPE_RFCOMM, false);
}

std::shared_ptr<ServerSocket> SocketFactory::DataListenRfcommByServiceRecord(const std::string &name, const UUID &uuid)
{
    HILOGD("enter");
    return std::make_shared<ServerSocket>(name, uuid, TYPE_RFCOMM, true);
}

int ClientSocket::UpdateCocConnectionParams(CocUpdateSocketParam &param)
{
    HILOGI("UpdateCocConnectionParams enter");
    BluetoothSocketCocInfo info;

    info.addr = param.addr;
    info.minInterval = param.minInterval;
    info.maxInterval = param.maxInterval;
    info.peripheralLatency = param.peripheralLatency;
    info.supervisionTimeout = param.supervisionTimeout;
    info.minConnEventLen = param.minConnEventLen;
    info.maxConnEventLen = param.maxConnEventLen;
    sptr<IBluetoothSocket> proxy = GetRemoteProxy<IBluetoothSocket>(PROFILE_SOCKET);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "proxy is null");
    return proxy->UpdateCocConnectionParams(info);
}
}  // namespace Bluetooth
}  // namespace OHOS