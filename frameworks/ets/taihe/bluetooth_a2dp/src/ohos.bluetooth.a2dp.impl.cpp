/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_a2dp_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.a2dp.proj.hpp"
#include "ohos.bluetooth.a2dp.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_snk.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_a2dp_src_observer.h"
#include "taihe_bluetooth_a2dp_snk_observer.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_ha_event_utils.h"

namespace OHOS {
namespace Bluetooth {
constexpr int32_t MIN_AUTO_PLAY_DURATION_SEC = 3 * 1000;
constexpr int32_t MAX_AUTO_PLAY_DURATION_SEC = 20 * 1000;

class A2dpSourceProfileImpl {
public:
    A2dpSourceProfileImpl()
    {
        observer_ = std::make_shared<TaiheA2dpSourceObserver>();
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            A2dpSource *profile = A2dpSource::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void SetCurrentCodecInfo(taihe::string_view deviceId, ohos::bluetooth::a2dp::CodecInfo codecInfo)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        A2dpCodecInfo a2dpCodecInfo;
        a2dpCodecInfo.codecType = static_cast<uint8_t>(codecInfo.codecType);
        a2dpCodecInfo.bitsPerSample = static_cast<uint8_t>(codecInfo.codecBitsPerSample);
        a2dpCodecInfo.channelMode = static_cast<uint8_t>(codecInfo.codecChannelMode);
        a2dpCodecInfo.sampleRate = static_cast<uint8_t>(codecInfo.codecSampleRate);
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t errorCode = profile->SetCodecPreference(remoteDevice, a2dpCodecInfo);
        HILOGI("err: %{public}d", errorCode);
        TAIHE_BT_ASSERT_RETURN_VOID(errorCode == BT_NO_ERROR, errorCode);
    }

    ohos::bluetooth::a2dp::CodecInfo GetCurrentCodecInfo(taihe::string_view deviceId)
    {
        HILOGI("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        ohos::bluetooth::a2dp::CodecInfo codecInfo = {
            ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_INVALID,
            ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_NONE,
            ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_NONE,
            ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_NONE
        };
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, codecInfo);

        A2dpCodecInfo a2dpCodecInfo;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int errorCode = profile->GetCodecPreference(remoteDevice, a2dpCodecInfo);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode, codecInfo);

        codecInfo.codecType = ohos::bluetooth::a2dp::CodecType::from_value(a2dpCodecInfo.codecType);
        codecInfo.codecBitsPerSample =
            ohos::bluetooth::a2dp::CodecBitsPerSample::from_value(a2dpCodecInfo.bitsPerSample);
        codecInfo.codecChannelMode = ohos::bluetooth::a2dp::CodecChannelMode::from_value(a2dpCodecInfo.channelMode);
        codecInfo.codecSampleRate = ohos::bluetooth::a2dp::CodecSampleRate::from_value(a2dpCodecInfo.sampleRate);

        return codecInfo;
    }

    ohos::bluetooth::a2dp::PlayingState GetPlayingState(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::a2dp::PlayingState::from_value(0));

        int state = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t errorCode = profile->GetPlayingState(remoteDevice, state);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::a2dp::PlayingState::from_value(state));

        return ohos::bluetooth::a2dp::PlayingState::from_value(state);
    }

    void Connect(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t ret = profile->Connect(remoteDevice);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t ret = profile->Disconnect(remoteDevice);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    static taihe::array<ohos::bluetooth::a2dp::CodecBitsPerSample> ConvertCodecBitsPerSampleToTaihe(
        uint8_t bitsPerSample)
    {
        std::vector<ohos::bluetooth::a2dp::CodecBitsPerSample> bitsVec;

        if (bitsPerSample == A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_NONE_USER) {
            bitsVec.push_back(ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_NONE);
        }
        if (static_cast<uint8_t>(bitsPerSample) & A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_16_USER) {
            bitsVec.push_back(ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_16);
        }
        if (static_cast<uint8_t>(bitsPerSample) & A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_24_USER) {
            bitsVec.push_back(ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_24);
        }
        if (static_cast<uint8_t>(bitsPerSample) & A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_32_USER) {
            bitsVec.push_back(ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_32);
        }

        return taihe::array<ohos::bluetooth::a2dp::CodecBitsPerSample>{
            taihe::copy_data_t{}, bitsVec.data(), bitsVec.size()};
    }

    static taihe::array<ohos::bluetooth::a2dp::CodecChannelMode> ConvertCodecChannelModeToTaihe(uint8_t channelMode)
    {
        std::vector<ohos::bluetooth::a2dp::CodecChannelMode> channelVec;

        if (channelMode == A2dpUserCodecChannelMode::A2DP_CHANNEL_MODE_NONE_USER) {
            channelVec.push_back(ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_NONE);
        }
        if (static_cast<uint8_t>(channelMode) & A2dpUserCodecChannelMode::A2DP_SBC_CHANNEL_MODE_MONO_USER) {
            channelVec.push_back(ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_MONO);
        }
        if (static_cast<uint8_t>(channelMode) & A2dpUserCodecChannelMode::A2DP_SBC_CHANNEL_MODE_MONO_STEREO_USER) {
            channelVec.push_back(ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_STEREO);
        }

        return taihe::array<ohos::bluetooth::a2dp::CodecChannelMode>{
            taihe::copy_data_t{}, channelVec.data(), channelVec.size()};
    }

    static taihe::array<ohos::bluetooth::a2dp::CodecSampleRate> ConvertCodecSampleRateToTaihe(uint32_t sampleRate)
    {
        std::vector<ohos::bluetooth::a2dp::CodecSampleRate> rateVec;

        if (sampleRate == A2dpUserCodecSampleRate::A2DP_SAMPLE_RATE_NONE_USER) {
            rateVec.push_back(ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_NONE);
        }
        if (static_cast<uint32_t>(sampleRate) & A2dpUserCodecSampleRate::A2DP_SBC_SAMPLE_RATE_44100_USER) {
            rateVec.push_back(ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_44100);
        }
        if (static_cast<uint32_t>(sampleRate) & A2dpUserCodecSampleRate::A2DP_L2HCV2_SAMPLE_RATE_48000_USER) {
            rateVec.push_back(ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_48000);
        }
        if (static_cast<uint32_t>(sampleRate) & A2dpUserCodecSampleRate::A2DP_L2HCV2_SAMPLE_RATE_96000_USER) {
            rateVec.push_back(ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_96000);
        }

        return taihe::array<ohos::bluetooth::a2dp::CodecSampleRate>{
            taihe::copy_data_t{}, rateVec.data(), rateVec.size()};
    }

    static taihe::array<ohos::bluetooth::a2dp::CodecFrameLength> ConvertCodecFrameLengthToTaihe(uint8_t codecType)
    {
        std::vector<ohos::bluetooth::a2dp::CodecFrameLength> frameLenVec;

        // 仅 L2HC ST 和 L2HC V2 支持帧长配置，固定为 10ms
        if (codecType == A2DP_CODEC_TYPE_L2HCST_USER || codecType == A2DP_CODEC_TYPE_L2HCV2_USER) {
            frameLenVec.push_back(ohos::bluetooth::a2dp::CodecFrameLength::key_t::CODEC_FRAME_LENGTH_10MS);
        }

        return taihe::array<ohos::bluetooth::a2dp::CodecFrameLength>{
            taihe::copy_data_t{}, frameLenVec.data(), frameLenVec.size()};
    }

    // 内部 codec type 值与 Taihe 枚举值不一致（如 L2HCST 内部值为 0x0B，Taihe 枚举值为 3），
    // 需按 NAPI g_a2dpCodecTypeMap 的映射关系转换，不能直接 from_value
    static ohos::bluetooth::a2dp::CodecType ConvertCodecTypeToTaihe(uint8_t codecType)
    {
        switch (codecType) {
            case A2DP_CODEC_TYPE_SBC_USER:
                return ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_SBC;
            case A2DP_CODEC_TYPE_AAC_USER:
                return ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_AAC;
            case A2DP_CODEC_TYPE_L2HCV2_USER:
                return ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_L2HC;
            case A2DP_CODEC_TYPE_L2HCST_USER:
                return ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_L2HCST;
            case A2DP_CODEC_TYPE_LDAC_USER:
                return ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_LDAC;
            default:
                return ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_INVALID;
        }
    }

    // 精确匹配转换（对应 NAPI g_a2dpCodecBitsPerSampleMap），用于 GetCurrentCodecInfo
    static ohos::bluetooth::a2dp::CodecBitsPerSample ConvertCodecBitsPerSampleValueToTaihe(uint8_t bitsPerSample)
    {
        if (bitsPerSample == A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_16_USER) {
            return ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_16;
        }
        if (bitsPerSample == A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_24_USER) {
            return ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_24;
        }
        if (bitsPerSample == A2dpUserCodecBitsPerSample::A2DP_SAMPLE_BITS_32_USER) {
            return ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_32;
        }
        return ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_NONE;
    }

    // 精确匹配转换（对应 NAPI g_a2dpCodecChannelModeMap），用于 GetCurrentCodecInfo
    static ohos::bluetooth::a2dp::CodecChannelMode ConvertCodecChannelModeValueToTaihe(uint8_t channelMode)
    {
        if (channelMode == A2dpUserCodecChannelMode::A2DP_SBC_CHANNEL_MODE_MONO_USER) {
            return ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_MONO;
        }
        if (channelMode == A2dpUserCodecChannelMode::A2DP_SBC_CHANNEL_MODE_MONO_STEREO_USER) {
            return ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_STEREO;
        }
        return ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_NONE;
    }

    // 精确匹配转换（对应 NAPI g_a2dpCodecSampleRateMap），用于 GetCurrentCodecInfo
    static ohos::bluetooth::a2dp::CodecSampleRate ConvertCodecSampleRateValueToTaihe(uint32_t sampleRate)
    {
        if (sampleRate == A2dpUserCodecSampleRate::A2DP_SBC_SAMPLE_RATE_44100_USER) {
            return ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_44100;
        }
        if (sampleRate == A2dpUserCodecSampleRate::A2DP_SBC_SAMPLE_RATE_48000_USER
            || sampleRate == A2dpUserCodecSampleRate::A2DP_L2HCV2_SAMPLE_RATE_48000_USER) {
            return ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_48000;
        }
        if (sampleRate == A2dpUserCodecSampleRate::A2DP_L2HCV2_SAMPLE_RATE_96000_USER) {
            return ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_96000;
        }
        return ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_NONE;
    }

    static taihe::array<ohos::bluetooth::a2dp::CodecBitRate> ConvertL2hcStCodecBitRateToTaihe(uint32_t codecBitRate)
    {
        std::vector<ohos::bluetooth::a2dp::CodecBitRate> bitRateVec;

        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_AUTO_RATE) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_ABR);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_96K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_96000);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_192K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_192000);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_256K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_256000);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_320K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_320000);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_480K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_480000);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_640K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_640000);
        }
        if (codecBitRate & A2dpUserL2hcStCodecBitRate::A2DP_L2HCST_BIT_RATE_960K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_960000);
        }

        return taihe::array<ohos::bluetooth::a2dp::CodecBitRate>{
            taihe::copy_data_t{}, bitRateVec.data(), bitRateVec.size()};
    }

    static taihe::array<ohos::bluetooth::a2dp::CodecBitRate> ConvertL2hcV2CodecBitRateToTaihe(uint32_t codecBitRate)
    {
        std::vector<ohos::bluetooth::a2dp::CodecBitRate> bitRateVec;

        if (codecBitRate & A2dpUserL2hcV2CodecBitRate::A2DP_L2HCV2_BIT_RATE_AUTO_RATE) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_ABR);
        }
        if (codecBitRate & A2dpUserL2hcV2CodecBitRate::A2DP_L2HCV2_BIT_RATE_320K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_320000);
        }
        if (codecBitRate & A2dpUserL2hcV2CodecBitRate::A2DP_L2HCV2_BIT_RATE_640K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_640000);
        }
        if (codecBitRate & A2dpUserL2hcV2CodecBitRate::A2DP_L2HCV2_BIT_RATE_960K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_960000);
        }
        if (codecBitRate & A2dpUserL2hcV2CodecBitRate::A2DP_L2HCV2_BIT_RATE_1500K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_1500000);
        }
        if (codecBitRate & A2dpUserL2hcV2CodecBitRate::A2DP_L2HCV2_BIT_RATE_2300K) {
            bitRateVec.push_back(ohos::bluetooth::a2dp::CodecBitRate::key_t::CODEC_BIT_RATE_2300000);
        }

        return taihe::array<ohos::bluetooth::a2dp::CodecBitRate>{
            taihe::copy_data_t{}, bitRateVec.data(), bitRateVec.size()};
    }

    taihe::array<ohos::bluetooth::a2dp::CodecInfoList> GetCurrentFullCodecInfo(taihe::string_view deviceId)
    {
        HILOGI("start");
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_HA(taihe::get_env(), "a2dp.GetCurrentFullCodecInfo", validErrCodes);
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);

        taihe::array<ohos::bluetooth::a2dp::CodecInfoList> result = {};

        TAIHE_BT_HA_ASSERT_RETURN_VERIFY(checkRet, BT_ERR_INVALID_PARAM, result);

        A2dpCodecStatus a2dpCodecStatus;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        a2dpCodecStatus = profile->GetCodecStatus(remoteDevice);

        std::vector<ohos::bluetooth::a2dp::CodecInfoList> codecInfoListVec;

        for (const auto& cap : a2dpCodecStatus.codecInfoConfirmedCap) {
            ohos::bluetooth::a2dp::CodecInfoList codecInfoList = {
                ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_INVALID,
                taihe::array<ohos::bluetooth::a2dp::CodecBitsPerSample>{},
                taihe::array<ohos::bluetooth::a2dp::CodecChannelMode>{},
                taihe::array<ohos::bluetooth::a2dp::CodecSampleRate>{},
                taihe::array<ohos::bluetooth::a2dp::CodecBitRate>{},
                taihe::array<ohos::bluetooth::a2dp::CodecFrameLength>{}
            };
            codecInfoList.codecType = ConvertCodecTypeToTaihe(cap.codecType);
            codecInfoList.codecBitsPerSampleArray = ConvertCodecBitsPerSampleToTaihe(cap.bitsPerSample);
            codecInfoList.codecChannelModeArray = ConvertCodecChannelModeToTaihe(cap.channelMode);
            codecInfoList.codecSampleRateArray = ConvertCodecSampleRateToTaihe(cap.sampleRate);
            if (cap.codecType == A2DP_CODEC_TYPE_L2HCST_USER) {
                codecInfoList.codecBitRateArray =
                    ConvertL2hcStCodecBitRateToTaihe(static_cast<uint32_t>(cap.codecSpecific3));
            } else if (cap.codecType == A2DP_CODEC_TYPE_L2HCV2_USER) {
                codecInfoList.codecBitRateArray =
                    ConvertL2hcV2CodecBitRateToTaihe(static_cast<uint32_t>(cap.codecSpecific3));
            }
            codecInfoList.codecFrameLengthArray = ConvertCodecFrameLengthToTaihe(cap.codecType);
            codecInfoListVec.push_back(codecInfoList);
        }

        result = taihe::array<ohos::bluetooth::a2dp::CodecInfoList>{
            taihe::copy_data_t{}, codecInfoListVec.data(), codecInfoListVec.size()};

        return result;
    }

    int32_t GetAutoPlayDisabledDuration(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, 0);

        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t duration = 0;
        int err = profile->GetAutoPlayDisabledDuration(device, duration);
        HILOGI("err: %{public}d, duration: %{public}d", err, duration);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, duration);

        return duration;
    }

    void DisableAutoPlay(taihe::string_view deviceId, int32_t duration)
    {
        std::vector<int32_t> validErrCodes = { BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED,
                                               BT_ERR_INVALID_PARAM, BT_ERR_API_NOT_SUPPORT,
                                               BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
                                               BT_ERR_INTERNAL_ERROR };
        TAIHE_BT_CONTEXT_WITHOUT_HA(validErrCodes);
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(checkRet, BT_ERR_INVALID_PARAM);
        TAIHE_BT_ASSERT_RETURN_VOID_VERIFY(
            duration >= MIN_AUTO_PLAY_DURATION_SEC && duration <= MAX_AUTO_PLAY_DURATION_SEC, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->DisableAutoPlay(device, duration);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    void EnableAutoPlay(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->EnableAutoPlay(device);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::string remoteAddr = std::string(deviceId);
        A2dpSource *profile = A2dpSource::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, 1);
        int btConnectState = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int errorCode = profile->GetDeviceState(device, btConnectState);
        int status = TaiheUtils::GetProfileConnectionState(btConnectState);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(status));
        HILOGD("status: %{public}d", status);

        return ohos::bluetooth::constant::ProfileConnectionState::from_value(status);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        A2dpSource *profile = A2dpSource::GetProfile();
        std::vector<int> states;
        states.push_back(1);
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetDevicesByStates(states, devices);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode, taihe::array<taihe::string>{});

        std::vector<std::string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    void DisableAbsoluteVolumeSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_CLOSE;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->SetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    void EnableAbsoluteVolumeSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->SetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    bool IsAbsoluteVolumeEnabledSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, false);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->GetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, false);
        if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN) {
            return true;
        }
        return false;
    }

    bool IsAbsoluteVolumeSupportedSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, false);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->GetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, false);
        if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
            return false;
        }
        return true;
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        int transport = BT_TRANSPORT_BREDR;
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGD("err: %{public}d, strategy: %{public}d", err, strategy);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));

        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(taihe::string_view deviceId,
                                   ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        int transport = BT_TRANSPORT_BREDR;
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }
private:
    std::shared_ptr<TaiheA2dpSourceObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

