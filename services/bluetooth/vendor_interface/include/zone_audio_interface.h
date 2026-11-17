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

#ifndef ZONE_AUDIO_INTERFACE_H
#define ZONE_AUDIO_INTERFACE_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {
constexpr int32_t ZONE_AUDIO_INTERFACE_SUCCESS = 0;
constexpr int32_t ZONE_AUDIO_INTERFACE_FAILED = -1;

/* 蓝牙地址字符串定长上限（"XX:XX:XX:XX:XX:XX" 共 17 字节，留余量）。 */
constexpr uint32_t ZONE_AUDIO_ADDR_MAX_LEN = 32;

/* 跨 so 传递的设备状态：CONNECT/DISCONNECT 与车音频框架语义一致，
 * 无法识别的状态统一映射为 UNKNOWN，由上层忽略。 */
constexpr int32_t ZONE_AUDIO_DEVICE_STATUS_CONNECT = 0;
constexpr int32_t ZONE_AUDIO_DEVICE_STATUS_DISCONNECT = 1;
constexpr int32_t ZONE_AUDIO_DEVICE_STATUS_UNKNOWN = 2;

/* 跨 so 传递的设备描述，字段与车音频框架 DeviceDescriptor 对应；
 * deviceAddress 为蓝牙地址字符串（空串表示无效条目）。 */
struct ZoneAudioDevice {
    char deviceAddress[ZONE_AUDIO_ADDR_MAX_LEN] = {0};
    int32_t status = ZONE_AUDIO_DEVICE_STATUS_UNKNOWN;
};

/* 设备变化回调：vendor 侧在原线程回调，devices 生命周期仅限回调内有效。 */
typedef void (*ZoneAudioDeviceChangeFunc)(int32_t displayId, const ZoneAudioDevice *devices,
    uint32_t deviceCount, void *userData);

/*
 * 桥接接口：仅透传分区音频设备变化回调的注册/去注册，
 * vendor_interface 模块经 dlopen/dlsym 获取符号后直接调用，
 * 签名须与 service 仓 bt_vendor/include/zone_audio_interface.h 保持同步。
 */
#ifdef __cplusplus
extern "C" {
#endif
int32_t OH_BtZoneAudioRegisterDeviceChangeCallback(int32_t displayId,
    ZoneAudioDeviceChangeFunc callback, void *userData);
int32_t OH_BtZoneAudioUnregisterDeviceChangeCallback(int32_t displayId);
#ifdef __cplusplus
}
#endif

/*
 * 分区音频接口：上层（MultiA2dpManager）持有并调用本类，
 * 内部完成 libbt_vendor_service.z.so 的加载及符号解析。
 * 库不可用（未加载/符号缺失）时返回 ZONE_AUDIO_INTERFACE_FAILED。
 */
class ZoneAudioInterface {
public:
    static int32_t RegisterDeviceChangeCallback(int32_t displayId,
        ZoneAudioDeviceChangeFunc callback, void *userData);
    static int32_t UnregisterDeviceChangeCallback(int32_t displayId);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // ZONE_AUDIO_INTERFACE_H
