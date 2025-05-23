/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "opp_transfer_information.h"
#include "string"

namespace OHOS {
namespace bluetooth {
int IOppTransferInformation::GetId() const
{
    return id_;
}

void IOppTransferInformation::SetId(int id)
{
    id_ = id;
}

std::string IOppTransferInformation::GetFileName() const
{
    return fileName_;
}

void IOppTransferInformation::SetFileName(const std::string &fileName)
{
    fileName_ = fileName;
}

std::string IOppTransferInformation::GetFilePath() const
{
    return filePath_;
}

void IOppTransferInformation::SetFilePath(const std::string &filePath)
{
    filePath_ = filePath;
}

std::string IOppTransferInformation::GetFileType() const
{
    return fileType_;
}

void IOppTransferInformation::SetFileType(const std::string &fileType)
{
    fileType_ = fileType;
}

std::string IOppTransferInformation::GetDeviceName() const
{
    return deviceName_;
}

void IOppTransferInformation::SetDeviceName(const std::string &deviceName)
{
    deviceName_ = deviceName;
}

std::string IOppTransferInformation::GetDeviceAddress() const
{
    return deviceAddress_;
}

void IOppTransferInformation::SetDeviceAddress(const std::string &deviceAddress)
{
    deviceAddress_ = deviceAddress;
}

int IOppTransferInformation::GetDirection() const
{
    return direction_;
}

void IOppTransferInformation::SetDirection(int direction)
{
    direction_ = direction;
}

int IOppTransferInformation::GetFileFd() const
{
    return fileFd_;
}

void IOppTransferInformation::SetFileFd(int fileFd)
{
    fileFd_ = fileFd;
}

int IOppTransferInformation::GetStatus() const
{
    return status_;
}

void IOppTransferInformation::SetStatus(int status)
{
    status_ = status;
}

int IOppTransferInformation::GetResult() const
{
    return result_;
}

void IOppTransferInformation::SetResult(int result)
{
    result_ = result;
}

uint64_t IOppTransferInformation::GetTimeStamp() const
{
    return timeStamp_;
}

void IOppTransferInformation::SetTimeStamp(uint64_t timeStamp)
{
    timeStamp_ = timeStamp;
}

uint64_t IOppTransferInformation::GetCurrentBytes() const
{
    return currentBytes_;
}

void IOppTransferInformation::SetCurrentBytes(uint64_t currentBytes)
{
    currentBytes_ = currentBytes;
}

uint64_t IOppTransferInformation::GetTotalBytes() const
{
    return totalBytes_;
}

void IOppTransferInformation::SetTotalBytes(uint64_t totalBytes)
{
    totalBytes_ = totalBytes;
}

int IOppTransferInformation::GetOperationNum() const
{
    return operationNum_;
}

void IOppTransferInformation::SetOperationNum(int operationNum)
{
    operationNum_ = operationNum;
}

int IOppTransferInformation::GetIsAccept() const
{
    return isAccept_;
}

void IOppTransferInformation::SetIsAccept(int isAccept)
{
    isAccept_ = isAccept;
}

int IOppTransferInformation::GetCurrentCount() const
{
    return currentCount_;
}

void IOppTransferInformation::SetCurrentCount(int currentCount)
{
    currentCount_ = currentCount;
}

int IOppTransferInformation::GetTotalCount() const
{
    return totalCount_;
}

void IOppTransferInformation::SetTotalCount(int totalCount)
{
    totalCount_ = totalCount;
}

IOppTransferFileHolder::IOppTransferFileHolder(const std::string &filePath,
    const int64_t &fileSize, const int32_t &fileFd)
{
    filePath_ = filePath;
    fileSize_ = fileSize;
    fileFd_ = fileFd;
}

std::string IOppTransferFileHolder::GetFilePath() const
{
    return filePath_;
}

void IOppTransferFileHolder::SetFilePath(const std::string &filePath)
{
    filePath_ = filePath;
}

int64_t IOppTransferFileHolder::GetFileSize() const
{
    return fileSize_;
}

void IOppTransferFileHolder::SetFileSize(const int64_t &fileSize)
{
    fileSize_ = fileSize;
}

int32_t IOppTransferFileHolder::GetFileFd() const
{
    return fileFd_;
}

void IOppTransferFileHolder::SetFileFd(const int32_t &fileFd)
{
    fileFd_ = fileFd;
}
}  // namespace bluetooth
}  // namespace OHOS