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

#ifndef VENDOR_LOADER_H
#define VENDOR_LOADER_H

/*
 * Load libbt_vendor.z.so into the current process and return the shared
 * object handle. The library is loaded only once; subsequent calls reuse
 * the cached handle. Returns 0 on success, -1 on failure.
 */
int vendor_interface_load_vendor_library(void **handle);

/*
 * Resolve an exported symbol from the loaded vendor library. Must be called
 * after a successful vendor_interface_load_vendor_library.
 */
void *vendor_interface_get_vendor_symbol(const char *symbolName);

#endif  // VENDOR_LOADER_H
