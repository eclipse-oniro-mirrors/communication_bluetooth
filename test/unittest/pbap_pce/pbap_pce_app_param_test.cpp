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

#include <gtest/gtest.h>
#include <cstring>
#include <endian.h>
#include "bluetooth_log.h"
#include "pbap_pce_app_params.h"
#include "obex_header.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
using namespace std;

class PbapPceAppParamTest : public testing::Test {
public:
    PbapPceAppParamTest();
    ~PbapPceAppParamTest();

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
};

PbapPceAppParamTest::PbapPceAppParamTest() {}

PbapPceAppParamTest::~PbapPceAppParamTest() {}

void PbapPceAppParamTest::SetUpTestCase(void) {}

void PbapPceAppParamTest::TearDownTestCase(void) {}

void PbapPceAppParamTest::SetUp(void) {}

void PbapPceAppParamTest::TearDown(void) {}

/*
 * @tc.number: PbapPce_AppParam_Order_00100
 * @tc.name: SetOrder and GetOrder
 * @tc.desc: Set and get order
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_Order_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_Order_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasOrder());
    appParams.SetOrder(0x01);
    EXPECT_TRUE(appParams.HasOrder());
    EXPECT_EQ(appParams.GetOrder(), 0x01);
    HILOGI("PbapPce_AppParam_Order_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_SearchProperty_00100
 * @tc.name: SetSearchProperty and GetSearchProperty
 * @tc.desc: Set and get search property
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_SearchProperty_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_SearchProperty_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasSearchProperty());
    appParams.SetSearchProperty(0x01);
    EXPECT_TRUE(appParams.HasSearchProperty());
    EXPECT_EQ(appParams.GetSearchProperty(), 0x01);
    HILOGI("PbapPce_AppParam_SearchProperty_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_MaxListCount_00100
 * @tc.name: SetMaxListCount and GetMaxListCount
 * @tc.desc: Set and get max list count
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_MaxListCount_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_MaxListCount_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasMaxListCount());
    appParams.SetMaxListCount(100);
    EXPECT_TRUE(appParams.HasMaxListCount());
    EXPECT_EQ(appParams.GetMaxListCount(), 100);
    HILOGI("PbapPce_AppParam_MaxListCount_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_ListStartOffset_00100
 * @tc.name: SetListStartOffset and GetListStartOffset
 * @tc.desc: Set and get list start offset
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_ListStartOffset_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_ListStartOffset_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasListStartOffset());
    appParams.SetListStartOffset(50);
    EXPECT_TRUE(appParams.HasListStartOffset());
    EXPECT_EQ(appParams.GetListStartOffset(), 50);
    HILOGI("PbapPce_AppParam_ListStartOffset_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_PropertySelector_00100
 * @tc.name: SetPropertySelector and GetPropertySelector
 * @tc.desc: Set and get property selector
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_PropertySelector_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_PropertySelector_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasPropertySelector());
    appParams.SetPropertySelector(0x10000081ULL);
    EXPECT_TRUE(appParams.HasPropertySelector());
    EXPECT_EQ(appParams.GetPropertySelector(), 0x10000081ULL);
    HILOGI("PbapPce_AppParam_PropertySelector_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_Format_00100
 * @tc.name: SetFormat and GetFormat
 * @tc.desc: Set and get format
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_Format_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_Format_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasFormat());
    appParams.SetFormat(0x00);
    EXPECT_TRUE(appParams.HasFormat());
    EXPECT_EQ(appParams.GetFormat(), 0x00);
    HILOGI("PbapPce_AppParam_Format_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_PhonebookSize_00100
 * @tc.name: SetPhonebookSize and GetPhonebookSize
 * @tc.desc: Set and get phonebook size
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_PhonebookSize_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_PhonebookSize_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasPhonebookSize());
    appParams.SetPhonebookSize(200);
    EXPECT_TRUE(appParams.HasPhonebookSize());
    EXPECT_EQ(appParams.GetPhonebookSize(), 200);
    HILOGI("PbapPce_AppParam_PhonebookSize_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_NewMissedCalls_00100
 * @tc.name: SetNewMissedCalls and GetNewMissedCalls
 * @tc.desc: Set and get new missed calls
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_NewMissedCalls_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_NewMissedCalls_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasNewMissedCalls());
    appParams.SetNewMissedCalls(5);
    EXPECT_TRUE(appParams.HasNewMissedCalls());
    EXPECT_EQ(appParams.GetNewMissedCalls(), 5);
    HILOGI("PbapPce_AppParam_NewMissedCalls_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_VcardSelector_00100
 * @tc.name: SetVcardSelector and GetVcardSelector
 * @tc.desc: Set and get vcard selector
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_VcardSelector_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_VcardSelector_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasVcardSelector());
    appParams.SetVcardSelector(0x82ULL);
    EXPECT_TRUE(appParams.HasVcardSelector());
    EXPECT_EQ(appParams.GetVcardSelector(), 0x82ULL);
    HILOGI("PbapPce_AppParam_VcardSelector_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_VcardSelectorOperator_00100
 * @tc.name: SetVcardSelectorOperator and GetVcardSelectorOperator
 * @tc.desc: Set and get vcard selector operator
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_VcardSelectorOperator_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_VcardSelectorOperator_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasVcardSelectorOperator());
    appParams.SetVcardSelectorOperator(0x01);
    EXPECT_TRUE(appParams.HasVcardSelectorOperator());
    EXPECT_EQ(appParams.GetVcardSelectorOperator(), 0x01);
    HILOGI("PbapPce_AppParam_VcardSelectorOperator_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_ResetNewMissedCalls_00100
 * @tc.name: SetResetNewMissedCalls and GetResetNewMissedCalls
 * @tc.desc: Set and get reset new missed calls
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_ResetNewMissedCalls_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_ResetNewMissedCalls_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasResetNewMissedCalls());
    appParams.SetResetNewMissedCalls(0x01);
    EXPECT_TRUE(appParams.HasResetNewMissedCalls());
    EXPECT_EQ(appParams.GetResetNewMissedCalls(), 0x01);
    HILOGI("PbapPce_AppParam_ResetNewMissedCalls_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_PbapSupportedFeatures_00100
 * @tc.name: SetPbapSupportedFeatures and GetPbapSupportedFeatures
 * @tc.desc: Set and get pbap supported features
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_PbapSupportedFeatures_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_PbapSupportedFeatures_00100 start");
    PbapPceAppParams appParams;
    EXPECT_FALSE(appParams.HasPbapSupportedFeatures());
    appParams.SetPbapSupportedFeatures(0x00000003);
    EXPECT_TRUE(appParams.HasPbapSupportedFeatures());
    EXPECT_EQ(appParams.GetPbapSupportedFeatures(), 0x00000003);
    HILOGI("PbapPce_AppParam_PbapSupportedFeatures_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_Name_00100
 * @tc.name: SetName and GetName
 * @tc.desc: Set and get name
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_Name_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_Name_00100 start");
    PbapPceAppParams appParams;
    appParams.SetName("telecom/pb.vcf");
    EXPECT_EQ(appParams.GetName(), "telecom/pb.vcf");
    HILOGI("PbapPce_AppParam_Name_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_SearchValueUtf8_00100
 * @tc.name: SetSearchValueUtf8 and GetSearchValueUtf8
 * @tc.desc: Set and get search value utf8
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_SearchValueUtf8_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_SearchValueUtf8_00100 start");
    PbapPceAppParams appParams;
    std::vector<uint8_t> val = {0x31, 0x32, 0x33, 0x34, 0x35};
    appParams.SetSearchValueUtf8(val);
    EXPECT_EQ(appParams.GetSearchValueUtf8(), val);
    HILOGI("PbapPce_AppParam_SearchValueUtf8_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_PrimaryFolderVersion_00100
 * @tc.name: SetPrimaryFolderVersion and GetPrimaryFolderVersion
 * @tc.desc: Set and get primary folder version
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_PrimaryFolderVersion_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_PrimaryFolderVersion_00100 start");
    PbapPceAppParams appParams;
    std::vector<uint8_t> val(16, 0xAB);
    appParams.SetPrimaryFolderVersion(val);
    EXPECT_EQ(appParams.GetPrimaryFolderVersion(), val);
    HILOGI("PbapPce_AppParam_PrimaryFolderVersion_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_SecondaryFolderVersion_00100
 * @tc.name: SetSecondaryFolderVersion and GetSecondaryFolderVersion
 * @tc.desc: Set and get secondary folder version
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_SecondaryFolderVersion_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_SecondaryFolderVersion_00100 start");
    PbapPceAppParams appParams;
    std::vector<uint8_t> val(16, 0xCD);
    appParams.SetSecondaryFolderVersion(val);
    EXPECT_EQ(appParams.GetSecondaryFolderVersion(), val);
    HILOGI("PbapPce_AppParam_SecondaryFolderVersion_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_DatabaseIdentifier_00100
 * @tc.name: SetDatabaseIdentifier and GetDatabaseIdentifier
 * @tc.desc: Set and get database identifier
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_DatabaseIdentifier_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_DatabaseIdentifier_00100 start");
    PbapPceAppParams appParams;
    std::vector<uint8_t> val(16, 0xEF);
    appParams.SetDatabaseIdentifier(val);
    EXPECT_EQ(appParams.GetDatabaseIdentifier(), val);
    HILOGI("PbapPce_AppParam_DatabaseIdentifier_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_AddToObexHeader_00100
 * @tc.name: AddToObexHeader and FromObexHeader round-trip
 * @tc.desc: Verify that parameters set and serialized can be deserialized back
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_AddToObexHeader_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_AddToObexHeader_00100 start");
    PbapPceAppParams appParams;
    appParams.SetOrder(0x01);
    appParams.SetSearchProperty(0x01);
    appParams.SetMaxListCount(100);
    appParams.SetListStartOffset(50);
    appParams.SetPropertySelector(0x10000081ULL);
    appParams.SetFormat(0x00);
    appParams.SetVcardSelector(0x82ULL);
    appParams.SetVcardSelectorOperator(0x01);
    appParams.SetPbapSupportedFeatures(0x00000003);

    ObexHeader hdr;
    appParams.AddToObexHeader(hdr);

    PbapPceAppParams result = PbapPceAppParams::FromObexHeader(hdr);
    EXPECT_TRUE(result.HasOrder());
    EXPECT_EQ(result.GetOrder(), 0x01);
    EXPECT_TRUE(result.HasSearchProperty());
    EXPECT_EQ(result.GetSearchProperty(), 0x01);
    EXPECT_TRUE(result.HasMaxListCount());
    EXPECT_EQ(result.GetMaxListCount(), 100);
    EXPECT_TRUE(result.HasListStartOffset());
    EXPECT_EQ(result.GetListStartOffset(), 50);
    EXPECT_TRUE(result.HasPropertySelector());
    EXPECT_EQ(result.GetPropertySelector(), 0x10000081ULL);
    EXPECT_TRUE(result.HasFormat());
    EXPECT_EQ(result.GetFormat(), 0x00);
    EXPECT_TRUE(result.HasVcardSelector());
    EXPECT_EQ(result.GetVcardSelector(), 0x82ULL);
    EXPECT_TRUE(result.HasVcardSelectorOperator());
    EXPECT_EQ(result.GetVcardSelectorOperator(), 0x01);
    EXPECT_TRUE(result.HasPbapSupportedFeatures());
    EXPECT_EQ(result.GetPbapSupportedFeatures(), 0x00000003);
    HILOGI("PbapPce_AppParam_AddToObexHeader_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_AddToObexHeader_00200
 * @tc.name: AddToObexHeader with search value
 * @tc.desc: Verify that search value is serialized and deserialized correctly
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_AddToObexHeader_00200, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_AddToObexHeader_00200 start");
    PbapPceAppParams appParams;
    std::vector<uint8_t> searchVal = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x38, 0x39};
    appParams.SetSearchValueUtf8(searchVal);

    ObexHeader hdr;
    appParams.AddToObexHeader(hdr);

    PbapPceAppParams result = PbapPceAppParams::FromObexHeader(hdr);
    EXPECT_EQ(result.GetSearchValueUtf8(), searchVal);
    HILOGI("PbapPce_AppParam_AddToObexHeader_00200 end");
}

/*
 * @tc.number: PbapPce_AppParam_FromObexHeader_00100
 * @tc.name: FromObexHeader with empty header
 * @tc.desc: FromObexHeader with no app params returns default object
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_FromObexHeader_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_FromObexHeader_00100 start");
    ObexHeader hdr;
    PbapPceAppParams result = PbapPceAppParams::FromObexHeader(hdr);
    EXPECT_FALSE(result.HasOrder());
    EXPECT_FALSE(result.HasMaxListCount());
    EXPECT_FALSE(result.HasFormat());
    HILOGI("PbapPce_AppParam_FromObexHeader_00100 end");
}

/*
 * @tc.number: PbapPce_AppParam_DefaultValues_00100
 * @tc.name: Default values
 * @tc.desc: Verify default values of PbapPceAppParams
 */
