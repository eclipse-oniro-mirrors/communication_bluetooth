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

#ifndef LOG_TAG
#define LOG_TAG "avrcp_ct_cover_art_cache"
#endif

#include <utility>
#include "avrcp_ct_cover_art_cache.h"
#include "avsession_pixel_map_adapter.h"
#include "image_source.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {

namespace {
constexpr uint16_t AVSESSION_INNER_IMAGE_INFO_SIZE = 48;

bool IsEncodedCoverArtData(const std::vector<uint8_t> &imageData)
{
    if (imageData.size() <= 2) {
        return false;
    }
    uint16_t imgBufferSize = static_cast<uint16_t>((static_cast<uint16_t>(imageData[0]) << 8) | imageData[1]);
    return imgBufferSize != AVSESSION_INNER_IMAGE_INFO_SIZE;
}
}  // namespace
 
std::shared_ptr<AVSession::AVSessionPixelMap> ConvertCoverArtToInnerPixelMap(
    const std::vector<uint8_t> &imageData)
{
    if (imageData.empty()) {
        return nullptr;
    }
    if (!IsEncodedCoverArtData(imageData)) {
        auto innerPixelMap = std::make_shared<AVSession::AVSessionPixelMap>();
        innerPixelMap->SetInnerImgBuffer(imageData);
        return innerPixelMap;
    }
    uint32_t errorCode = 0;
    Media::SourceOptions sourceOpts;
    auto imageSource = Media::ImageSource::CreateImageSource(imageData.data(),
        static_cast<uint32_t>(imageData.size()), sourceOpts, errorCode);
    if (imageSource == nullptr) {
        HILOGE("CreateImageSource failed, errorCode=%{public}u, size=%{public}zu", errorCode, imageData.size());
        return nullptr;
    }
    Media::DecodeOptions decodeOpts;
    decodeOpts.allocatorType = Media::AllocatorType::HEAP_ALLOC;
    auto pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (pixelMap == nullptr) {
        HILOGE("CreatePixelMap failed, errorCode=%{public}u", errorCode);
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMapShared = std::move(pixelMap);
    auto innerPixelMap = AVSession::AVSessionPixelMapAdapter::ConvertAndSetInnerImgBuffer(pixelMapShared);
    if (innerPixelMap == nullptr) {
        HILOGE("ConvertAndSetInnerImgBuffer failed");
        return nullptr;
    }
    HILOGI("ConvertCoverArtToInnerPixelMap success, encoded size=%{public}zu", imageData.size());
    return innerPixelMap;
}

AvrcpCtCoverArtCache::AvrcpCtCoverArtCache() {}

void AvrcpCtCoverArtCache::AddImage(const std::string &deviceAddr, const std::string &imageUuid,
                                    const std::vector<uint8_t> &imageData)
{
    if (deviceAddr.empty() || imageUuid.empty() || imageData.empty()) {
        HILOGE("Invalid parameters for AddImage");
        return;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto &deviceCache = cache_[deviceAddr];
    deviceCache[imageUuid] = imageData;
    HILOGD("Added cover art uuid: %{public}s, size: %{public}zu", imageUuid.c_str(), imageData.size());
}

bool AvrcpCtCoverArtCache::GetImage(const std::string &deviceAddr, const std::string &imageUuid,
                                     std::vector<uint8_t> &imageData) const
{
    if (deviceAddr.empty() || imageUuid.empty()) {
        HILOGE("Invalid parameters for GetImage");
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        HILOGD("No cache");
        return false;
    }

    auto &deviceCache = deviceIt->second;
    auto imageIt = deviceCache.find(imageUuid);
    if (imageIt == deviceCache.end()) {
        HILOGD("No cover art for uuid: %{public}s", imageUuid.c_str());
        return false;
    }

    imageData.assign(imageIt->second.begin(), imageIt->second.end());
    return true;
}

bool AvrcpCtCoverArtCache::HasImage(const std::string &deviceAddr, const std::string &imageUuid) const
{
    if (deviceAddr.empty() || imageUuid.empty()) {
        HILOGE("Invalid parameters for HasImage");
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        return false;
    }

    auto &deviceCache = deviceIt->second;
    return deviceCache.find(imageUuid) != deviceCache.end();
}

void AvrcpCtCoverArtCache::RemoveImage(const std::string &deviceAddr, const std::string &imageUuid)
{
    if (deviceAddr.empty() || imageUuid.empty()) {
        HILOGE("Invalid parameters for RemoveImage");
        return;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        return;
    }

    auto &deviceCache = deviceIt->second;
    auto it = deviceCache.find(imageUuid);
    if (it != deviceCache.end()) {
        deviceCache.erase(it);
        HILOGD("Removed cover art uuid: %{public}s", imageUuid.c_str());
    }

    if (deviceCache.empty()) {
        cache_.erase(deviceIt);
    }
}

void AvrcpCtCoverArtCache::ClearDevice(const std::string &deviceAddr)
{
    if (deviceAddr.empty()) {
        HILOGE("Invalid parameters for ClearDevice");
        return;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (cache_.find(deviceAddr) != cache_.end()) {
        cache_.erase(deviceAddr);
        HILOGD("Cleared cover art cache");
    }
}

void AvrcpCtCoverArtCache::ClearAll()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cache_.clear();
    HILOGD("Cleared all cover art cache");
}

size_t AvrcpCtCoverArtCache::GetDeviceCacheSize(const std::string &deviceAddr) const
{
    if (deviceAddr.empty()) {
        HILOGE("Invalid parameters for GetDeviceCacheSize");
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        return 0;
    }
    return deviceIt->second.size();
}

size_t AvrcpCtCoverArtCache::GetTotalCacheSize() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    size_t total = 0;
    for (const auto &devicePair : cache_) {
        total += devicePair.second.size();
    }
    return total;
}

}  // namespace bluetooth
}  // namespace OHOS