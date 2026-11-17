/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_edm"
#endif

#include "bluetooth_edm_manager.h"
#include <regex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "cJSON.h"
#include "bluetooth_manager_proxy.h"
#include "log.h"
#include "bluetooth_errorcode.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_util.h"
#include "bluetooth_os_account.h"
#include "permission_manager.h"
#include "sg_collect_client.h"
#include "ipc_skeleton.h"
#include "json_utils.h"
#include "string_ex.h"

namespace OHOS {
namespace bluetooth {
static constexpr int MAX_ACCOUNT_SIZE = 1000;
static constexpr int MAX_PROFILE_SIZE = 500;
static const char *const BLUETOOTH_ALLOWEDLIST_CHANGED_EVENT = "com.ohos.edm.bluetoothdeviceschanged";
static const char *const BLUETOOTH_DISALLOWEDLIST_CHANGED_EVENT = "com.ohos.edm.disallowedbluetoothdeviceschanged";
static const char *const BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT = "usual.event.EDM_CONFIG_CHANGED";
static const char *const BLUETOOTH_ACCOUNT_DENYLIST_CONFIG_FILEPATH =
    "/data/service/el1/public/edm/config/system/all/bluetooth/config.json";

std::mutex BluetoothEdmManager::instanceMutexLock_;
BluetoothEdmManager& BluetoothEdmManager::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutexLock_);
    static BluetoothEdmManager ins;
    return ins;
}

void BluetoothEdmManager::Init()
{
    CHECK_AND_RETURN_LOG(!isInit_, "Already Init");
    int ret = GetAllowedList();
    HILOGI("GetAllowedList ret: %{public}d", ret);
    ret = GetDisallowedList();
    HILOGI("GetDisallowedList ret: %{public}d", ret);
    ret = GetAccountSendDenylist();
    HILOGI("GetAccountSendDenylist: %{public}d", ret);
    ret = GetAccountRecvDenylist();
    HILOGI("GetAccountRecvDenylist: %{public}d", ret);

    EventFwk::MatchingSkills accountMatchingSkills;
    accountMatchingSkills.AddEvent(BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT);
    EventFwk::CommonEventSubscribeInfo accountSubscriberInfo(accountMatchingSkills);
    accountSubscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    accountSubscriberInfo.SetPermission("ohos.permission.MANAGE_EDM_POLICY");
    HILOGI("SetPermission MANAGE_EDM_POLICY");
    accountDisallowedEventSubscriber_ = std::make_shared<BluetoothCommonEventSubscriber>(accountSubscriberInfo);
    CHECK_AND_RETURN_LOG(EventFwk::CommonEventManager::SubscribeCommonEvent(accountDisallowedEventSubscriber_),
        "SubscribeCommonEvent account denylist fail.");

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(BLUETOOTH_ALLOWEDLIST_CHANGED_EVENT);
    matchingSkills.AddEvent(BLUETOOTH_DISALLOWEDLIST_CHANGED_EVENT);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    subscriberInfo.SetPermission("ohos.permission.MANAGE_BLUETOOTH");
    HILOGI("SetPermission MANAGE_BLUETOOTH");
    deviceEventSubscriber_ = std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    CHECK_AND_RETURN_LOG(EventFwk::CommonEventManager::SubscribeCommonEvent(deviceEventSubscriber_),
        "SubscribeCommonEvent device event fail.");
    isInit_ = true;
}

bool BluetoothEdmManager::IsValidAddress(const std::string& address)
{
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(address, deviceIdRegex);
}

bool BluetoothEdmManager::ConvertToRealAddress(const std::string &mac, std::string &realMac)
{
    EdmRealAddrConvertFunc func = nullptr;
    {
        std::lock_guard<std::mutex> lock(convertFuncMutexLock_);
        func = realAddrConvertFunc_;
    }
    if (func == nullptr) {
        HILOGD("real address convert func is not registered");
        return false;
    }
    RawAddress inputAddr(mac);
    RawAddress realUsedAddr;
    if (!func(inputAddr, realUsedAddr) || realUsedAddr.GetAddress().empty()) {
        HILOGD("convert to real address failed, mac: %{public}s", GetEncryptAddr(mac).c_str());
        return false;
    }
    realMac = UpperStr(realUsedAddr.GetAddress());
    return true;
}