HWTEST_F(PbapPceAppParamTest, PbapPce_AppParam_DefaultValues_00100, TestSize.Level1)
{
    HILOGI("PbapPce_AppParam_DefaultValues_00100 start");
    PbapPceAppParams appParams;
    EXPECT_EQ(appParams.GetOrder(), 0xFF);
    EXPECT_EQ(appParams.GetSearchProperty(), 0xFF);
    EXPECT_EQ(appParams.GetMaxListCount(), 0xFFFF);
    EXPECT_EQ(appParams.GetListStartOffset(), 0);
    EXPECT_EQ(appParams.GetPropertySelector(), 0ULL);
    EXPECT_EQ(appParams.GetFormat(), 0xFF);
    EXPECT_EQ(appParams.GetPhonebookSize(), 0);
    EXPECT_EQ(appParams.GetNewMissedCalls(), 0xFF);
    EXPECT_EQ(appParams.GetVcardSelector(), 0ULL);
    EXPECT_EQ(appParams.GetVcardSelectorOperator(), 0xFF);
    EXPECT_EQ(appParams.GetResetNewMissedCalls(), 0xFF);
    EXPECT_EQ(appParams.GetPbapSupportedFeatures(), 0);
    EXPECT_TRUE(appParams.GetName().empty());
    EXPECT_TRUE(appParams.GetSearchValueUtf8().empty());
    EXPECT_TRUE(appParams.GetPrimaryFolderVersion().empty());
    EXPECT_TRUE(appParams.GetSecondaryFolderVersion().empty());
    EXPECT_TRUE(appParams.GetDatabaseIdentifier().empty());
    HILOGI("PbapPce_AppParam_DefaultValues_00100 end");
}
}  // namespace bluetooth
}  // namespace OHOS
