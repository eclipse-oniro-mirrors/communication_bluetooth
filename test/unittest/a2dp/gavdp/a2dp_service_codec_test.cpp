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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "a2dp_service.h"
#include "a2dp_service_codec.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "a2dp_def.h"
#include "hw_a2dp_vendor_codec_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;
using namespace Bluetooth;

constexpr int TIME = 4;
static BluetoothHost *host_;

class A2dpServiceCodecTest : public testing::Test {
public:
    A2dpServiceCodecTest()
    {}
    ~A2dpServiceCodecTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void A2dpServiceCodecTest::SetUpTestCase(void)
{
    HILOGI("SetUpTestCase start");
    host_ = &BluetoothHost::GetDefaultHost();

    bool isBrEnable = host_->IsBrEnabled();
    HILOGI("isBrEnable = %{public}d", isBrEnable);

    if (!isBrEnable) {
        host_->EnableBle();
        host_->EnableBt();
        sleep(TIME);
    }

    HILOGI("SetUpTestCase end");
}

void A2dpServiceCodecTest::TearDownTestCase(void)
{
    host_ = nullptr;
}

void A2dpServiceCodecTest::SetUp()
{}

void A2dpServiceCodecTest::TearDown()
{}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_001
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_001, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_001 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_DISABLED),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_SBC),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_001 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_002
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_002, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_002 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_DEFAULT),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_AAC),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_48000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_002 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_003
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_003, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_003 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_HIGHEST),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_003 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_004
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_004, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_004 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_L2HCV2),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_16000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_004 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_005
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_005, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_005 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_L2HCST),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_005 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_006
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_006, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_006 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_LDAC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_006 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_007
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_007, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_007 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_AAC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_007 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_008
 * @tc.name: ConvertAudioConfigCodec
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_008, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_008 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_SBC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_008 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_009
 * @tc.name: ConvertAudioConfigCodecChannelMode
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_009, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_009 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_SBC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(4)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_FALSE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_009 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_010
 * @tc.name: ConvertAudioConfigCodecBitsPer
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_010, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_010 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_SBC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(8),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_FALSE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_010 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_011
 * @tc.name: ConvertAudioConfigCodecSampleRate
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_011, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_011 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_SBC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(12),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_FALSE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_011 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_012
 * @tc.name: ConvertAudioConfigCodecType
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_012, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_012 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(A2DP_CODEC_PRIORITY_SBC),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(12),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_FALSE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_012 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_013
 * @tc.name: ConvertAudioConfigCodecPriority
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_013, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_013 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(8002),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    EXPECT_TRUE(OHOS::bluetooth::ConvertAudioConfigCodec(codec_config, codecInfo));
    HILOGI("A2dpServiceCodecTest_UnitTest_013 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_014
 * @tc.name: ConvertL2hcV2Bitrate
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_014, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_014 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(8002),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO),
        .codec_specific_4 = CodecCommon::L2HCST_USER_RATE_DEFAULT
    };
    A2dpSrcCodecInfo codecInfo;
    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_320K;
    OHOS::bluetooth::ConvertL2hcV2Bitrate(codec_config, codecInfo);
    EXPECT_EQ(codecInfo.codecSpecific4, CodecCommon::L2HCV2_USER_RATE_LOW);

    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_640K;
    OHOS::bluetooth::ConvertL2hcV2Bitrate(codec_config, codecInfo);
    EXPECT_EQ(codecInfo.codecSpecific4, CodecCommon::L2HCV2_USER_RATE_MID);

    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_960K;
    OHOS::bluetooth::ConvertL2hcV2Bitrate(codec_config, codecInfo);
    EXPECT_EQ(codecInfo.codecSpecific4, CodecCommon::L2HCV2_USER_RATE_HIGH);

    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_AUTO_RATE;
    OHOS::bluetooth::ConvertL2hcV2Bitrate(codec_config, codecInfo);
    EXPECT_EQ(codecInfo.codecSpecific4, CodecCommon::L2HCV2_USER_AUTO_RATE);

    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_1500K;
    OHOS::bluetooth::ConvertL2hcV2Bitrate(codec_config, codecInfo);
    EXPECT_EQ(codecInfo.codecSpecific4, CodecCommon::L2HCV2_USER_RATE_LOSSLESS);

    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_2300K;
    OHOS::bluetooth::ConvertL2hcV2Bitrate(codec_config, codecInfo);
    EXPECT_EQ(codecInfo.codecSpecific4, CodecCommon::L2HCV2_USER_RATE_2MBPS);
    HILOGI("A2dpServiceCodecTest_UnitTest_014 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_015
 * @tc.name: ConvertL2hcStBitrate
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_015, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_015 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(8002),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO),
        .codec_specific_4 = CodecCommon::L2HCST_USER_RATE_DEFAULT
    };
    A2dpSrcCodecInfo codecInfo;
    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_96K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_96K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_128K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_128K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_192K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_192K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_256K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_256K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_320K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_320K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_480K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_480K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_640K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_640K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_960K;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_960K);

    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_AUTO_RATE;
    OHOS::bluetooth::ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_AUTO_RATE);
    HILOGI("A2dpServiceCodecTest_UnitTest_015 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_016
 * @tc.name: ConvertL2hcStBitrate\ConvertL2hcV2Bitrate
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_016, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_016 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(8002),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    codecInfo.codecType = A2DP_CODEC_TYPE_L2HCST_USER;
    codecInfo.codecSpecific4 = A2DP_L2HCST_BIT_RATE_96K;
    ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCST_USER_RATE_96K);

    codecInfo.codecType = A2DP_CODEC_TYPE_L2HCV2_USER;
    codecInfo.codecSpecific4 = A2DP_L2HCV2_BIT_RATE_320K;
    ConvertL2hcStBitrate(codec_config, codecInfo);
    EXPECT_EQ(codec_config.codec_specific_4, CodecCommon::L2HCV2_USER_RATE_LOW);

    HILOGI("A2dpServiceCodecTest_UnitTest_016 end");
}

/*
 * @tc.number: A2dpServiceCodecTest_UnitTest_017
 * @tc.name: ConvertCodecConfig
 * @tc.desc:
*/
HWTEST_F(A2dpServiceCodecTest, A2dpServiceCodecTest_UnitTest_017, TestSize.Level1)
{
    HILOGI("A2dpServiceCodecTest_UnitTest_017 start");
    btav_a2dp_codec_config_t codec_config = {
        .codec_priority = static_cast<btav_a2dp_codec_priority_t>(8002),
        .codec_type = static_cast<btav_a2dp_codec_index_t>(BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST),
        .sample_rate = static_cast<btav_a2dp_codec_sample_rate_t>(A2DP_CODEC_SAMPLE_RATE_44100_48000_96000),
        .bits_per_sample = static_cast<btav_a2dp_codec_bits_per_sample_t>(BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32),
        .channel_mode = static_cast<btav_a2dp_codec_channel_mode_t>(A2DP_CODEC_CHANNEL_MODE_MONO_STEREO)
    };
    A2dpSrcCodecInfo codecInfo;
    codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_16_USER;
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        a2dpService->ConvertCodecConfig(codec_config, codecInfo);
        EXPECT_EQ(codec_config.bits_per_sample, BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16);

        codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_32_USER;
        EXPECT_EQ(codec_config.bits_per_sample, BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32);
    }
    HILOGI("A2dpServiceCodecTest_UnitTest_017 end");
}
}  // namespace Bluetooth
}  // namespace OHOS