bool BluetoothEdmManager::IsAddrInList(const std::map<std::string, std::string> &listMap,
    const std::set<std::string> &realAddrSet, const std::string &addr)
{
    // Check both the key (original address) and the value (real address).
    return ((realAddrSet.find(addr) != realAddrSet.end() || listMap.find(addr) != listMap.end()));
}

void BluetoothEdmManager::UpdateListMap(std::map<std::string, std::string> &listMap,
    std::set<std::string> &realAddrSet, const std::vector<std::string> &addrList)
{
    std::map<std::string, std::string> newListMap;
    for (std::vector<std::string>::const_iterator it = addrList.begin(); it != addrList.end(); ++it) {
        if (!IsValidAddress(*it)) {
            continue;
        }
        std::string mac = UpperStr(*it);
        std::string realMac = "";
        // A valid real address which differs from the key has the highest priority.
        if (!ConvertToRealAddress(mac, realMac) || (realMac == mac)) {
            // The virtual address may have expired, keep the real address resolved before.
            std::map<std::string, std::string>::const_iterator oldIt = listMap.find(mac);
            if ((oldIt != listMap.end()) && (oldIt->second != mac)) {
                newListMap[mac] = oldIt->second;
                continue;
            }
            // Keep the original address as the value when it cannot be converted.
            realMac = mac;
        }
        newListMap[mac] = realMac;
    }
    // The new list is authoritative, the keys must be exactly the same as the list.
    listMap.swap(newListMap);
    realAddrSet.clear();
    for (std::map<std::string, std::string>::const_iterator it = listMap.begin(); it != listMap.end(); ++it) {
        realAddrSet.insert(it->second);
    }
}

bool BluetoothEdmManager::IsAllowedConnect(const std::string &mac)
{
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(mac), false, "Invalid address");
    std::string macTemp = UpperStr(mac);
    std::string realMac = "";
    if (!ConvertToRealAddress(macTemp, realMac)) {
        // Address which cannot be converted is treated as a real address.
        realMac = macTemp;
    }
    {
        std::shared_lock<std::shared_mutex> lock(allowedListMutexLock_);
        if (!allowedListMap_.empty()) {
            if (IsAddrInList(allowedListMap_, realAllowedAddrSet_, realMac)) {
                HILOGI("IsAllowedConnect mac is in allowedList");
                return true;
            }
            HILOGE("[EDM MODE]device %{public}s restricted by edm allowedList.", GetEncryptAddr(mac).c_str());
            BtChrDftEventWriteInt(CHR_USER_DISCONNECT, mac, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
            return false;
        }
    }
    std::shared_lock<std::shared_mutex> lock(disallowedListMutexLock_);
    CHECK_AND_RETURN_LOG_RET(!disallowedListMap_.empty(), true, "IsAllowedConnect disallowedList is empty");
    if (IsAddrInList(disallowedListMap_, realDisallowedAddrSet_, realMac)) {
        HILOGE("[EDM MODE]device %{public}s restricted by edm disallowedList.", GetEncryptAddr(mac).c_str());
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, mac, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
        return false;
    }
    HILOGI("IsAllowedConnect mac is not in disallowedList");
    return true;
}

