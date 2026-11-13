/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_IPC_IPC_SKELETON_H
#define OHOS_IPC_IPC_SKELETON_H

#include <cstdint>

namespace OHOS {
class IPCSkeleton {
public:
    static int GetCallingFullTokenID()
    {
        return 0;
    }

    static int GetCallingTokenID()
    {
        return 0;
    }

    static int GetCallingUid()
    {
        return 7035;
    }

    static int32_t GetCallingPid()
    {
        return callingPid_;
    }

    static void SetCallingPid(int32_t pid)
    {
        callingPid_ = pid;
    }

private:
    static inline int32_t callingPid_ = 0;
};

} // namespace OHOS
#endif // OHOS_IPC_IPC_SKELETON_H