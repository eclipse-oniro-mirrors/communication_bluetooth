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

#pragma once

#include <features.h>
#include <sys/types.h>

#if __GLIBC__

#include <unistd.h>

/* Get thread identification. */ /* 获取线程标识（线程 ID） */
pid_t gettid(void) throw(); /* 返回当前线程的线程 ID */

/* Copy src to string dst of size siz. */ /* 将 src 拷贝到大小为 siz 的字符串 dst */
size_t strlcpy(char* dst, const char* src, size_t siz); /* 安全字符串拷贝，保证结果以 '\0' 结尾，返回 src 长度 */

/* Appends src to string dst of size siz. */ /* 将 src 追加到大小为 siz 的字符串 dst 末尾 */
size_t strlcat(char* dst, const char* src, size_t siz); /* 安全字符串追加，保证结果以 '\0' 结尾，返回追加后总长度 */

#endif