bool BluetoothEdmManager::IsAccountConfigAllowedInternal(const ControlInterceptMessage &msg,
    const std::string &profile, std::shared_mutex &mutexLock,
    std::map<std::string, std::vector<std::string>> &denylist)
{
    std::shared_lock<std::shared_mutex> lock(mutexLock);
    int osAccountId = BluetoothOsAccount::GetActiveOsAccountId();
    HILOGI("GetActiveOsAccountId %{public}d", osAccountId);
    auto it = denylist.find(std::to_string(osAccountId));
    if (it == denylist.end()) {
        HILOGI("osAccountId not found");
        return true;
    } else {
        if (find(it->second.begin(), it->second.end(), profile) == it->second.end()) {
            HILOGI("profile not in denylist");
            return true;
        } else if (profile == "OPP") {
            HILOGE("profile %{public}s restricted by edm account denylist.", profile.c_str());
            BtChrDftEventWriteInt(CHR_USER_DISCONNECT, msg.addr, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
            return false;
        } else if (((profile == "GATT") || (profile == "SPP")) && !msg.isSystemHap && !msg.isNativeCaller) {
            HILOGI("profile %{public}s restricted by edm account denylist.", profile.c_str());
            BtChrDftEventWriteInt(CHR_USER_DISCONNECT, msg.addr, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
            return false;
        } else {
            HILOGI("profile %{public}s allow connect.", profile.c_str());
            return true;
        }
    }
}

bool BluetoothEdmManager::IsAccountConfigAllowedConnect(const ControlInterceptMessage &msg, const std::string &profile)
{
    return IsAccountConfigAllowedInternal(msg, profile, accountDenylistMutexLock_, accountDenylist_);
}

bool BluetoothEdmManager::IsAccountConfigAllowedReceive(const ControlInterceptMessage &msg, const std::string &profile)
{
    return IsAccountConfigAllowedInternal(msg, profile, accountRecvDenylistMutexLock_, accountRecvDenylist_);
}

int BluetoothEdmManager::GetAllowedList()
{
    std::unique_lock<std::shared_mutex> lock(allowedListMutexLock_);
    auto proxy = EDM::BluetoothManagerProxy::GetBluetoothManagerProxy();
    CHECK_AND_RETURN_LOG_RET(proxy, -1, "GetBluetoothManagerProxy failed");
    std::vector<std::string> allowedListTmp;
    int result = proxy->GetAllowedBluetoothDevices(allowedListTmp);
    if (result != 0) {
        HILOGE("GetAllowedBluetoothDevices failed, ret: %{public}d", result);
        return result;
    }
    UpdateListMap(allowedListMap_, realAllowedAddrSet_, allowedListTmp);
    return result;
}

int BluetoothEdmManager::GetDisallowedList()
{
    std::unique_lock<std::shared_mutex> lock(disallowedListMutexLock_);
    auto proxy = EDM::BluetoothManagerProxy::GetBluetoothManagerProxy();
    CHECK_AND_RETURN_LOG_RET(proxy, -1, "GetBluetoothManagerProxy failed");
    std::vector<std::string> disallowedListTmp;
    int result = proxy->GetDisallowedBluetoothDevices(disallowedListTmp);
    if (result != 0) {
        HILOGE("GetDisallowedBluetoothDevices failed, ret: %{public}d", result);
        return result;
    }
    UpdateListMap(disallowedListMap_, realDisallowedAddrSet_, disallowedListTmp);
    return result;
}

static void ReadStringFromFile(const std::string &path, std::string &fileStr)
{
    if (path.length() >= PATH_MAX) {
        HILOGE("length invalid");
        return;
    }
    char resolvedPath[PATH_MAX] = {0};
    char *result = realpath(path.c_str(), resolvedPath);
    if (!result) {
        HILOGE("realpath failed");
        return;
    }
    std::ifstream ifs(result);
    if (!ifs.is_open()) {
        HILOGE("ifstream open failed");
        return;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    fileStr = std::string(buffer.str());
    ifs.close();
}

int BluetoothEdmManager::ParseAccountDenylist(cJSON *obj, std::map<std::string, std::vector<std::string>> &denylist)
{
    int accountSize = cJSON_GetArraySize(obj);
    if (accountSize > MAX_ACCOUNT_SIZE) {
        HILOGE("accountSize %{public}d invalid", accountSize);
        return -1;
    }
    std::vector<std::string> profileDenylist;
    for (int i = 0; i < accountSize; ++i) {
        cJSON *childNode = cJSON_GetArrayItem(obj, i);
        if ((childNode == nullptr) || (childNode->string == nullptr)) {
            continue;
        }
        HILOGI("childNode string is %{public}s", childNode->string);
        int profileSize = cJSON_GetArraySize(childNode);
        if (profileSize > MAX_PROFILE_SIZE) {
            HILOGE("profileSize invalid");
            continue;
        }
        HILOGI("childNode profileSize is %{public}d", profileSize);
        profileDenylist.clear();
        for (int j = 0; j < profileSize; ++j) {
            cJSON *profileNode = cJSON_GetArrayItem(childNode, j);
            if ((profileNode == nullptr) || (profileNode->valuestring == nullptr)) {
                continue;
            }
            std::string profile = profileNode->valuestring;
            HILOGI("profile:%{public}s", profile.c_str());
            std::transform(profile.begin(), profile.end(), profile.begin(), ::toupper);
            profileDenylist.push_back(profile);
        }
        denylist[childNode->string] = profileDenylist;
    }
    return 0;
}

int BluetoothEdmManager::GetAccountDenylistInternal(std::shared_mutex& mutexLock,
    std::map<std::string, std::vector<std::string>>& denylist, const char* jsonKey)
{
    std::unique_lock<std::shared_mutex> lock(mutexLock);
    denylist.clear();
    std::string protocolDenyList;
    ReadStringFromFile(std::string(BLUETOOTH_ACCOUNT_DENYLIST_CONFIG_FILEPATH), protocolDenyList);
    CHECK_AND_RETURN_LOG_RET(!protocolDenyList.empty(), -1, "get protocolDenyList failed");
    cJSON *root = JsonUtil::ReadJsonContents(protocolDenyList);
    CHECK_AND_RETURN_LOG_RET(root, -1, "json parse failed");
    cJSON *obj = cJSON_GetObjectItem(root, jsonKey);
    if (obj == nullptr) {
        HILOGE("get %{public}s failed", jsonKey);
        cJSON_Delete(root);
        return -1;
    }

    if (ParseAccountDenylist(obj, denylist) != 0) {
        cJSON_Delete(root);
        return -1;
    }
    cJSON_Delete(root);
    return 0;
}

int BluetoothEdmManager::GetAccountSendDenylist()
{
    return GetAccountDenylistInternal(accountDenylistMutexLock_, accountDenylist_, "ProtocolDenyList");
}

int BluetoothEdmManager::GetAccountRecvDenylist()
{
    return GetAccountDenylistInternal(accountRecvDenylistMutexLock_, accountRecvDenylist_, "ProtocolRecDenyList");
}

void BluetoothEdmManager::OnEdmListChanged(const std::string &action)
{
    int ret = -1;
    if (action == BLUETOOTH_ALLOWEDLIST_CHANGED_EVENT) {
        HILOGI("OnReceiveEvent allowedList changed");
        ret = GetAllowedList();
        if (ret != 0) {
            HILOGE("GetAllowedList failed, ret: %{public}d", ret);
        }
    } else if (action == BLUETOOTH_DISALLOWEDLIST_CHANGED_EVENT) {
        HILOGI("OnReceiveEvent disallowedList changed");
        ret = GetDisallowedList();
        if (ret != 0) {
            HILOGE("GetDisallowedList failed, ret: %{public}d", ret);
        }
    } else if (action == BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT) {
        HILOGI("OnReceiveEvent account denylist changed");
        ret = GetAccountSendDenylist();
        if (ret != 0) {
            HILOGE("GetAccountSendDenylist failed, ret: %{public}d", ret);
        }
        ret = GetAccountRecvDenylist();
        if (ret != 0) {
            HILOGE("GetAccountRecvDenylist failed, ret: %{public}d", ret);
        }
    }
}

void BluetoothEdmManager::RegisterRealAddrConvertFunc(const EdmRealAddrConvertFunc &func)
{
    std::lock_guard<std::mutex> lock(convertFuncMutexLock_);
    realAddrConvertFunc_ = func;
    HILOGI("RegisterRealAddrConvertFunc %{public}s", func == nullptr ? "failed" : "success");
}

BluetoothEdmManager::BluetoothEdmManager()
{
}

BluetoothEdmManager::~BluetoothEdmManager()
{
}

/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/
static bool EdmIsAllowedConn(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    return true;
}

static bool EdmIsAllowedGattConn(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "GATT");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedGattReceive(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedReceive(msg, "GATT");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedSppConn(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "SPP");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedOppWrite(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        if (msg.filePaths.empty()) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        }
        for (const auto& path : msg.filePaths) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid, path);
        }
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "OPP");
    if (!accountAllowed) {
        if (msg.filePaths.empty()) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        }
        for (const auto& path : msg.filePaths) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid, path);
        }
    }
    return accountAllowed;
}

