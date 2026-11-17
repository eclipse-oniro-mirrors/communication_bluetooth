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

#ifndef AVRCP_COVER_ART_STORAGE_H
#define AVRCP_COVER_ART_STORAGE_H

#include <mutex>
#include <string>
#include "cover_art.h"

namespace OHOS {
namespace bluetooth {

class AvrcpCoverArtStorage {
public:
    static AvrcpCoverArtStorage* GetInstance();

    std::string StoreImage(std::shared_ptr<CoverArt> coverArt);
    std::shared_ptr<CoverArt> GetImage(const std::string &imageHandleStr);
    std::string GetNextImageHandle();
    void ResetImageHandle();
    void Clear();
private:
    explicit AvrcpCoverArtStorage() {};
    ~AvrcpCoverArtStorage(){};

    std::mutex imageHandlesLock_; // lock nextImageHandle_
    int32_t nextImageHandle_ = 0;
    struct imageInfo {
        std::string hashs;
        std::string imageHandle;
        std::shared_ptr<CoverArt> coverArt;
        int64_t timestamp;
    };
    std::mutex imageInfoMutex_;
    imageInfo newInfo_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_COVER_ART_STORAGE_H