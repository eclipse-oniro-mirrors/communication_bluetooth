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

#ifndef APPGALLERY_INTERFACE_H
#define APPGALLERY_INTERFACE_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace bluetooth {
/* 跨 so 传递的分类信息，仅保留使用方需要的两个字段。 */
struct AppgalleryCategoryInfo {
    int32_t primaryCategoryId = 0;
    int32_t secondaryCategoryId = 0;
};

/*
 * 桥接接口：仅透传 appgallery 的 GetCategoryFromSystem 与结果字段，
 * vendor_interface 模块经 dlopen/dlsym 获取符号后直接调用，
 * 签名须与 service 仓 bt_vendor/include/appgallery_interface.h 保持同步。
 */
#ifdef __cplusplus
extern "C" {
#endif
int32_t OH_BtGetCategoryFromSystem(const std::vector<std::string> &bundleNames,
    std::vector<AppgalleryCategoryInfo> &resultArray, const std::string &tag);
#ifdef __cplusplus
}
#endif

/*
 * appgallery 专有封装层：内部自动完成 libbt_vendor_service.z.so 的加载与符号解析，
 * 使用方直接 include 本头文件调用即可，无需自行 dlopen/dlsym。
 * 库不可用（未加载/符号缺失）时返回 -1。
 */
class AppgalleryInterface {
public:
    static int32_t GetCategoryFromSystem(const std::vector<std::string> &bundleNames,
        std::vector<AppgalleryCategoryInfo> &resultArray, const std::string &tag);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // APPGALLERY_INTERFACE_H
