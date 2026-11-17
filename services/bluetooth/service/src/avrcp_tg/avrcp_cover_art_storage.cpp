/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_cover_art_storage"
#endif

#include "avrcp_cover_art_storage.h"
#include "common_util.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {
constexpr int32_t MAX_IMAGE_HANDLE = 9999999;
constexpr int32_t IMAGE_HANDLE_LENGTH = 7;

AvrcpCoverArtStorage* AvrcpCoverArtStorage::GetInstance()
{
    static AvrcpCoverArtStorage instance;
    return &instance;
}

// assign a valid handle
std::string AvrcpCoverArtStorage::GetNextImageHandle()
{
    std::lock_guard<std::mutex> lock(imageHandlesLock_);
    CHECK_AND_RETURN_LOG_RET(nextImageHandle_ <= MAX_IMAGE_HANDLE, "", "No more image handles left");
    std::string handle = std::to_string(nextImageHandle_);
    while (handle.length() != IMAGE_HANDLE_LENGTH) {
        handle = "0" + handle;
    }
    HILOGI("handle %{public}s", handle.c_str());
    nextImageHandle_++;
    return handle;
}

std::string AvrcpCoverArtStorage::StoreImage(std::shared_ptr<CoverArt> coverArt)
{
    CHECK_AND_RETURN_LOG_RET(coverArt != nullptr, "", "null coverArt");
    std::shared_ptr<Media::PixelMap> image = coverArt->GetImage();
    CHECK_AND_RETURN_LOG_RET(image, "", "image is nullptr");
    std::string hashs = coverArt->GetImageHash(image);
    CHECK_AND_RETURN_LOG_RET(!hashs.empty(), "", "null hash");
    std::lock_guard<std::mutex> lock(imageInfoMutex_);
    if (newInfo_.hashs == hashs) {
        return newInfo_.imageHandle;
    }
    std::string imageHandle = GetNextImageHandle(); // assign a handle.
    int64_t currentTime = GetTimeStamp();
    newInfo_ = {
        .hashs = hashs,
        .imageHandle = imageHandle,
        .coverArt = coverArt,
        .timestamp = currentTime,
    };
    HILOGI("coverArt handle: %{public}s.", imageHandle.c_str());
    return imageHandle;
}

std::shared_ptr<CoverArt> AvrcpCoverArtStorage::GetImage(const std::string &imageHandleStr)
{
    HILOGI("enter");
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
    CHECK_AND_RETURN_LOG_RET(!imageHandleStr.empty(), coverArt, "image handle is empty");
    std::lock_guard<std::mutex> lock(imageInfoMutex_);
    if (newInfo_.imageHandle == imageHandleStr) {
        return newInfo_.coverArt;
    }
    return coverArt;
}

void AvrcpCoverArtStorage::ResetImageHandle()
{
    std::lock_guard<std::mutex> lock(imageHandlesLock_);
    nextImageHandle_ = 0;
}
void AvrcpCoverArtStorage::Clear()
{
    std::lock_guard<std::mutex> lock(imageInfoMutex_);
    newInfo_ = {};
    ResetImageHandle();
}
} // namespace bluetooth
} // namespace OHOS