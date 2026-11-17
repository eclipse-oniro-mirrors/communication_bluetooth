/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
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
#include <gmock/gmock.h>
#include <chrono>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include "bluetooth_log.h"
#include "obex_def.h"
#include "obex_header.h"
#include "watch_define.h"
#include "watch_pbap_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {
namespace {
const std::string TEST_ADDRESS = "11:22:33:44:55:66";
const std::string TEST_NUMBER = "13800138000";
const int TEST_CONNECTION_ID = 7;

std::vector<uint8_t> StrToVector(const std::string &str)
{
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string VectorToStr(const std::vector<uint8_t> &vec)
{
    return std::string(vec.begin(), vec.end());
}

// 与 SetNameHeaderU16 期望对齐：ASCII → UTF-16BE（大端） + 0x0000 终止符
std::vector<uint8_t> MakeUtf16Be(const std::string &str)
{
    std::vector<uint8_t> out;
    for (char c : str) {
        out.push_back(0x00);
        out.push_back(static_cast<uint8_t>(c));
    }
    out.push_back(0x00);
    out.push_back(0x00);
    return out;
}

std::vector<WatchPbapClient::VcardListingCard> ParseListingXml(const std::string &xml)
{
    return WatchPbapClient::ParseVcardListing(StrToVector(xml));
}

WatchPbapClient::VcardListingCard MakeCard(const std::string &handle, const std::string &name)
{
    WatchPbapClient::VcardListingCard card;
    card.handle = handle;
    card.name = name;
    return card;
}

// 向 fd 循环写入直至完成（模拟 PSE 侧发送）
bool WriteAll(int fd, const std::vector<uint8_t> &data)
{
    size_t written = 0;
    while (written < data.size()) {
        ssize_t ret = write(fd, data.data() + written, data.size() - written);
        if (ret <= 0) {
            return false;
        }
        written += static_cast<size_t>(ret);
    }
    return true;
}

// 构造 OBEX 响应帧：[响应码][总长度 BE16][头域]
std::vector<uint8_t> MakeObexResponse(uint8_t responseCode, const std::vector<uint8_t> &headers)
{
    std::vector<uint8_t> frame;
    size_t totalLen = HEADER_PREFIX_LENGTH + headers.size();
    frame.push_back(responseCode);
    frame.push_back((totalLen >> 8) & 0xFF);
    frame.push_back(totalLen & 0xFF);
    frame.insert(frame.end(), headers.begin(), headers.end());
    return frame;
}

// 构造 BODY/END_OF_BODY 头：[头标识][头长度 BE16][负载]
std::vector<uint8_t> MakeBodyHeader(uint8_t headerId, size_t payloadLen, uint8_t fill)
{
    std::vector<uint8_t> header;
    size_t headerLen = HEADER_PREFIX_LENGTH + payloadLen;
    header.push_back(headerId);
    header.push_back((headerLen >> 8) & 0xFF);
    header.push_back(headerLen & 0xFF);
    header.insert(header.end(), payloadLen, fill);
    return header;
}
}  // namespace

// 覆写广播发送以捕获事件载荷，避免单测环境依赖公共事件服务
class TestWatchPbapClient : public WatchPbapClient {
public:
    explicit TestWatchPbapClient(const std::string &address) : WatchPbapClient(address) {}

    void SendSecondPhoneCallNameEvent(const std::string &name, const std::string &number) override
    {
        eventCount_++;
        lastName_ = name;
        lastNumber_ = number;
    }

    int eventCount_ = 0;
    std::string lastName_ {};
    std::string lastNumber_ {};
};

class WatchPbapClientTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        HILOGI("WatchPbapClientTest SetUpTestCase");
    }
    static void TearDownTestCase()
    {
        HILOGI("WatchPbapClientTest TearDownTestCase");
    }
    void SetUp()
    {
        client_ = std::make_shared<WatchPbapClient>(TEST_ADDRESS);
    }
    void TearDown()
    {
        if (client_ != nullptr) {
            client_->Disconnect();
            client_ = nullptr;
        }
    }

protected:
    std::shared_ptr<WatchPbapClient> client_ {nullptr};
};

