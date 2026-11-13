/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include <string>
#include <vector>
#include "bluetooth_log.h"
#include "pbap_pce_vcard_parser.h"
#include "pbap_pce_obex_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

static std::vector<uint8_t> ToBytes(const std::string &s)
{
    return std::vector<uint8_t>(s.begin(), s.end());
}

class PbapPceVCardParserTest : public testing::Test {
public:
    PbapPceVCardParserTest() {}
    ~PbapPceVCardParserTest() {}

    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/*
 * @tc.number: PbapPce_Parser_KeepAll_00100
 * @tc.name: keep-all: nameless-with-phone card kept
 * @tc.desc: 无FN/N仅有TEL的联系人（末批20张无名卡回归场景）必须保留，
 *           保证解析数==传输条数、offset与PSE位置空间对齐
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_KeepAll_00100, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:19834406023\r\nEND:VCARD\r\n"));
    EXPECT_EQ(contacts.size(), static_cast<size_t>(1));
    ASSERT_EQ(contacts[0].phones.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].phones[0], "19834406023");
    EXPECT_TRUE(contacts[0].displayName.empty());
}

/*
 * @tc.number: PbapPce_Parser_KeepAll_00110
 * @tc.name: keep-all: empty card kept
 * @tc.desc: 规格允许的空名片（无任何属性）也保留，计入解析数（offset 推进）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_KeepAll_00110, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nEND:VCARD\r\n"
        "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Bob\r\nTEL:123\r\nEND:VCARD\r\n"));
    EXPECT_EQ(contacts.size(), static_cast<size_t>(2));
    EXPECT_TRUE(contacts[0].displayName.empty());
    EXPECT_TRUE(contacts[0].phones.empty());
    EXPECT_EQ(contacts[1].displayName, "Bob");
}

/*
 * @tc.number: PbapPce_Parser_KeepAll_00120
 * @tc.name: keep-all: email-only card kept
 * @tc.desc: 仅邮件卡保留（过滤职责在入库侧，解析层不丢弃）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_KeepAll_00120, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nEMAIL:a@b.c\r\nEND:VCARD\r\n"));
    EXPECT_EQ(contacts.size(), static_cast<size_t>(1));
    ASSERT_EQ(contacts[0].emails.size(), static_cast<size_t>(1));
}

/*
 * @tc.number: PbapPce_Parser_Truncated_00200
 * @tc.name: truncated tail card dropped, complete cards kept
 * @tc.desc: 流截断（缺END:VCARD）的末尾卡片丢弃，前面完整卡片保留
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Truncated_00200, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nFN:A\r\nEND:VCARD\r\n"
        "BEGIN:VCARD\r\nFN:B\r\n"));
    EXPECT_EQ(contacts.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].displayName, "A");
}

/*
 * @tc.number: PbapPce_Parser_Malformed_00210
 * @tc.name: END without BEGIN ignored, garbage outside cards ignored
 * @tc.desc: 无BEGIN的END:VCARD与卡片外垃圾行均忽略，不产生空卡片
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Malformed_00210, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "garbage line\r\nEND:VCARD\r\nBEGIN:VCARD\r\nFN:C\r\nEND:VCARD\r\njunk\r\n"));
    EXPECT_EQ(contacts.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].displayName, "C");
}

/*
 * @tc.number: PbapPce_Parser_Scenario_00300
 * @tc.name: CALL_LOGS scenario also keep-all
 * @tc.desc: keep-all 与场景无关：通话记录场景（TEL+X-IRMC-CALL-DATETIME 无姓名）保留
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Scenario_00300, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:100\r\n"
        "X-IRMC-CALL-DATETIME:20260911T114500\r\nEND:VCARD\r\n"),
        PbapPceParseScenario::CALL_LOGS);
    EXPECT_EQ(contacts.size(), static_cast<size_t>(1));
    ASSERT_EQ(contacts[0].unknownXData.size(), static_cast<size_t>(1));
}

/*
 * @tc.number: PbapPce_Parser_OwnerN_00310
 * @tc.name: N property fallback display name
 * @tc.desc: N:;A;;; 名片形态经 N 回退生成 displayName "A"（given名）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_OwnerN_00310, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nN:;A;;;\r\nTEL:19834406023\r\nEND:VCARD\r\n"));
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].displayName, "A");
    EXPECT_EQ(contacts[0].givenName, "A");
    EXPECT_TRUE(contacts[0].familyName.empty());
}

/*
 * @tc.number: PbapPce_Parser_CallParam_00320
 * @tc.name: X-IRMC-CALL-DATETIME with MISSED param preserved
 * @tc.desc: "X-IRMC-CALL-DATETIME;MISSED:..." 带参形式经解析保留完整属性名
 *           （含参数）与时间戳值——ICH 入库侧 MISSED 过滤的数据基础
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_CallParam_00320, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000001\r\n"
        "X-IRMC-CALL-DATETIME;MISSED:20260911T114500\r\nEND:VCARD\r\n"),
        PbapPceParseScenario::CALL_LOGS);
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    ASSERT_EQ(contacts[0].unknownXData.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].unknownXData[0].first, "X-IRMC-CALL-DATETIME;MISSED");
    EXPECT_EQ(contacts[0].unknownXData[0].second, "20260911T114500");
}

/*
 * @tc.number: PbapPce_Parser_CallParam_00321
 * @tc.name: X-IRMC-CALL-DATETIME with RECEIVED/DIALED params preserved
 * @tc.desc: ";RECEIVED"、";DIALED" 及 ";TYPE=MISSED" 参数形式均保留完整属性名，
 *           过滤判定不依赖特定参数写法
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_CallParam_00321, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000001\r\n"
        "X-IRMC-CALL-DATETIME;RECEIVED:20260911T114500\r\nEND:VCARD\r\n"
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000002\r\n"
        "X-IRMC-CALL-DATETIME;DIALED:20260911T120000\r\nEND:VCARD\r\n"
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000003\r\n"
        "X-IRMC-CALL-DATETIME;TYPE=MISSED:20260911T123000\r\nEND:VCARD\r\n"),
        PbapPceParseScenario::CALL_LOGS);
    ASSERT_EQ(contacts.size(), static_cast<size_t>(3));
    EXPECT_EQ(contacts[0].unknownXData[0].first, "X-IRMC-CALL-DATETIME;RECEIVED");
    EXPECT_EQ(contacts[1].unknownXData[0].first, "X-IRMC-CALL-DATETIME;DIALED");
    EXPECT_EQ(contacts[2].unknownXData[0].first, "X-IRMC-CALL-DATETIME;TYPE=MISSED");
}

/*
 * @tc.number: PbapPce_Parser_EmptyInput_00400
 * @tc.name: empty input returns empty list
 * @tc.desc: 空输入返回空列表
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_EmptyInput_00400, TestSize.Level1)
{
    std::vector<uint8_t> empty;
    EXPECT_EQ(PbapPceVCardParser::Parse(empty).size(), static_cast<size_t>(0));
}

/*
 * @tc.number: PbapPce_Parser_Escape_00500
 * @tc.name: FN TEXT value unescape (bug repro)
 * @tc.desc: 手机侧姓名含 ',' 时 PSE 按 RFC 2426 转义为 "\," 下发，
 *           解析侧必须反转义，不得残留反斜杠（线上问题：Aa全@,/'x😂(^.^)
 *           同步后变成 Aa全@\,/'x😂(^.^)）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Escape_00500, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\n"
        "FN:Aa\xe5\x85\xa8@\\,/'x\xf0\x9f\x98\x82(^.^)\r\n"
        "TEL:123\r\nEND:VCARD\r\n"));
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].displayName, "Aa\xe5\x85\xa8@,/'x\xf0\x9f\x98\x82(^.^)");
}

/*
 * @tc.number: PbapPce_Parser_Escape_00510
 * @tc.name: FN full escape set unescape
 * @tc.desc: TEXT 转义全集：\\→'\'、\;→';'、\n→LF、\N→LF；
 *           未知转义 \x 按字面保留（防御不规范 PSE）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Escape_00510, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\n"
        "FN:a\\\\b\\;c\\nd\\Ne\\q\r\nEND:VCARD\r\n"));
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    // \n 与 \N 均反转义为 LF：a\b;c LF d LF e\q
    EXPECT_EQ(contacts[0].displayName, "a\\b;c\nd\ne\\q");
}

/*
 * @tc.number: PbapPce_Parser_Escape_00520
 * @tc.name: N escaped semicolon keeps column structure
 * @tc.desc: N 值中 "\;" 为字面分号不得拆列：N:张\;三;;; 的 familyName 为
 *           "张;三"，givenName 为空（修复前被错拆为 family="张\" given="三"，
 *           姓/名列错位 + 反斜杠残留）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Escape_00520, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\n"
        "N:\xe5\xbc\xa0\\;\xe4\xb8\x89;;;\r\nEND:VCARD\r\n"));
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].familyName, "\xe5\xbc\xa0;\xe4\xb8\x89");
    EXPECT_TRUE(contacts[0].givenName.empty());
    // displayName 经 N 回退拼接，同样不含转义符
    EXPECT_EQ(contacts[0].displayName, "\xe5\xbc\xa0;\xe4\xb8\x89");
}

/*
 * @tc.number: PbapPce_Parser_Escape_00530
 * @tc.name: N normal structure regression after escape-aware split
 * @tc.desc: 普通无转义 N 解析回归：family/given/middle 分列与 displayName
 *           "given family" 拼接行为不变
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Escape_00530, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\n"
        "N:Doe;John;Middle\r\nEND:VCARD\r\n"));
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].familyName, "Doe");
    EXPECT_EQ(contacts[0].givenName, "John");
    EXPECT_EQ(contacts[0].middleName, "Middle");
    EXPECT_EQ(contacts[0].displayName, "John Doe");
}

/*
 * @tc.number: PbapPce_Parser_Escape_00540
 * @tc.name: EMAIL TEXT value unescape; TEL kept verbatim
 * @tc.desc: EMAIL 为 TEXT 值反转义；TEL 非 TEXT 类型保持原样（不反转义，
 *           避免 PSE 未转义号码被误改）
 */
HWTEST_F(PbapPceVCardParserTest, PbapPce_Parser_Escape_00540, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\n"
        "EMAIL:a\\,b@c.d\r\nTEL:123\r\nEND:VCARD\r\n"));
    ASSERT_EQ(contacts.size(), static_cast<size_t>(1));
    ASSERT_EQ(contacts[0].emails.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].emails[0], "a,b@c.d");
    ASSERT_EQ(contacts[0].phones.size(), static_cast<size_t>(1));
    EXPECT_EQ(contacts[0].phones[0], "123");
}

