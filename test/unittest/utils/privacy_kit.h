/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PRIVACY_KIT_H
#define PRIVACY_KIT_H

namespace OHOS {
class PrivacyKit {
public:
    static int32_t AddPermissionUsedRecord(
        int32_t tokenId, std::string permissionName, int32_t successCount, int32_t failCount, bool asyncMode = false)
    {
        return 0;
    }
};

} // namespace OHOS
#endif // PRIVACY_KIT_H