/*
 * @tc.number: WatchPbapClient_Constructor_001
 * @tc.name: Constructor
 * @tc.desc: 构造后地址正确、初始状态为 IDLE 且未连接
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_Constructor_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_Constructor_001 start");
    ASSERT_NE(client_, nullptr);
    EXPECT_EQ(client_->GetAddress(), TEST_ADDRESS);
    EXPECT_FALSE(client_->IsConnected());
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_EQ(client_->socketFd_, -1);
    EXPECT_EQ(client_->connectionId_, -1);
    EXPECT_FALSE(client_->isObexConnected_);
}

/*
 * @tc.number: WatchPbapClient_PseServiceUuid_001
 * @tc.name: PseServiceUuid
 * @tc.desc: PBAP PSE 服务 UUID 常量与协议规定值一致
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_PseServiceUuid_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_PseServiceUuid_001 start");
    const uint8_t expectedUuid[WATCH_PBAP_PSE_SERVICE_UUID_LEN] = {
        0x79, 0x61, 0x35, 0xF0, 0xF0, 0xC5, 0x11, 0xD8,
        0x09, 0x66, 0x08, 0x00, 0x20, 0x0C, 0x9A, 0x66
    };
    EXPECT_EQ(WATCH_PBAP_PSE_SERVICE_UUID_LEN, 16);
    for (uint8_t i = 0; i < WATCH_PBAP_PSE_SERVICE_UUID_LEN; i++) {
        EXPECT_EQ(WATCH_PBAP_PSE_SERVICE_UUID[i], expectedUuid[i]);
    }
    EXPECT_EQ(WATCH_PBAP_PATH_PB, "telecom/pb");
    EXPECT_EQ(WATCH_PBAP_SEARCH_ATTR_NUMBER, 0x01);
    EXPECT_EQ(WATCH_PBAP_SEARCH_MAX_COUNT, 1);
    EXPECT_EQ(WATCH_PBAP_FEATURES_DOWNLOAD_BROWSING, 0x03);
    EXPECT_EQ(WATCH_PBAP_SOCKET_SEC_ENCRYPT_AUTHENTICATE, 3);
}

/*
 * @tc.number: WatchPbapClient_AppendTlv_001
 * @tc.name: AppendTlv
 * @tc.desc: TLV 追加格式为 tag + len + value
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_AppendTlv_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_AppendTlv_001 start");
    std::vector<uint8_t> out;
    const uint8_t value[] = {0x01, 0x02, 0x03};
    WatchPbapClient::AppendTlv(out, 0x02, value, sizeof(value));
    const std::vector<uint8_t> expected = {0x02, 0x03, 0x01, 0x02, 0x03};
    EXPECT_EQ(out, expected);

    // 追加第二个 TLV，互不覆盖
    const uint8_t value2[] = {0x01};
    WatchPbapClient::AppendTlv(out, 0x03, value2, sizeof(value2));
    EXPECT_EQ(out.size(), 7);
    EXPECT_EQ(out[5], 0x03);
    EXPECT_EQ(out[6], 0x01);
}

/*
 * @tc.number: WatchPbapClient_SetNameHeaderU16_001
 * @tc.name: SetNameHeaderU16
 * @tc.desc: NAME 头编码为 UTF-16BE 并以 0x0000 终止
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_SetNameHeaderU16_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_SetNameHeaderU16_001 start");
    ObexHeader header;
    WatchPbapClient::SetNameHeaderU16(header, "telecom/pb");
    std::vector<uint8_t> nameValue = header.GetHeader(NAME);
    std::vector<uint8_t> expected = MakeUtf16Be("telecom/pb");
    EXPECT_EQ(nameValue, expected);
    EXPECT_EQ(nameValue.size(), 22);  // 10 字符 * 2 + 2 终止符
}

/*
 * @tc.number: WatchPbapClient_SetNameHeaderU16_002
 * @tc.name: SetNameHeaderU16
 * @tc.desc: 含中文的 name 按码点解码为 UTF-16BE，多字节字符不按字节平铺
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_SetNameHeaderU16_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_SetNameHeaderU16_002 start");
    ObexHeader header;
    WatchPbapClient::SetNameHeaderU16(header, "张三");
    std::vector<uint8_t> nameValue = header.GetHeader(NAME);
    // 张=U+5F20，三=U+4E09，编码后追加 0x0000 终止符
    const std::vector<uint8_t> expected = {0x5F, 0x20, 0x4E, 0x09, 0x00, 0x00};
    EXPECT_EQ(nameValue, expected);
}

/*
 * @tc.number: WatchPbapClient_Utf8ToUtf16_001
 * @tc.name: Utf8ToUtf16
 * @tc.desc: ASCII 原样映射；BMP 中文按码点转换；BMP 外字符生成代理对
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_Utf8ToUtf16_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_Utf8ToUtf16_001 start");
    std::u16string ascii = WatchPbapClient::Utf8ToUtf16("telecom/pb");
    EXPECT_EQ(ascii, std::u16string(u"telecom/pb"));
    std::u16string bmp = WatchPbapClient::Utf8ToUtf16("张三");
    const char16_t expectedBmp[] = {0x5F20, 0x4E09};
    EXPECT_EQ(bmp, std::u16string(expectedBmp, 2));
    std::u16string astral = WatchPbapClient::Utf8ToUtf16("😀");  // U+1F600
    const char16_t expectedAstral[] = {0xD83D, 0xDE00};
    EXPECT_EQ(astral, std::u16string(expectedAstral, 2));
}

/*
 * @tc.number: WatchPbapClient_Utf8ToUtf16_002
 * @tc.name: Utf8ToUtf16
 * @tc.desc: 非法首字节/截断序列/过长编码以 U+FFFD 替换，不抛异常，合法部分保留
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_Utf8ToUtf16_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_Utf8ToUtf16_002 start");
    // 0xFF 为非法首字节
    std::string invalidLead(1, static_cast<char>(0xFF));
    std::u16string invalidLeadResult = WatchPbapClient::Utf8ToUtf16(invalidLead);
    EXPECT_EQ(invalidLeadResult.size(), static_cast<size_t>(1));
    EXPECT_EQ(invalidLeadResult[0], 0xFFFD);
    // 三字节中文序列在末尾截断
    std::u16string truncated = WatchPbapClient::Utf8ToUtf16("张".substr(0, 2));
    EXPECT_EQ(truncated.size(), static_cast<size_t>(1));
    EXPECT_EQ(truncated[0], 0xFFFD);
    // 过长编码 C0 80（NUL 的非法编码形式）：首字节与孤立续字节各替换一次
    const std::string overlongInput = {static_cast<char>(0xC0), static_cast<char>(0x80)};
    std::u16string overlong = WatchPbapClient::Utf8ToUtf16(overlongInput);
    EXPECT_EQ(overlong.size(), static_cast<size_t>(2));
    EXPECT_EQ(overlong[0], 0xFFFD);
    EXPECT_EQ(overlong[1], 0xFFFD);
    // 合法字符后跟非法字节：合法部分保留，非法字节替换
    std::string mixedInput = "a";
    mixedInput.push_back(static_cast<char>(0xFF));
    std::u16string mixed = WatchPbapClient::Utf8ToUtf16(mixedInput);
    EXPECT_EQ(mixed.size(), static_cast<size_t>(2));
    EXPECT_EQ(mixed[0], u'a');
    EXPECT_EQ(mixed[1], 0xFFFD);
}

/*
 * @tc.number: WatchPbapClient_BuildConnectHeader_001
 * @tc.name: BuildConnectHeader
 * @tc.desc: OBEX CONNECT 头携带 PSE TARGET、SUPPORTEDFEATURES TLV 与 ConnectionId 0
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_BuildConnectHeader_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_BuildConnectHeader_001 start");
    ObexHeader header;
    client_->BuildConnectHeader(header);

    std::vector<uint8_t> target = header.GetHeader(TARGET);
    ASSERT_EQ(target.size(), WATCH_PBAP_PSE_SERVICE_UUID_LEN);
    for (uint8_t i = 0; i < WATCH_PBAP_PSE_SERVICE_UUID_LEN; i++) {
        EXPECT_EQ(target[i], WATCH_PBAP_PSE_SERVICE_UUID[i]);
    }

    // 应用参数仅包含 SUPPORTEDFEATURES（0x10）TLV：BE32 值 0x03
    const std::vector<uint8_t> expectedAppParams = {0x10, 0x04, 0x00, 0x00, 0x00, 0x03};
    EXPECT_EQ(header.GetHeader(APPLICATION_PARAMETER), expectedAppParams);
    EXPECT_EQ(header.GetConnectionId(), 0);
}

/*
 * @tc.number: WatchPbapClient_BuildSearchByNumberRequest_001
 * @tc.name: BuildSearchByNumberRequest
 * @tc.desc: 按号码查询请求包含 TYPE/NAME/应用参数/ConnectionId/接收超时
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_BuildSearchByNumberRequest_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_BuildSearchByNumberRequest_001 start");
    client_->connectionId_ = TEST_CONNECTION_ID;
    ObexHeader req;
    client_->BuildSearchByNumberRequest(req, TEST_NUMBER);

    EXPECT_EQ(VectorToStr(req.GetHeader(TYPE)), "x-bt/vcard-listing");
    EXPECT_EQ(req.GetHeader(NAME), MakeUtf16Be(WATCH_PBAP_PATH_PB));
    EXPECT_EQ(req.GetConnectionId(), TEST_CONNECTION_ID);
    EXPECT_EQ(req.GetRecvTimeoutMs(), WATCH_PBAP_GET_TIMEOUT_MS);

    // 应用参数：SEARCH_VALUE + SEARCH_ATTRIBUTE=number + MAXLISTCOUNT=1
    const std::vector<uint8_t> expectedAppParams = {
        WATCH_PBAP_TAG_SEARCH_VALUE, static_cast<uint8_t>(TEST_NUMBER.size()),
        '1', '3', '8', '0', '0', '1', '3', '8', '0', '0', '0',
        WATCH_PBAP_TAG_SEARCH_PROPERTY, 0x01, WATCH_PBAP_SEARCH_ATTR_NUMBER,
        WATCH_PBAP_TAG_MAX_LIST_COUNT, 0x02, 0x00, 0x01,
    };
    EXPECT_EQ(req.GetHeader(APPLICATION_PARAMETER), expectedAppParams);
}

/*
 * @tc.number: WatchPbapClient_ParseVcardListing_001
 * @tc.name: ParseVcardListing
 * @tc.desc: 空 BODY 返回空结果
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ParseVcardListing_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ParseVcardListing_001 start");
    std::vector<uint8_t> emptyBody;
    EXPECT_TRUE(WatchPbapClient::ParseVcardListing(emptyBody).empty());
}

/*
 * @tc.number: WatchPbapClient_ParseVcardListing_002
 * @tc.name: ParseVcardListing
 * @tc.desc: 解析单个 card 元素的 handle 与 name 属性
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ParseVcardListing_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ParseVcardListing_002 start");
    const std::string xml =
        "<?xml version=\"1.0\"?><vCard-listing version=\"1.0\">"
        "<card handle=\"1.vcf\" name=\"ZhangSan\"/>"
        "</vCard-listing>";
    auto cards = ParseListingXml(xml);
    ASSERT_EQ(cards.size(), 1);
    EXPECT_EQ(cards[0].handle, "1.vcf");
    EXPECT_EQ(cards[0].name, "ZhangSan");
}

/*
 * @tc.number: WatchPbapClient_ParseVcardListing_003
 * @tc.name: ParseVcardListing
 * @tc.desc: 解析多个 card 元素且顺序保持
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ParseVcardListing_003, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ParseVcardListing_003 start");
    const std::string xml =
        "<vCard-listing>"
        "<card handle=\"1.vcf\" name=\"ZhangSan\"/>"
        "<card handle=\"2.vcf\" name=\"LiSi\"/>"
        "<card handle=\"3.vcf\" name=\"WangWu\"/>"
        "</vCard-listing>";
    auto cards = ParseListingXml(xml);
    ASSERT_EQ(cards.size(), 3);
    EXPECT_EQ(cards[0].name, "ZhangSan");
    EXPECT_EQ(cards[1].name, "LiSi");
    EXPECT_EQ(cards[2].name, "WangWu");
    EXPECT_EQ(cards[2].handle, "3.vcf");
}

/*
 * @tc.number: WatchPbapClient_ParseVcardListing_004
 * @tc.name: ParseVcardListing
 * @tc.desc: 属性值中的 XML 实体被正确反转义
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ParseVcardListing_004, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ParseVcardListing_004 start");
    const std::string xml =
        "<vCard-listing>"
        "<card handle=\"1.vcf\" name=\"A&amp;B&lt;C&gt;D&quot;E&apos;F\"/>"
        "</vCard-listing>";
    auto cards = ParseListingXml(xml);
    ASSERT_EQ(cards.size(), 1);
    EXPECT_EQ(cards[0].name, "A&B<C>D\"E'F");
}

/*
 * @tc.number: WatchPbapClient_ParseVcardListing_005
 * @tc.name: ParseVcardListing
 * @tc.desc: 仅含 handle 或仅含 name 的 card 均保留，两者皆空的 card 被过滤
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ParseVcardListing_005, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ParseVcardListing_005 start");
    const std::string xml =
        "<vCard-listing>"
        "<card handle=\"1.vcf\"/>"
        "<card name=\"OnlyName\"/>"
        "<card/>"
        "</vCard-listing>";
    auto cards = ParseListingXml(xml);
    ASSERT_EQ(cards.size(), 2);
    EXPECT_EQ(cards[0].handle, "1.vcf");
    EXPECT_TRUE(cards[0].name.empty());
    EXPECT_TRUE(cards[1].handle.empty());
    EXPECT_EQ(cards[1].name, "OnlyName");
}

/*
 * @tc.number: WatchPbapClient_ParseVcardListing_006
 * @tc.name: ParseVcardListing
 * @tc.desc: 无 card 元素（未命中查询的空 listing）返回空结果
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ParseVcardListing_006, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ParseVcardListing_006 start");
    const std::string xml = "<?xml version=\"1.0\"?><vCard-listing version=\"1.0\"/>";
    EXPECT_TRUE(ParseListingXml(xml).empty());

    // 截断的 XML（card 无闭合 '>'）：不崩溃，返回空
    const std::string truncated = "<vCard-listing><card handle=\"1.vcf\"";
    EXPECT_TRUE(ParseListingXml(truncated).empty());
}

/*
 * @tc.number: WatchPbapClient_OnConnectResp_001
 * @tc.name: OnConnectResp
 * @tc.desc: 响应成功且携带 ConnectionId 时记录连接 ID 并置位连接标志
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_OnConnectResp_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_OnConnectResp_001 start");
    ObexHeader reply;
    reply.SetConnectionId(TEST_CONNECTION_ID);
    client_->OnConnectResp(reply, OBEX_HTTP_OK);
    EXPECT_TRUE(client_->isObexConnected_);
    EXPECT_EQ(client_->connectionId_, TEST_CONNECTION_ID);
}

/*
 * @tc.number: WatchPbapClient_OnConnectResp_002
 * @tc.name: OnConnectResp
 * @tc.desc: 响应码非成功时连接标志保持 false
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_OnConnectResp_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_OnConnectResp_002 start");
    ObexHeader reply;
    reply.SetConnectionId(TEST_CONNECTION_ID);
    client_->OnConnectResp(reply, OBEX_HTTP_INTERNAL_ERROR);
    EXPECT_FALSE(client_->isObexConnected_);
    EXPECT_EQ(client_->connectionId_, -1);
}

/*
 * @tc.number: WatchPbapClient_OnConnectResp_003
 * @tc.name: OnConnectResp
 * @tc.desc: 响应成功但 ConnectionId 为 0 时不回填连接 ID
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_OnConnectResp_003, TestSize.Level1)
{
    HILOGI("WatchPbapClient_OnConnectResp_003 start");
    ObexHeader reply;
    client_->OnConnectResp(reply, OBEX_HTTP_OK);
    EXPECT_TRUE(client_->isObexConnected_);
    EXPECT_EQ(client_->connectionId_, -1);
}

/*
 * @tc.number: WatchPbapClient_OnDisConnectResp_001
 * @tc.name: OnDisConnectResp
 * @tc.desc: 断开响应后状态复位为 IDLE 并清理连接标志；传输层清理收敛在任务边界，回调不触碰 socketFd_
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_OnDisConnectResp_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_OnDisConnectResp_001 start");
    int fds[2] = {-1, -1};
    EXPECT_EQ(pipe(fds), 0);
    client_->isObexConnected_ = true;
    client_->connectionId_ = TEST_CONNECTION_ID;
    client_->state_.store(WatchPbapState::CONNECTED);
    client_->socketFd_ = fds[0];
    ObexHeader reply;
    client_->OnDisConnectResp(reply, OBEX_HTTP_OK);
    EXPECT_FALSE(client_->isObexConnected_);
    EXPECT_EQ(client_->connectionId_, -1);
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_EQ(client_->socketFd_, fds[0]);
    client_->socketFd_ = WATCH_PBAP_INVALID_SOCKET_FD;
    close(fds[0]);
    close(fds[1]);
}

/*
 * @tc.number: WatchPbapClient_OnAbortResp_001
 * @tc.name: OnAbortResp
 * @tc.desc: 中止响应后连接标志复位
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_OnAbortResp_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_OnAbortResp_001 start");
    client_->isObexConnected_ = true;
    client_->OnAbortResp();
    EXPECT_FALSE(client_->isObexConnected_);
}

/*
 * @tc.number: WatchPbapClient_OnAvailableResp_001
 * @tc.name: OnPutDateAvailableResp/OnGetDateAvailableResp
 * @tc.desc: 数据可用回调仅打印日志，不影响客户端状态
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_OnAvailableResp_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_OnAvailableResp_001 start");
    ObexHeader reply;
    client_->OnPutDateAvailableResp(reply, OBEX_HTTP_OK);
    client_->OnGetDateAvailableResp(reply, OBEX_HTTP_OK);
    EXPECT_FALSE(client_->isObexConnected_);
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
}

/*
 * @tc.number: WatchPbapClient_DoConnectTask_001
 * @tc.name: DoConnectTask
 * @tc.desc: 非 IDLE 状态重复发起连接任务时直接跳过
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_DoConnectTask_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_DoConnectTask_001 start");
    client_->state_.store(WatchPbapState::CONNECTING);
    client_->DoConnectTask();
    EXPECT_EQ(client_->state_.load(), WatchPbapState::CONNECTING);
}

/*
 * @tc.number: WatchPbapClient_DoConnectTask_002
 * @tc.name: DoConnectTask
 * @tc.desc: 单测环境下 SPP 服务不可用，连接失败后状态回退 IDLE
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_DoConnectTask_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_DoConnectTask_002 start");
    ASSERT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    // SocketService 未启动 → DoSocketConnect 失败 → 状态复位 IDLE
    client_->DoConnectTask();
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_FALSE(client_->IsConnected());
    EXPECT_EQ(client_->socketFd_, -1);
}

/*
 * @tc.number: WatchPbapClient_DoDisconnectTask_001
 * @tc.name: DoDisconnectTask
 * @tc.desc: 断开任务清理会话与状态并复位 IDLE
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_DoDisconnectTask_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_DoDisconnectTask_001 start");
    client_->state_.store(WatchPbapState::CONNECTED);
    client_->isObexConnected_ = true;
    client_->connectionId_ = TEST_CONNECTION_ID;
    client_->DoDisconnectTask();
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_FALSE(client_->isObexConnected_);
    EXPECT_EQ(client_->connectionId_, -1);
    EXPECT_EQ(client_->socketFd_, -1);
    EXPECT_EQ(client_->clientSession_, nullptr);
    EXPECT_EQ(client_->transport_, nullptr);
}

/*
 * @tc.number: WatchPbapClient_DoQueryTask_001
 * @tc.name: DoQueryTask
 * @tc.desc: 非连接状态下查询任务直接跳过，不崩溃
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_DoQueryTask_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_DoQueryTask_001 start");
    client_->DoQueryTask(TEST_NUMBER);
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_FALSE(client_->isObexConnected_);
}

/*
 * @tc.number: WatchPbapClient_DoQueryTask_002
 * @tc.name: DoQueryTask
 * @tc.desc: CONNECTED 但会话为空时查询任务跳过，状态保持
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_DoQueryTask_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_DoQueryTask_002 start");
    client_->state_.store(WatchPbapState::CONNECTED);
    client_->DoQueryTask(TEST_NUMBER);
    EXPECT_EQ(client_->state_.load(), WatchPbapState::CONNECTED);
}

/*
 * @tc.number: WatchPbapClient_Disconnect_001
 * @tc.name: Disconnect
 * @tc.desc: 工作线程未启动时断开在调用线程同步完成，不崩溃
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_Disconnect_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_Disconnect_001 start");
    client_->Disconnect();
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_EQ(client_->socketFd_, -1);
}

/*
 * @tc.number: WatchPbapClient_QueryContactByNumber_001
 * @tc.name: QueryContactByNumber
 * @tc.desc: 工作线程未运行时查询任务被丢弃，不崩溃
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_QueryContactByNumber_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_QueryContactByNumber_001 start");
    client_->QueryContactByNumber(TEST_NUMBER);
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
}

/*
 * @tc.number: WatchPbapClient_QueryContactByNumber_002
 * @tc.name: QueryContactByNumber
 * @tc.desc: 空串与超长（>255）号码在入口被拒不入队，正常号码正常入队
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_QueryContactByNumber_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_QueryContactByNumber_002 start");
    {
        // 模拟 worker 存活但无消费者：入队即滞留，入队行为可确定性观测
        std::lock_guard<std::mutex> lock(client_->taskMutex_);
        client_->running_ = true;
    }
    client_->QueryContactByNumber(TEST_NUMBER);
    EXPECT_EQ(client_->taskQueue_.size(), static_cast<size_t>(1));
    client_->QueryContactByNumber(std::string(WATCH_PBAP_MAX_SEARCH_VALUE_LEN + 1, '1'));
    EXPECT_EQ(client_->taskQueue_.size(), static_cast<size_t>(1));
    client_->QueryContactByNumber("");
    EXPECT_EQ(client_->taskQueue_.size(), static_cast<size_t>(1));
    {
        std::lock_guard<std::mutex> lock(client_->taskMutex_);
        client_->running_ = false;
        client_->taskQueue_.clear();
    }
}

/*
 * @tc.number: WatchPbapClient_ConnectDisconnectLifecycle_001
 * @tc.name: ConnectDisconnectLifecycle
 * @tc.desc: 异步连接失败后断开，工作线程正常退出且对象可安全析构
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ConnectDisconnectLifecycle_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ConnectDisconnectLifecycle_001 start");
    auto client = std::make_shared<WatchPbapClient>(TEST_ADDRESS);
    client->Connect();
    client->Disconnect();
    // 断开任务会终止工作线程；等待任务队列排空
    int waitCount = 0;
    while (waitCount < 100) {
        {
            std::lock_guard<std::mutex> lock(client->taskMutex_);
            if (!client->running_ && client->taskQueue_.empty()) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        waitCount++;
    }
    EXPECT_EQ(client->state_.load(), WatchPbapState::IDLE);
    EXPECT_FALSE(client->IsConnected());
    client = nullptr;  // 析构 join 工作线程，不应死锁
    SUCCEED();
}

/*
 * @tc.number: WatchPbapClient_PostTask_001
 * @tc.name: PostTask
 * @tc.desc: 工作线程未运行时任务投递失败并返回 false
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_PostTask_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_PostTask_001 start");
    bool executed = false;
    EXPECT_FALSE(client_->PostTask([&executed]() { executed = true; }));
    EXPECT_FALSE(executed);
}

/*
 * @tc.number: WatchPbapClient_SecondPhoneCallNameEvent_001
 * @tc.name: SecondPhoneCallNameEvent
 * @tc.desc: SECOND_PHONE_CALL_NAME 广播 event 与参数 key 常量定义正确
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_SecondPhoneCallNameEvent_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_SecondPhoneCallNameEvent_001 start");
    EXPECT_EQ(std::string(EVENT_BLUETOOTH_SECOND_PHONE_CALL_NAME),
        "usual.event.bluetooth.SECOND_PHONE_CALL_NAME");
    EXPECT_EQ(std::string(PARAM_KEY_CALL_NAME), "name");
    EXPECT_EQ(std::string(PARAM_KEY_CALL_NUMBER), "number");
}

/*
 * @tc.number: WatchPbapClient_HandleQueryResult_001
 * @tc.name: HandleQueryResult
 * @tc.desc: 查询未命中（空结果）时不发送广播
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_HandleQueryResult_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_HandleQueryResult_001 start");
    TestWatchPbapClient testClient(TEST_ADDRESS);
    testClient.HandleQueryResult(TEST_NUMBER, {});
    EXPECT_EQ(testClient.eventCount_, 0);
    EXPECT_TRUE(testClient.lastName_.empty());
    EXPECT_TRUE(testClient.lastNumber_.empty());
}

/*
 * @tc.number: WatchPbapClient_HandleQueryResult_002
 * @tc.name: HandleQueryResult
 * @tc.desc: 查询单条命中时发送一次广播，name 为联系人姓名、number 为来电号码
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_HandleQueryResult_002, TestSize.Level1)
{
    HILOGI("WatchPbapClient_HandleQueryResult_002 start");
    TestWatchPbapClient testClient(TEST_ADDRESS);
    std::vector<WatchPbapClient::VcardListingCard> cards = {MakeCard("1.vcf", "ZhangSan")};
    testClient.HandleQueryResult(TEST_NUMBER, cards);
    EXPECT_EQ(testClient.eventCount_, 1);
    EXPECT_EQ(testClient.lastName_, "ZhangSan");
    EXPECT_EQ(testClient.lastNumber_, TEST_NUMBER);
}

/*
 * @tc.number: WatchPbapClient_HandleQueryResult_003
 * @tc.name: HandleQueryResult
 * @tc.desc: 命中多条时仅以首条联系人的姓名发送一次广播
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_HandleQueryResult_003, TestSize.Level1)
{
    HILOGI("WatchPbapClient_HandleQueryResult_003 start");
    TestWatchPbapClient testClient(TEST_ADDRESS);
    std::vector<WatchPbapClient::VcardListingCard> cards = {
        MakeCard("1.vcf", "ZhangSan"),
        MakeCard("2.vcf", "LiSi"),
    };
    testClient.HandleQueryResult(TEST_NUMBER, cards);
    EXPECT_EQ(testClient.eventCount_, 1);
    EXPECT_EQ(testClient.lastName_, "ZhangSan");
    EXPECT_EQ(testClient.lastNumber_, TEST_NUMBER);
}

/*
 * @tc.number: WatchPbapClient_HandleQueryResult_004
 * @tc.name: HandleQueryResult
 * @tc.desc: 解析出的 listing XML 命中后经完整解析链路触发广播（实体反转义后的姓名）
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_HandleQueryResult_004, TestSize.Level1)
{
    HILOGI("WatchPbapClient_HandleQueryResult_004 start");
    TestWatchPbapClient testClient(TEST_ADDRESS);
    const std::string xml =
        "<?xml version=\"1.0\"?><vCard-listing version=\"1.0\">"
        "<card handle=\"1.vcf\" name=\"Zhang&amp;San\"/>"
        "</vCard-listing>";
    auto cards = ParseListingXml(xml);
    testClient.HandleQueryResult(TEST_NUMBER, cards);
    EXPECT_EQ(testClient.eventCount_, 1);
    EXPECT_EQ(testClient.lastName_, "Zhang&San");
    EXPECT_EQ(testClient.lastNumber_, TEST_NUMBER);
}

/*
 * @tc.number: WatchPbapClient_AppendUint16Be_001
 * @tc.name: AppendUint16Be
 * @tc.desc: 16 位值按大端字节序追加
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_AppendUint16Be_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_AppendUint16Be_001 start");
    std::vector<uint8_t> out;
    WatchPbapClient::AppendUint16Be(out, 0x1234);
    const std::vector<uint8_t> expected = {0x12, 0x34};
    EXPECT_EQ(out, expected);

    WatchPbapClient::AppendUint16Be(out, WATCH_PBAP_U16_NULL_TERMINATOR);
    const std::vector<uint8_t> expectedWithNull = {0x12, 0x34, 0x00, 0x00};
    EXPECT_EQ(out, expectedWithNull);
}

/*
 * @tc.number: WatchPbapClient_ListingBodyLimit_001
 * @tc.name: ListingBodyLimit
 * @tc.desc: vcard-listing BODY 收集上限为 64KB，防止对端违约超发导致无界累积
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_ListingBodyLimit_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_ListingBodyLimit_001 start");
    EXPECT_EQ(WATCH_PBAP_MAX_LISTING_BODY_SIZE, static_cast<size_t>(64 * 1024));
    // 合法响应为单卡片 XML（远小于 1KB），上限须留足余量
    EXPECT_GT(WATCH_PBAP_MAX_LISTING_BODY_SIZE, static_cast<size_t>(WATCH_PBAP_DEFAULT_MTU));
}

/*
 * @tc.number: WatchPbapClient_WorkerLoop_001
 * @tc.name: WorkerLoop
 * @tc.desc: running_ 置 false 后不再执行队列中的残留任务并清空队列（同步断开兜底依赖的不变量）
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_WorkerLoop_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_WorkerLoop_001 start");
    bool stopRequested = false;
    bool residualExecuted = false;
    {
        std::lock_guard<std::mutex> lock(client_->taskMutex_);
        client_->running_ = true;
        // 任务 1：置位停止标志，模拟 Disconnect 任务收尾
        client_->taskQueue_.push_back([this, &stopRequested]() {
            std::lock_guard<std::mutex> innerLock(client_->taskMutex_);
            client_->running_ = false;
            stopRequested = true;
        });
        // 任务 2：running_ 置 false 后队列中的残留任务，不应被执行
        client_->taskQueue_.push_back([&residualExecuted]() { residualExecuted = true; });
    }
    // 在测试线程直接驱动事件循环（与工作线程执行路径一致）
    client_->WorkerLoop();
    EXPECT_TRUE(stopRequested);
    EXPECT_FALSE(residualExecuted);
    EXPECT_FALSE(client_->running_);
    {
        std::lock_guard<std::mutex> lock(client_->taskMutex_);
        EXPECT_TRUE(client_->taskQueue_.empty());
    }
    HILOGI("WatchPbapClient_WorkerLoop_001 end");
}

/*
 * @tc.number: WatchPbapClient_SelfDestructOnWorker_001
 * @tc.name: SelfDestructOnWorker
 * @tc.desc: 最后一个引用随 WorkerLoop 的 self 在工作线程释放时，析构在工作线程内
 *           完成（pthread_detach 路径），不阻塞不崩溃
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_SelfDestructOnWorker_001, TestSize.Level1)
{
    HILOGI("WatchPbapClient_SelfDestructOnWorker_001 start");
    auto client = std::make_shared<WatchPbapClient>(TEST_ADDRESS);
    std::weak_ptr<WatchPbapClient> weakClient = client;
    client->Connect();
    // 等待连接任务排空（单测环境 SPP 不可用，连接失败回 IDLE）
    int waitCount = 0;
    while (waitCount < 100) {
        {
            std::lock_guard<std::mutex> lock(client->taskMutex_);
            if (client->state_.load() == WatchPbapState::IDLE && client->taskQueue_.empty()) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        waitCount++;
    }
    // 通知工作线程退出后释放外部引用：析构随 WorkerLoop 的 self 释放发生在
    // 工作线程内，须走 detach 路径完成自我回收
    {
        std::lock_guard<std::mutex> lock(client->taskMutex_);
        client->running_ = false;
    }
    client->taskCv_.notify_all();
    client = nullptr;
    bool destroyed = false;
    waitCount = 0;
    while (waitCount < 100) {
        if (weakClient.expired()) {
            destroyed = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        waitCount++;
    }
    EXPECT_TRUE(destroyed);
    HILOGI("WatchPbapClient_SelfDestructOnWorker_001 end");
}

/*
 * @tc.number: WatchPbapClient_DoQueryTask_003
 * @tc.name: DoQueryTask
 * @tc.desc: 对端违约超发 BODY（累计超过收集上限）时按传输级失败处理：拆除链路并
 *           复位状态，不解析溢出数据
 */