class PbapPceInsertFilterTest : public testing::Test {
public:
    PbapPceInsertFilterTest() {}
    ~PbapPceInsertFilterTest() {}

    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

static PbapPceVCardContact MakeContact(const std::string &name, const std::string &phone)
{
    PbapPceVCardContact c;
    c.displayName = name;
    if (!phone.empty()) {
        c.phones.push_back(phone);
    }
    return c;
}

/*
 * @tc.number: PbapPce_Filter_FirstBatch_00120
 * @tc.name: first batch at ListStartOffset=1 all kept
 * @tc.desc: 请求级名片跳过（PBAP v1.2.3 §3.1.5）：首批从 ListStartOffset=1 起拉，
 *           首张为普通联系人，不做任何位置规则跳过（首批首张错杀回归）
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_FirstBatch_00120, TestSize.Level1)
{
    std::vector<PbapPceVCardContact> contacts = {
        MakeContact("Dahl0", "1"),  // position 1, ordinary contact (first of first batch)
        MakeContact("Eve", "012"),
    };
    size_t emptyDropped = 0;
    auto insertSet = PbapPceObexClient::FilterContactsForInsert(contacts, emptyDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(2));
    EXPECT_EQ(emptyDropped, static_cast<size_t>(0));
    EXPECT_EQ(insertSet[0].displayName, "Dahl0");
    EXPECT_EQ(insertSet[1].displayName, "Eve");
}

/*
 * @tc.number: PbapPce_Filter_Empty_00200
 * @tc.name: empty cards dropped from insert set
 * @tc.desc: 无电话且无姓名的空卡不入库（防服务端空 bucket）；
 *           无姓名有电话的联系人保留（入库时不设 name 由 helper 处理）
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_Empty_00200, TestSize.Level1)
{
    std::vector<PbapPceVCardContact> contacts = {
        MakeContact("", ""),           // empty card -> dropped
        MakeContact("", "19834406023"), // nameless-with-phone -> kept (20-card scenario)
        MakeContact("OnlyName", ""),    // name-only -> kept
        MakeContact("", ""),           // empty card -> dropped
    };
    size_t emptyDropped = 0;
    auto insertSet = PbapPceObexClient::FilterContactsForInsert(contacts, emptyDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(2));
    EXPECT_EQ(emptyDropped, static_cast<size_t>(2));
    ASSERT_EQ(insertSet[0].phones.size(), static_cast<size_t>(1));
    EXPECT_TRUE(insertSet[0].displayName.empty());
    EXPECT_EQ(insertSet[1].displayName, "OnlyName");
}

/*
 * @tc.number: PbapPce_Filter_RealBatch_00300
 * @tc.name: realistic first batch filtering
 * @tc.desc: 首批实测场景（请求从位置 1 起拉）：普通联系人 + 空卡(丢弃) +
 *           无名有电话(保留)，全部正常联系人入库
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_RealBatch_00300, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Dahl0\r\nTEL:1\r\nEND:VCARD\r\n"    // first ordinary contact
        "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Bob\r\nTEL:123\r\nEND:VCARD\r\n"   // normal
        "BEGIN:VCARD\r\nVERSION:3.0\r\nEND:VCARD\r\n"                        // empty card
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:999\r\nEND:VCARD\r\n"));          // nameless
    ASSERT_EQ(contacts.size(), static_cast<size_t>(4));
    size_t emptyDropped = 0;
    auto insertSet = PbapPceObexClient::FilterContactsForInsert(contacts, emptyDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(3));
    EXPECT_EQ(emptyDropped, static_cast<size_t>(1));
    EXPECT_EQ(insertSet[0].displayName, "Dahl0");
    EXPECT_EQ(insertSet[1].displayName, "Bob");
    EXPECT_TRUE(insertSet[2].displayName.empty());
    ASSERT_EQ(insertSet[2].phones.size(), static_cast<size_t>(1));
    EXPECT_EQ(insertSet[2].phones[0], "999");
}

/*
 * @tc.number: PbapPce_Window_00400
 * @tc.name: empty phonebook has no contact batch
 * @tc.desc: totalSize=0（空簿）：无联系人批次，直接进入通话记录阶段
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Window_00400, TestSize.Level1)
{
    uint32_t firstOffset = 0;
    EXPECT_FALSE(PbapPceObexClient::CalcFirstContactBatch(0, firstOffset));
}

/*
 * @tc.number: PbapPce_Window_00410
 * @tc.name: owner-card-only phonebook has no contact batch
 * @tc.desc: totalSize=1（PhonebookSize 仅含位置 0 名片槽位）：无真实联系人可拉，
 *           直接进入通话记录阶段
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Window_00410, TestSize.Level1)
{
    uint32_t firstOffset = 0;
    EXPECT_FALSE(PbapPceObexClient::CalcFirstContactBatch(1, firstOffset));
}

/*
 * @tc.number: PbapPce_Window_00420
 * @tc.name: first batch starts at position 1
 * @tc.desc: totalSize=9（实测场景）：首批 ListStartOffset=1，首批 count 由下载
 *           循环按窗口计算为 totalSize-1=8，名片槽位不下发
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Window_00420, TestSize.Level1)
{
    uint32_t firstOffset = 0;
    EXPECT_TRUE(PbapPceObexClient::CalcFirstContactBatch(9, firstOffset));
    EXPECT_EQ(firstOffset, static_cast<uint32_t>(1));
}

/*
 * @tc.number: PbapPce_Window_00430
 * @tc.name: large phonebook still starts at position 1
 * @tc.desc: totalSize=600（需分批拉取）：首批起点仍为位置 1，后续批次由下载
 *           循环按解析数推进
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Window_00430, TestSize.Level1)
{
    uint32_t firstOffset = 0;
    EXPECT_TRUE(PbapPceObexClient::CalcFirstContactBatch(600, firstOffset));
    EXPECT_EQ(firstOffset, static_cast<uint32_t>(1));
}

static PbapPceVCardContact MakeCallLog(const std::string &phone, const std::string &callParam)
{
    PbapPceVCardContact c;
    if (!phone.empty()) {
        c.phones.push_back(phone);
    }
    if (!callParam.empty()) {
        // callParam 形如 ""、";MISSED"、";TYPE=MISSED"、";missed"，模拟解析层在
        // unknownXData 中保留的完整属性名（keep-all，含参数）
        c.unknownXData.push_back(std::make_pair("X-IRMC-CALL-DATETIME" + callParam, "20260911T114500"));
    }
    return c;
}

/*
 * @tc.number: PbapPce_Filter_Calllog_00100
 * @tc.name: ICH mixed entries: MISSED dropped, others kept
 * @tc.desc: 部分手机 ICH 响应混入 MISSED 条目：带 ;MISSED 参数的条目丢弃
 *           （后续 MCH 拉取补齐）；;RECEIVED 与无参数条目按路径默认保留
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_Calllog_00100, TestSize.Level1)
{
    std::vector<PbapPceVCardContact> contacts = {
        MakeCallLog("13800000001", ";RECEIVED"),
        MakeCallLog("13800000002", ";MISSED"),
        MakeCallLog("13800000003", ""),  // 无参数：按路径默认（已接来电）保留
    };
    size_t missedDropped = 0;
    auto insertSet = PbapPceObexClient::FilterCallLogsForInsert(contacts, PATH_ICH, missedDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(2));
    EXPECT_EQ(missedDropped, static_cast<size_t>(1));
    EXPECT_EQ(insertSet[0].phones[0], "13800000001");
    EXPECT_EQ(insertSet[1].phones[0], "13800000003");
}

/*
 * @tc.number: PbapPce_Filter_Calllog_00110
 * @tc.name: MISSED param form variants all recognized
 * @tc.desc: ";TYPE=MISSED"（vCard 3.0 参数形式）与小写 ";missed"（部分 PSE 实现）
 *           均识别为未接条目丢弃，大小写与写法不敏感
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_Calllog_00110, TestSize.Level1)
{
    std::vector<PbapPceVCardContact> contacts = {
        MakeCallLog("13800000001", ";TYPE=MISSED"),
        MakeCallLog("13800000002", ";missed"),
        MakeCallLog("13800000003", ";RECEIVED"),
    };
    size_t missedDropped = 0;
    auto insertSet = PbapPceObexClient::FilterCallLogsForInsert(contacts, PATH_ICH, missedDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(1));
    EXPECT_EQ(missedDropped, static_cast<size_t>(2));
    EXPECT_EQ(insertSet[0].phones[0], "13800000003");
}

/*
 * @tc.number: PbapPce_Filter_Calllog_00120
 * @tc.name: MCH/OCH paths not filtered
 * @tc.desc: MCH 的 MISSED 条目是其正主数据不得过滤；OCH 同样不过滤
 *           （过滤仅作用于 ICH 路径，避免误删）
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_Calllog_00120, TestSize.Level1)
{
    std::vector<PbapPceVCardContact> contacts = {
        MakeCallLog("13800000001", ";MISSED"),
        MakeCallLog("13800000002", ";DIALED"),
    };
    size_t missedDropped = 0;
    auto mchSet = PbapPceObexClient::FilterCallLogsForInsert(contacts, PATH_MCH, missedDropped);
    EXPECT_EQ(mchSet.size(), static_cast<size_t>(2));
    EXPECT_EQ(missedDropped, static_cast<size_t>(0));

    auto ochSet = PbapPceObexClient::FilterCallLogsForInsert(contacts, PATH_OCH, missedDropped);
    EXPECT_EQ(ochSet.size(), static_cast<size_t>(2));
    EXPECT_EQ(missedDropped, static_cast<size_t>(0));
}

/*
 * @tc.number: PbapPce_Filter_Calllog_00130
 * @tc.name: ICH all-missed yields empty insert set
 * @tc.desc: ICH 全部条目为 MISSED 时过滤后为空集，走既有 empty skip 路径不触发
 *           入库；丢弃计数与输入条数一致
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_Calllog_00130, TestSize.Level1)
{
    std::vector<PbapPceVCardContact> contacts = {
        MakeCallLog("13800000001", ";MISSED"),
        MakeCallLog("13800000002", ";MISSED"),
    };
    size_t missedDropped = 0;
    auto insertSet = PbapPceObexClient::FilterCallLogsForInsert(contacts, PATH_ICH, missedDropped);
    EXPECT_TRUE(insertSet.empty());
    EXPECT_EQ(missedDropped, static_cast<size_t>(2));
}

/*
 * @tc.number: PbapPce_Filter_Calllog_00140
 * @tc.name: parse-to-filter chain on realistic ICH stream
 * @tc.desc: 端到端链路：解析真实 ICH vCard 流（RECEIVED/MISSED 混合）后过滤，
 *           仅 RECEIVED 条目进入入库集
 */
HWTEST_F(PbapPceInsertFilterTest, PbapPce_Filter_Calllog_00140, TestSize.Level1)
{
    auto contacts = PbapPceVCardParser::Parse(ToBytes(
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000001\r\n"
        "X-IRMC-CALL-DATETIME;RECEIVED:20260911T114500\r\nEND:VCARD\r\n"
        "BEGIN:VCARD\r\nVERSION:3.0\r\nTEL:13800000002\r\n"
        "X-IRMC-CALL-DATETIME;MISSED:20260911T120000\r\nEND:VCARD\r\n"),
        PbapPceParseScenario::CALL_LOGS);
    ASSERT_EQ(contacts.size(), static_cast<size_t>(2));
    size_t missedDropped = 0;
    auto insertSet = PbapPceObexClient::FilterCallLogsForInsert(contacts, PATH_ICH, missedDropped);
    EXPECT_EQ(insertSet.size(), static_cast<size_t>(1));
    EXPECT_EQ(missedDropped, static_cast<size_t>(1));
    ASSERT_EQ(insertSet[0].unknownXData.size(), static_cast<size_t>(1));
    EXPECT_EQ(insertSet[0].unknownXData[0].first, "X-IRMC-CALL-DATETIME;RECEIVED");
}

}  // namespace bluetooth
}  // namespace OHOS
