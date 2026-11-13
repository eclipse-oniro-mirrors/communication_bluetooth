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

#ifndef BLUETOOTH_VENDOR_INTERFACE_H
#define BLUETOOTH_VENDOR_INTERFACE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include "base_def.h"
#include "base_observer_list.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace bluetooth {

#define BT_TRANSPORT_LE 0
#define HW_CONN_ATTR_TYPE_SET_LP_DEVICE_BUFFER_SIZE_AND_COUNT 9

namespace STACK {
struct RawAddress {
    uint8_t addr[6] = {0};
    static const RawAddress kEmpty;
};
}

struct HwConnAttr {
    uint8_t type;
};

typedef struct {
    void (*sendParamsToSensorhub)(const char *params, char type, int length);
    /* set connection attribution. supported attribution see the defination of HwConnAttrType */
    void (*hwConnAttrSet)(uint8_t transport, const STACK::RawAddress &addr, const HwConnAttr *attr);
} bthwif_interface_t;

class BluetoothHwInterface final {
public:
    class SensorhubObserver {
    public:
        virtual ~SensorhubObserver() = default;
        virtual void SensorhubDevInfoCallback(uint8_t *buffer, int length) {};
        virtual void SensorhubResetCallback(uint32_t state) {};
#ifdef CONTEXTHUB_BLE_V3
        virtual void SensorhubCollaborationCallback(const std::vector<uint8_t> &notifyValue) {};
#endif
    };

    static BluetoothHwInterface *GetInstance(void);
    bool InitBtHwInterface(const bthwif_interface_t *bthwif);
    const bthwif_interface_t* GetBtHwInterface(void);
    void SetBtHwInterface(const bthwif_interface_t* interface);
    void AddSensorhubObserver(SensorhubObserver &observer);
    void RemoveSensorhubObserver(SensorhubObserver &observer);

private:
    BluetoothHwInterface();
    ~BluetoothHwInterface() = default;

    const bthwif_interface_t *bthwInterface_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(BluetoothHwInterface);
};

void HwSensorhubCollaborationCb(const std::vector<uint8_t> &notifyValue);

}  // namespace bluetooth
}  // namespace OHOS

void TestSetScreenStatus(const std::string &status);

#endif  // BLUETOOTH_VENDOR_INTERFACE_H