HWTEST_F(WatchPbapClientTest, WatchPbapClient_DoQueryTask_003, TestSize.Level1)
{
    HILOGI("WatchPbapClient_DoQueryTask_003 start");
    int fds[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    constexpr uint32_t TEST_MTU = 4096;
    constexpr size_t CHUNK_SIZE = 3000;
    // 注入基于 socketpair 的会话，模拟已建立连接
    auto device = std::make_shared<ObexSocketDevice>(fds[0], TEST_MTU, TEST_MTU,
        SocketType::TYPE_RFCOMM, TEST_ADDRESS);
    auto transport = std::make_shared<ObexServerTransport>(device);
    client_->transport_ = transport;
    client_->clientSession_ = std::make_shared<ObexClientSession>(transport, client_);
    client_->state_.store(WatchPbapState::CONNECTED);
    client_->isObexConnected_ = true;
    client_->connectionId_ = TEST_CONNECTION_ID;

    // 模拟恶意 PSE：无视 MAXLISTCOUNT=1，以 Continue + BODY 持续发送累计超过
    // WATCH_PBAP_MAX_LISTING_BODY_SIZE 的分片，最后以 OK + END_OF_BODY 收尾
    size_t sent = 0;
    while (sent <= WATCH_PBAP_MAX_LISTING_BODY_SIZE) {
        std::vector<uint8_t> frame = MakeObexResponse(OBEX_HTTP_CONTINUE,
            MakeBodyHeader(BODY, CHUNK_SIZE, 'A'));
        ASSERT_TRUE(WriteAll(fds[1], frame));
        sent += CHUNK_SIZE;
    }
    // END_OF_BODY 须携带至少 1 字节负载（空负载头不会被解析器记录）
    std::vector<uint8_t> finalFrame = MakeObexResponse(OBEX_HTTP_OK,
        MakeBodyHeader(END_OF_BODY, 1, 'B'));
    ASSERT_TRUE(WriteAll(fds[1], finalFrame));

    client_->DoQueryTask(TEST_NUMBER);

    // 溢出按传输级失败处理：链路拆除、会话与传输清空、状态复位
    EXPECT_EQ(client_->state_.load(), WatchPbapState::IDLE);
    EXPECT_FALSE(client_->isObexConnected_);
    EXPECT_EQ(client_->connectionId_, WATCH_PBAP_INVALID_CONNECTION_ID);
    EXPECT_EQ(client_->clientSession_, nullptr);
    EXPECT_EQ(client_->transport_, nullptr);
    EXPECT_EQ(client_->socketFd_, WATCH_PBAP_INVALID_SOCKET_FD);
    close(fds[1]);
    HILOGI("WatchPbapClient_DoQueryTask_003 end");
}
}  // namespace bluetooth
}  // namespace OHOS
