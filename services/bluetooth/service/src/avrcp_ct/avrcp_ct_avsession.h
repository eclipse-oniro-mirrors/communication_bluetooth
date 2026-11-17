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

#ifndef AVRCP_CT_AVSESSION_H
#define AVRCP_CT_AVSESSION_H

#include "log.h"
#include "avsession_manager.h"
#include "avrcp_ct_metadata.h"

namespace OHOS {
namespace bluetooth {
class AvrcpCtAVSession {
public:
    AvrcpCtAVSession();
    ~AvrcpCtAVSession() = default;
    void CreateAVSession(const std::string &tag, int32_t type, const AppExecFwk::ElementName &elementName);
    void Destroy();

    void RegisterCallback(std::shared_ptr<AVSession::AVSessionCallback> callback);

    void AddSupportCommands();
    void DeleteSupportCommands();

    void Activate();
    void Deactivate();
    void UpdateMetaData(const AVSession::AVMetaData &meta);
    void UpdatePlaybackState(const AVSession::AVPlaybackState &state);
    AVSession::AVPlaybackState &GetPlaybackState();

    void UpdatePlayingQueueItems(const std::vector<AVSession::AVQueueItem> &items);
    void ClearPlayingQueueItems();

private:
    std::shared_ptr<AVSession::AVSession> GetAvSession();
    bool isActive_{false};
    std::shared_ptr<AVSession::AVSession> avSession_{nullptr};
    AVSession::AVPlaybackState playbackState_{};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif