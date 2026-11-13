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

#ifndef MOCK_WIFI_HOTSPOT_H
#define MOCK_WIFI_HOTSPOT_H
namespace OHOS {
namespace Wifi {
WifiHotspot::~WifiHotspot()
{}
class MockWifiHotSpot : public Wifi::WifiHotspot {
public:
    MockWifiHotSpot() = default;
    ~MockWifiHotSpot() = default;
    MOCK_METHOD(ErrCode, IsHotspotActive, (bool &isActive), (override));
    MOCK_METHOD(ErrCode, IsHotspotDualBandSupported, (bool &isSupported), (override));
    MOCK_METHOD(ErrCode, IsOpenSoftApAllowed, (bool &isSupported), (override));
    MOCK_METHOD(ErrCode, GetHotspotState, (int &state), (override));
    MOCK_METHOD(ErrCode, GetHotspotConfig, (HotspotConfig &config), (override));
    MOCK_METHOD(ErrCode, SetHotspotConfig, (const HotspotConfig &config), (override));
    MOCK_METHOD(ErrCode, SetHotspotIdleTimeout, (int time), (override));
    MOCK_METHOD(ErrCode, GetStationList, (std::vector<StationInfo> &result), (override));
    MOCK_METHOD(ErrCode, DisassociateSta, (const StationInfo &info), (override));
    MOCK_METHOD(ErrCode, EnableHotspot, (const ServiceType type), (override));
    MOCK_METHOD(ErrCode, DisableHotspot, (const ServiceType type), (override));
    MOCK_METHOD(ErrCode, GetBlockLists, (std::vector<StationInfo> &infos), (override));
    MOCK_METHOD(ErrCode, AddBlockList, (const StationInfo &info), (override));
    MOCK_METHOD(ErrCode, DelBlockList, (const StationInfo &info), (override));
    MOCK_METHOD(ErrCode, GetValidBands, (std::vector<BandType> &bands), (override));
    MOCK_METHOD(ErrCode, GetValidChannels, (BandType band, std::vector<int32_t> &validchannels), (override));
    MOCK_METHOD(ErrCode, RegisterCallBack, (const sptr<IWifiHotspotCallback> &callback,
        const std::vector<std::string> &event), (override));
    MOCK_METHOD(ErrCode, GetSupportedFeatures, (long &features), (override));
    MOCK_METHOD(bool, IsFeatureSupported, (long feature), (override));
    MOCK_METHOD(ErrCode, GetSupportedPowerModel, (std::set<PowerModel>& setPowerModelList), (override));
    MOCK_METHOD(ErrCode, GetPowerModel, (PowerModel& model), (override));
    MOCK_METHOD(ErrCode, SetPowerModel, (const PowerModel& model), (override));
    MOCK_METHOD(ErrCode, GetApIfaceName, (std::string& ifaceName), (override));
    MOCK_METHOD(ErrCode, EnableLocalOnlyHotspot, (const ServiceType type), (override));
    MOCK_METHOD(ErrCode, DisableLocalOnlyHotspot, (const ServiceType type), (override));
    MOCK_METHOD(ErrCode, GetHotspotMode, (HotspotMode &mode), (override));
    MOCK_METHOD(ErrCode, GetLocalOnlyHotspotConfig, (HotspotConfig &config), (override));
};
}
}
#endif  // MOCk_WIFI_DEVICE_H