static bool EdmIsAllowedOppRead(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedReceive(msg, "OPP");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedGattAddService(const ControlInterceptMessage &msg)
{
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "GATT");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedSocketListen(const ControlInterceptMessage &msg)
{
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedReceive(msg, "SPP");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static ControlInterceptPlugin g_edmManagerPlugin = {
    .fastestGattConn = EdmIsAllowedGattConn,
    .gattConn = EdmIsAllowedGattConn,
    .avrcpConn = EdmIsAllowedConn,
    .pairConn = EdmIsAllowedConn,
    .aclConn = EdmIsAllowedConn,
    .a2dpConn = EdmIsAllowedConn,
    .hfpConn = EdmIsAllowedConn,
    .hidConn = EdmIsAllowedConn,
    .socketConn = EdmIsAllowedSppConn,
    .oppWrite = EdmIsAllowedOppWrite,
    .gattAddService = EdmIsAllowedGattAddService,
    .socketListen = EdmIsAllowedSocketListen,
    .oppRead = EdmIsAllowedOppRead,
    .gattReceive = EdmIsAllowedGattReceive,
};

REGISTER_CONTROL_INTERCEPT_PLUGIN(g_edmManagerPlugin);
/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/

void BluetoothEdmManager::ReportEdmAuditInfo(const std::string &connectedDeviceId, int appId, int userId,
    const std::string &filePath)
{
    std::string appIdStr = std::to_string(appId);
    int64_t happenTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    cJSON *outJson = cJSON_CreateObject();
    if (outJson == nullptr) {
        HILOGE("ReportEdmAuditInfo json object created error");
        return;
    }
    cJSON_AddStringToObject(outJson, "connectedDeviceId", GetEncryptAddr(connectedDeviceId).c_str());
    cJSON_AddStringToObject(outJson, "appId", appIdStr.c_str());
    cJSON_AddNumberToObject(outJson, "userId", userId);
    cJSON_AddNumberToObject(outJson, "happTime", happenTime);
    cJSON_AddStringToObject(outJson, "file_path", filePath.c_str());
    char *jsonContent = cJSON_PrintUnformatted(outJson);
    if (jsonContent == nullptr) {
        HILOGE("ReportEdmAuditInfo print json unformatted error");
        cJSON_Delete(outJson);
        outJson = nullptr;
        return;
    }
    std::string content = std::string(jsonContent);
    cJSON_free(jsonContent);
    cJSON_Delete(outJson);
    outJson = nullptr;
    std::shared_ptr<Security::SecurityGuard::EventInfo> eventInfo =
        std::make_shared<Security::SecurityGuard::EventInfo>(0x03000200, "1.0", content);
    int ret = Security::SecurityGuard::NativeDataCollectKit::ReportSecurityInfo(eventInfo);
    HILOGI("report appIdStr:%{public}s,userId:%{public}d, ret:%{public}d", appIdStr.c_str(), userId, ret);
}
} // namespace bluetooth
} // namespace OHOS