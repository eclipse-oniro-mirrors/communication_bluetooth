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

#ifndef SPACE_MGR_INTERFACE_H
#define SPACE_MGR_INTERFACE_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {
/*
 * 桥接接口：仅透传 space_mgr_service 的企业空间查询，
 * 不做任何业务判断，逻辑留在调用方。
 * vendor_interface 模块经 dlopen/dlsym 获取符号后直接调用，
 * 签名须与 service 仓 bt_vendor/include/space_mgr_interface.h 保持同步。
 */
#ifdef __cplusplus
extern "C" {
#endif
/* Query whether the given user id is in enterprise workspace. */
int32_t OH_SpaceMgrIsEnterpriseWorkspace(int32_t userId, bool *isEnterprise);
#ifdef __cplusplus
}
#endif

/*
 * space_mgr 封装层：供 BluetoothWorkspaceManager 调用。
 * 库不可用（未加载/符号缺失）时返回失败码，由调用方回退默认值。
 */
class SpaceMgrInterface {
public:
    static int32_t IsEnterpriseWorkspace(int32_t userId, bool &isEnterprise);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // SPACE_MGR_INTERFACE_H
