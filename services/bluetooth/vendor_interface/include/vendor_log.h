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

/*
 * 模块内部公用日志宏（LOG_CORE 域版本）。
 * 使用方须先定义 LOG_DOMAIN / LOG_TAG，再包含本头文件。
 * 定义与 vendor_loader.cpp / vendor_interface.cpp 各自的 LOG_TAG 配合使用。
 */

#ifndef VENDOR_LOG_H
#define VENDOR_LOG_H

#include "hilog/log.h"

#undef HILOGF
#undef HILOGE
#undef HILOGW
#undef HILOGI
#undef HILOGD
#define HILOGE(fmt, ...) HILOG_ERROR(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGW(fmt, ...) HILOG_WARN(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGI(fmt, ...) HILOG_INFO(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)
#define HILOGF(fmt, ...) HILOG_FATAL(LOG_CORE, "(%{public}s): " fmt, __FUNCTION__, ##__VA_ARGS__)

#endif  // VENDOR_LOG_H
