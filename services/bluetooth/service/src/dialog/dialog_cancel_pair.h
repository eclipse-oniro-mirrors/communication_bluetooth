/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef DIALOG_CANCEL_PAIR_H
#define DIALOG_CANCEL_PAIR_H

#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
class DialogCancelPair {
public:
    /**
     * @brief Request bluetooth cancel pair dialog before removing a paired device
     *        for a third-party application.
     *
     * @param device The paired device to be removed.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 26
     */
    static bool RequestCancelPairDialog(const RawAddress &device);

    /**
     * @brief Build JSON command string for the cancel-pair dialog.
     *
     * @param device The paired device to be removed.
     * @param appLabel The third-party caller's display name.
     * @return Returns JSON string to send.
     * @since 26
     */
    static std::string BuildStartCommand(const RawAddress &device, const std::string &appLabel);
};
} // namespace bluetooth
} // namespace OHOS
#endif // DIALOG_CANCEL_PAIR_H