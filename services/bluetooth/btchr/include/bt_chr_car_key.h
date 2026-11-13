/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_CAR_KEY_H
#define BT_CHR_CAR_KEY_H

#include <vector>
#include "bt_chr_base.h"

namespace OHOS {
namespace bluetooth {
namespace CarKey {

#define CAR_KEY_PROCESS_LOG "CAR_KEY_PROCESS_LOG"

// 车钥匙相关流程日志定义
constexpr const char *START_PAIR_LOG = "StartPair enter";
constexpr const char *START_CREDIBLE_PAIR_LOG = "StartCrediblePair enter";
constexpr const char *PAIRING_LOG = "pairing";
constexpr const char *PAIR_SUCCESS_LOG = "pair Success";
constexpr const char *PAIR_NONE_LOG = "pair None";

constexpr const char *START_GATT_CONNECT = "GattConnect enter";
constexpr const char *GATT_CONNECTING = "Gatt Connecting";
constexpr const char *GATT_CONNECTED = "Gatt Connected";
constexpr const char *GATT_CONNECT_FAILED = "Gatt Connect failed";

constexpr const char *GATT_DISCOVERYSERVICES_START = "DiscoveryServices Start";
constexpr const char *GATT_DISCOVERYSERVICE_END = "DiscoveryServices End";

constexpr const char *GATT_SET_MTU_START = "SetMtu Start";
constexpr const char *GATT_SET_MTU_END = "SetMtu End";

/**
 * record wallet chr data
 *
 * @param eventName eventName
 * @param params chr param
 */
void RecordWalletChr(const std::string &eventName, const std::vector<BtChrEventParam> &params);

/**
 * get chr data
 *
 * @return chr data str
 */
std::string GetChrData();

/**
 * set card data
 *
 * @param address car key mac
 * @param action 0 delete 1 add
 */
void SetCardChrInfo(const std::string &address, int32_t action);

/**
 * int card info
 */
void InitCardInfo();

/**
 * wallet dft event
 *
 * @param eventName eventName
 * @param param chr data
 * @return true if special chr or false
 */
bool IsWalletDftEvent(const std::string &eventName, BtChrEventParam &param);

/**
 * carkey Process log
 *
 * @param keyLogs keyLogs
 * @param address carkey address
 * @param callingName callingName
 */
void CarKeyProcessLogReport(
    const std::string &keyLogs, const std::string &address, const std::string &callingName = "");
}
}
}
#endif /* BT_CHR_CAR_KEY_H */