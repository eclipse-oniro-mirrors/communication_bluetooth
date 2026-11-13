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

#ifndef BLUETOOTH_HW_BT_WATCH_INTERFACE_H
#define BLUETOOTH_HW_BT_WATCH_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "hardware/bluetooth.h"
#include "types/raw_address.h"
#ifndef STACK
#define STACK
#endif

/**
 * Callback for disconnect rssi
 */
typedef void (*HwWatchDisconnectRssiCallback)(const RawAddress* bdAddr, int rssi);

/**
 * Callback for sais connection state change
 */
typedef void (*HwWatchSaisConnectionStateCallback)(const RawAddress* bdAddr, bool isSaisConnected);

/**
 * Callback for mcu play state change
 */
typedef void (*HwWatchA2dpOfldPlayStateCallback)(uint8_t state);

/**
 * Callback when sais added
 */
typedef void (*HwWatchSaisServerAddedCallback)(void);

/**
 * Callback for sais enable state change
 */
typedef void (*HwWatchLeSaisEnable)(const RawAddress* bdAddr);

/**
 * Callback for br hid control action change
 */
typedef void (*HwWatchHidControlStateCallback)(uint8_t action);

/**
 * Callback when linkloss notify
 */
typedef void (*HwWatchLinkLossNotifyCallback)(void);

/** BT-WATCH callback structure. */
typedef struct {
    /** set to sizeof(BtWatchCallbacks) */
    size_t size;
    HwWatchDisconnectRssiCallback disconnectRssiCb;
    HwWatchSaisConnectionStateCallback saisConnectionStateCb;
    HwWatchA2dpOfldPlayStateCallback watchA2dpOfldPlayStateCb;
    HwWatchSaisServerAddedCallback watchSaisServerAddedCb;
    HwWatchLeSaisEnable watchLeSaisEnableCb;
    HwWatchHidControlStateCallback watchHidCtrlActionCb;
    HwWatchLinkLossNotifyCallback linkLossNotifyCb;
} BtWatchCallbacks;

/** Represents the standard BT-WATCH interface. */
typedef struct {
    /** set to sizeof(BtHwWatchInterface) */
    size_t size;
    bt_status_t (*init)(BtWatchCallbacks* callbacks);
    bt_status_t (*sendTbsf)(const RawAddress* bdAddr);
    void (*sendTbsr)(const RawAddress& bdAddr, uint16_t value);
    void (*sendScreenOn2Mcu)(void);
    void (*sendHfpState2Hisi)(const RawAddress& bdAddr, bool isStartConnect);
    bool (*isPhoneMajorClass)(const RawAddress& bdAddr);
    void (*updateDevice)(const RawAddress& bdAddr, bool isActiveDevice);
    void (*updateReconnectState)(const RawAddress& bdAddr, bool isAllowConnect);
    void (*sendIsSecondPhonePair)(bool isSecondPhonePair);
    void (*sendBtState)(bool isOn);
} BtHwWatchInterface;

#endif /* HW_BT_WATCH_INTERFACE_H */