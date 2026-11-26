/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_WIFI_DEVICE_H
#define MOCK_WIFI_DEVICE_H
namespace OHOS {
namespace Wifi {
WifiDevice::~WifiDevice()
{}
class MockWifiDevice : public Wifi::WifiDevice {
public:
    MockWifiDevice() = default;
    ~MockWifiDevice() = default;
    MOCK_METHOD(OHOS::Wifi::ErrCode, EnableWifi, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, DisableWifi, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, EnableSemiWifi, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, InitWifiProtect,
        (const Wifi::WifiProtectType &protectType, const std::string &protectName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetWifiProtectRef,
        (const Wifi::WifiProtectMode &protectType, const std::string &protectName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, PutWifiProtectRef, (const std::string &protectName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetWifiDetailState, (WifiDetailState &state), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsHeldWifiProtectRef,
        (const std::string &protectName, bool &isHeld), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsHeldWifiProtect, (bool &isHeld), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetWifiProtect, (const Wifi::WifiProtectMode &protectMode), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, PutWifiProtect, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RemoveCandidateConfig, (int networkId), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RemoveCandidateConfig, (const Wifi::WifiDeviceConfig &config), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, AddDeviceConfig,
        (const Wifi::WifiDeviceConfig &config, int &result, bool isCandidate), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetWifiRestrictedList,
        (const std::vector<Wifi::WifiRestrictedInfo> &wifiRestrictedInfoList, int mode), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetWifiTxPower, (int power), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, UpdateDeviceConfig,
        (const Wifi::WifiDeviceConfig &config, int &result), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RemoveDevice, (int networkId), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RemoveAllDevice, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, StartWifiDetection, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetDeviceConfigs,
        (std::vector<Wifi::WifiDeviceConfig> &result, bool isCandidate), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetDeviceConfig,
        (const int &networkId, Wifi::WifiDeviceConfig &config), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetDpiMarkRule,
        (const std::string &ifaceName, int uid, int protocol, int enable), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, ConnectToNetwork, (int networkId, bool isCandidate,
        int dialogTimeout), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, ConnectToDevice, (const Wifi::WifiDeviceConfig &config), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsConnected, (bool &isConnected), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, Disconnect, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsWifiActive, (bool &bActive), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsMeteredHotspot, (bool &bMeteredHotspot), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetWifiState, (int &state), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetLinkedInfo, (Wifi::WifiLinkedInfo &info), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetSignalPollInfoArray, (std::vector<Wifi::WifiSignalPollInfo> &wifiSignalPollInfos, int length), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetDisconnectedReason, (Wifi::DisconnectedReason &reason), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetCountryCode, (const std::string &countryCode), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetCountryCode, (std::string &countryCode), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RegisterCallBack,
        (const sptr<IWifiDeviceCallBack> &callback, const std::vector<std::string> &event), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetSignalLevel, (const int &rssi, const int &band, int &level), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetSupportedFeatures, (long &features), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsFeatureSupported, (long feature, bool &isSupported), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, EnableDeviceConfig, (int networkId, bool attemptEnable), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, DisableDeviceConfig, (int networkId, int64_t blockDuration), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, AllowAutoConnect, (int32_t networkId, bool isAllowed), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetIpInfo, (IpInfo &info), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetIpv6Info, (IpV6Info &info), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, ReConnect, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, ReAssociate, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetDeviceMacAddress, (std::string &result), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsBandTypeSupported, (int bandType, bool &supported), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, Get5GHzChannelList, (std::vector<int> &result), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, StartPortalCertification, (), (override));
    MOCK_METHOD(bool, SetLowLatencyMode, (bool enabled), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetAppFrozen, (std::set<int> pidList, bool isFrozen), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, ResetAllFrozenApp, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, DisableAutoJoin, (const std::string &conditionName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, EnableAutoJoin, (const std::string &conditionName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RegisterAutoJoinCondition,
        (const std::string &conditionName, const std::function<bool()> &autoJoinCondition), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, DeregisterAutoJoinCondition, (const std::string &conditionName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RegisterFilterBuilder,
        (const FilterTag &filterTag, const std::string &filterName, const FilterBuilder &filterBuilder), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, DeregisterFilterBuilder,
        (const FilterTag &filterTag, const std::string &filterName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, RegisterCommonBuilder,
        (const TagType &tagType, const std::string &tagName, const CommonBuilder &commonBuilder), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, DeregisterCommonBuilder,
        (const TagType &tagType, const std::string &tagName), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetChangeDeviceConfig,
        (ConfigChange& value, Wifi::WifiDeviceConfig &config), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, FactoryReset, (), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, LimitSpeed, (const int controlId, const int limitMode), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, EnableHiLinkHandshake,
        (bool uiFlag, std::string &bssid, Wifi::WifiDeviceConfig &deviceConfig), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetSatelliteState, (const int state), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, StartRoamToNetwork, (const int networkId, const std::string bssid,
        const bool isCandidate), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetLowTxPower, (const WifiLowPowerParam wifiLowPowerParam), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, StartConnectToUserSelectNetwork,
        (int networkId, std::string bssid, bool isCandidate), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, UpdateNetworkLagInfo,
        (const Wifi::NetworkLagType, const Wifi::NetworkLagInfo &), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, ReceiveNetworkControlInfo,
        (const WifiNetworkControlInfo& networkControlInfo), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, FetchWifiSignalInfoForVoWiFi, (Wifi::VoWifiSignalInfo &), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, IsSupportVoWifiDetect, (bool &), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetVoWifiDetectMode, (Wifi::WifiDetectConfInfo), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetVoWifiDetectMode, (Wifi::WifiDetectConfInfo &), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, SetVoWifiDetectPeriod, (int), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetVoWifiDetectPeriod, (int &), (override));
    MOCK_METHOD(OHOS::Wifi::ErrCode, GetMultiLinkedInfo,
        (std::vector<Wifi::WifiLinkedInfo> &multiLinkedInfo), (override));
};
}
}
#endif  // MOCk_WIFI_DEVICE_H