::ohos::bluetooth::a2dp::A2dpSourceProfile CreateA2dpSrcProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<A2dpSourceProfileImpl, ohos::bluetooth::a2dp::A2dpSourceProfile>();
}

class A2dpSinkProfileImpl {
public:
    A2dpSinkProfileImpl()
    {
        observer_ = std::make_shared<TaiheA2dpSinkObserver>();
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            A2dpSink *profile = A2dpSink::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void Connect(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        A2dpSink *profile = A2dpSink::GetProfile();
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        bool ret = profile->Connect(remoteDevice);
        HILOGI("ret: %{public}d", ret);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        A2dpSink *profile = A2dpSink::GetProfile();
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        bool ret = profile->Disconnect(remoteDevice);
        HILOGI("ret: %{public}d", ret);
    }

    ohos::bluetooth::a2dp::PlayingState GetPlayingState(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::a2dp::PlayingState::from_value(0));
        
        int state = 0;
        A2dpSink *profile = A2dpSink::GetProfile();
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->GetPlayingState(remoteDevice, state);
        HILOGI("errorCode: %{public}d, state: %{public}d", errorCode, state);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::a2dp::PlayingState::from_value(state));
        return ohos::bluetooth::a2dp::PlayingState::from_value(state);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::string remoteAddr = std::string(deviceId);
        A2dpSink *profile = A2dpSink::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int btConnectState = profile->GetDeviceState(device);
        int status = TaiheUtils::GetProfileConnectionState(btConnectState);
        HILOGD("status: %{public}d", status);
        return ohos::bluetooth::constant::ProfileConnectionState::from_value(status);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        A2dpSink *profile = A2dpSink::GetProfile();
        std::vector<int> states;
        states.push_back(1);
        std::vector<BluetoothRemoteDevice> devices = profile->GetDevicesByStates(states);

        std::vector<std::string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSink *profile = A2dpSink::GetProfile();
        int strategy = profile->GetConnectStrategy(remoteDevice);
        HILOGD("strategy: %{public}d", strategy);
        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(taihe::string_view deviceId,
        ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSink *profile = A2dpSink::GetProfile();
        bool ret = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("ret: %{public}d", ret);
    }

private:
    std::shared_ptr<TaiheA2dpSinkObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

::ohos::bluetooth::a2dp::A2dpSinkProfile CreateA2dpSnkProfile()
{
    return taihe::make_holder<A2dpSinkProfileImpl, ohos::bluetooth::a2dp::A2dpSinkProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateA2dpSrcProfile(OHOS::Bluetooth::CreateA2dpSrcProfile);
TH_EXPORT_CPP_API_CreateA2dpSnkProfile(OHOS::Bluetooth::CreateA2dpSnkProfile);
// NOLINTEND
