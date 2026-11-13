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

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD000102
#endif
#ifndef LOG_TAG
#define LOG_TAG "bt_appgallery_interface"
#endif

#include "appgallery_interface.h"
#include "vendor_loader.h"
#include "vendor_log.h"

namespace {
constexpr int32_t APPGALLERY_ERROR = -1;

/*
 * 懒加载 vendor 库并解析单个导出符号（extern "C"，如 OH_BtGetCategoryFromSystem）。
 * 库不可用或符号缺失时返回 nullptr，由调用方回退默认值。
 */
void *GetVendorSymbol(const char *symbolName)
{
    void *handle = nullptr;
    if (vendor_interface_load_vendor_library(&handle) != 0) {
        HILOGE("vendor library is unavailable");
        return nullptr;
    }
    return vendor_interface_get_vendor_symbol(symbolName);
}
}  // namespace

namespace OHOS {
namespace bluetooth {
int32_t AppgalleryInterface::GetCategoryFromSystem(const std::vector<std::string> &bundleNames,
    std::vector<AppgalleryCategoryInfo> &resultArray, const std::string &tag)
{
    static const auto vendorFunc = reinterpret_cast<decltype(&OH_BtGetCategoryFromSystem)>(
        GetVendorSymbol("OH_BtGetCategoryFromSystem"));
    if (vendorFunc == nullptr) {
        HILOGE("OH_BtGetCategoryFromSystem GetVendorSymbol failed");
        return APPGALLERY_ERROR;
    }
    return vendorFunc(bundleNames, resultArray, tag);
}
}  // namespace bluetooth
}  // namespace OHOS
