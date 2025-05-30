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
#define LOG_TAG "bt_fwk_socket_outputstream"
#endif

#include <unistd.h>
#include <cerrno>
#include "bluetooth_socket_outputstream.h"
#include "bluetooth_log.h"
#include "sys/socket.h"
#include "sys/ioctl.h"

namespace OHOS {
namespace Bluetooth {

static constexpr int32_t SOCKET_PACKET_HEAD_LENGTH = 1512;
static constexpr int32_t AAM_UID = 7878;
static constexpr int32_t AAM_BAD_RET = -978974;
static constexpr int32_t SOFTBUS_UID = 1024;
static constexpr int32_t SOCKET_WRITE_TIMEOUT_50_SEC = 50;
OutputStream::OutputStream(int socketFd) : socketFd_(socketFd)
{}

OutputStream::~OutputStream()
{}

int OutputStream::Write(const uint8_t *buf, size_t length)
{
    if (socketFd_ == -1) {
        HILOGE("socket closed.");
        return -1;
    }

    int32_t bufSize;
    socklen_t optlen = sizeof(bufSize);
    int sockOptRet = getsockopt(socketFd_, SOL_SOCKET, SO_SNDBUF, &bufSize, &optlen);
    unsigned long bytesInBuffer;
    int ioctlRet = ioctl(socketFd_, TIOCOUTQ, &bytesInBuffer);
    if (sockOptRet != -1 && ioctlRet != -1 && static_cast<unsigned long>(bufSize) > bytesInBuffer) { // -1代表无权限获取发送队列大小
        // 该方法是跟踪send前socket发送通道是否占满导致发包阻塞
        unsigned long availableLength = static_cast<unsigned long>(bufSize) - bytesInBuffer;
        int32_t sendLength = static_cast <int32_t>(length) + SOCKET_PACKET_HEAD_LENGTH;
        if (availableLength < static_cast<unsigned long>(sendLength)) {
            HILOGW("send queue is full, availableLength is %{public}lu, sendlength is %{public}d",
                availableLength, sendLength);
            if (getuid() == AAM_UID) {
                HILOGE("AAM close socket");
                return AAM_BAD_RET;
            }
        }
    }
    if (getuid() == SOFTBUS_UID && setTimeoutFlag_ == false) {
        HILOGD("SOFTBUS write data set 50s tiemout.");
        timeval timeout;
        timeout.tv_sec = SOCKET_WRITE_TIMEOUT_50_SEC;
        timeout.tv_usec = 0;
        if (setsockopt(socketFd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            HILOGW("Failed to set send timeout");
            return -1;
        }
        setTimeoutFlag_ = true;
    }

    auto ret = send(socketFd_, buf, length, MSG_NOSIGNAL);

    HILOGD("ret: %{public}zd", ret);

    if (ret <= 0) {
        setTimeoutFlag_ == false;
        HILOGE("socket write exception! ret:%{public}zd errno:%{public}d", ret, errno);
    }
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS