/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef BT_SENSING_INFO_H
#define BT_SENSING_INFO_H

#include <string>
#include "bt_def.h"

namespace OHOS {
namespace bluetooth {
constexpr const char *INVALID_UUID_STRING = "00000000-0000-0000-0000-000000000000";
constexpr const char *INVALID_PKGNAME = "invalid_name";
constexpr const char *INVALID_BUSSINESS_TYPE = "0x00";

struct BleAdvData {
    BleAdvData(const std::string &uuid, bool connectable, int payloadLen, const std::string &bussinessType)
        : uuid_(uuid), connectable_(connectable), payloadLen_(payloadLen), bussinessType_(bussinessType)
    {}
    std::string uuid_;
    bool connectable_;
    int payloadLen_;
    std::string bussinessType_;
};

struct SensingInfo {
    SensingInfo()
        : addr_(INVALID_MAC_ADDRESS), uuid_(INVALID_UUID_STRING), resourceId_(0), pkgName_(INVALID_PKGNAME),
        isServer_(false), interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(0)
    {}
    ~SensingInfo() {}

    // used by ble fastest conn decision
    explicit SensingInfo(const std::string &addr)
        : addr_(addr), uuid_(INVALID_UUID_STRING), resourceId_(0), pkgName_(INVALID_PKGNAME), isServer_(false),
        interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE), scanMode_(0)
    {}
    // used by gatt connect SensingInfo construct
    explicit SensingInfo(uint32_t resourceId)
        : addr_(INVALID_MAC_ADDRESS), uuid_(INVALID_UUID_STRING), resourceId_(resourceId), pkgName_(INVALID_PKGNAME),
        isServer_(false), interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(0)
    {}
    SensingInfo(const std::string &addr, uint32_t resourceId)
        : addr_(addr), uuid_(INVALID_UUID_STRING), resourceId_(resourceId), pkgName_(INVALID_PKGNAME),
        isServer_(false), interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(0)
    {}
    // used by gatt pkgName SensingInfo construct
    SensingInfo(const std::string &pkgName, bool isServer)
        : addr_(INVALID_MAC_ADDRESS), uuid_(INVALID_UUID_STRING), resourceId_(0), pkgName_(pkgName),
        isServer_(isServer), interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(0)
    {}
    SensingInfo(const std::string &addr, const std::string &pkgName, bool isServer)
        : addr_(addr), uuid_(INVALID_UUID_STRING), resourceId_(0), pkgName_(pkgName), isServer_(isServer),
        interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE), scanMode_(0)
    {}
    // used by socket pkgName SensingInfo construct
    SensingInfo(const std::string uuid, const std::string pkgName)
        : addr_(INVALID_MAC_ADDRESS), uuid_(uuid), resourceId_(0), pkgName_(pkgName), isServer_(false), interval_(0),
        connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE), scanMode_(0)
    {}
    // used by ble conn interval update
    SensingInfo(const std::string &addr, uint16_t interval)
        : addr_(addr), uuid_(INVALID_UUID_STRING), resourceId_(0), pkgName_(INVALID_PKGNAME), isServer_(false),
        interval_(interval), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(0)
    {}
    // used by stack sensing info transfer
    SensingInfo(const std::string &addr, const std::string &uuid, uint32_t resourceId, uint16_t interval)
        : addr_(addr), uuid_(uuid), resourceId_(resourceId), pkgName_(INVALID_PKGNAME), isServer_(false),
        interval_(interval), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(0)
    {}
    // used by ble adv sensing info transfer
    SensingInfo(const BleAdvData &bleAdvData, uint32_t resourceId, uint16_t interval)
        : addr_(INVALID_MAC_ADDRESS), uuid_(bleAdvData.uuid_), resourceId_(resourceId), pkgName_(INVALID_PKGNAME),
        isServer_(false), interval_(interval), connectable_(bleAdvData.connectable_),
        payloadLen_(bleAdvData.payloadLen_), bussinessType_(bleAdvData.bussinessType_), scanMode_(0)
    {}
    // used by ble scan sensing info transfer
    SensingInfo(int scanMode, int scanStatus)
        : addr_(INVALID_MAC_ADDRESS), uuid_(INVALID_UUID_STRING), resourceId_(0), pkgName_(INVALID_PKGNAME),
        isServer_(false), interval_(0), connectable_(false), payloadLen_(0), bussinessType_(INVALID_BUSSINESS_TYPE),
        scanMode_(scanMode)
    {}

    std::string addr_;
    std::string uuid_;
    uint32_t resourceId_;
    std::string pkgName_;
    bool isServer_;
    uint16_t interval_;
    bool connectable_;
    int payloadLen_;
    std::string bussinessType_;
    int scanMode_;
};
}
}

#endif //BT_SENSING_INFO_H