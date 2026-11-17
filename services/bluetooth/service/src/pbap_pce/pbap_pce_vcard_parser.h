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

#ifndef PBAP_PCE_VCARD_PARSER_H
#define PBAP_PCE_VCARD_PARSER_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace OHOS {
namespace bluetooth {

/// Parsed vCard contact entry.
/// (VERSION | FN | N | TEL) plus X-IRMC-CALL-DATETIME for call logs.
struct PbapPceVCardContact {
    /// Formatted display name (FN property, fallback to N property joined).
    std::string displayName {};
    /// Family name (N property, first element).
    std::string familyName {};
    /// Given name (N property, second element).
    std::string givenName {};
    /// Middle name (N property, third element).
    std::string middleName {};
    /// Phone numbers (TEL property, may be multiple).
    std::vector<std::string> phones {};
    /// Email addresses (EMAIL property, may be multiple).
    std::vector<std::string> emails {};
    /// Unknown X- properties, pair.first=propertyName, pair.second=propertyValue.
    /// Includes X-IRMC-CALL-DATETIME for call log timestamps.
    std::vector<std::pair<std::string, std::string>> unknownXData {};
    /// vCard version string (e.g. "3.0", "2.1").
    std::string version {};
};

/// Parsing scenario for vCard stream.
/// NOTE: keep policy is scenario-independent (keep-all)：所有 BEGIN/END 配对完整
/// 的卡片一律保留，保证"解析数 == 传输条数"，使 ListStartOffset 推进与 PSE
/// 位置空间严格对齐。场景化过滤（名片跳过/空卡丢弃）由入库方执行。
/// scenario 仅保留用于日志标注。
enum class PbapPceParseScenario {
    /// Contacts scenario (telecom/pb).
    CONTACTS,
    /// Call logs scenario (telecom/mch/ich/och).
    CALL_LOGS,
};

/// Lightweight vCard parser for PBAP PCE.
/// Parses vCard 2.1/3.0 byte stream in-memory (no temp files)
/// Only extracts fields needed by PBAP PCE: VERSION/FN/N/TEL/EMAIL/X-*.
class PbapPceVCardParser {
public:
    /// Parse a vCard byte stream into a list of contacts.
    /// Keep-all：每个 BEGIN/END 配对完整的卡片均保留（含空卡/无名卡），
    /// 截断（缺 END:VCARD）的末尾卡片丢弃并计入 incomplete 统计。
    /// @param data vCard byte data from OBEX GET response body.
    /// @return List of parsed vCard contacts.
    static std::vector<PbapPceVCardContact> Parse(const std::vector<uint8_t> &data);

    /// Parse a vCard byte stream into a list of contacts with scenario.
    /// Keep-all 策略与 scenario 无关（见 PbapPceParseScenario 说明）。
    /// @param data vCard byte data from OBEX GET response body.
    /// @param scenario Parsing scenario (CONTACTS or CALL_LOGS), log annotation only.
    /// @return List of parsed vCard contacts.
    static std::vector<PbapPceVCardContact> Parse(
        const std::vector<uint8_t> &data, PbapPceParseScenario scenario);

private:
    /// Parse-loop statistics counters, shared by END:VCARD handling and summary
    /// logging (bundled into a struct to keep parameter count within gate limit).
    struct ParseStats {
        /// Stream ended inside a vCard (no END:VCARD), tail card dropped.
        bool truncatedTail = false;
        /// BEGIN:VCARD lines seen.
        size_t begunCount = 0;
        /// Completed cards kept (keep-all policy).
        size_t keptCount = 0;
        /// END:VCARD lines without a matching BEGIN:VCARD.
        size_t endWithoutBeginCount = 0;
        /// Content lines outside any BEGIN/END block.
        size_t lineOutsideCount = 0;
        /// Cards dropped over the max-card-count cap.
        size_t cappedDroppedCount = 0;
    };

    /// Parse a single logical content line (inside a vCard) into the current
    /// contact. BEGIN:VCARD/END:VCARD/卡片外行由 Parse 循环处理，本函数不处理。
    /// @param line Logical vCard line (line-folding already unfolded).
    /// @param contact Current contact being built.
    /// @param inVCard Whether we are inside a BEGIN:VCARD..END:VCARD block.
    static void ParseLine(const std::string &line, PbapPceVCardContact &contact, bool &inVCard);

    /// Handle the END:VCARD line: complete the current card (keep-all, with card
    /// count cap) or count a malformed END-without-BEGIN.
    static void HandleEndOfVCard(std::vector<PbapPceVCardContact> &contacts,
        PbapPceVCardContact &currentContact, bool &inVCard, ParseStats &stats);

    /// Log parse result summary after the loop: truncated tail, malformed input
    /// counters and keep-all statistics (aggregated to avoid log flooding).
    static void LogParseSummary(PbapPceParseScenario scenario, const ParseStats &stats);

    /// Split a vCard content line "PROPERTY;PARAM=val:VALUE" into
    /// property name (uppercased), parameter map, and value.
    static void SplitContentLine(const std::string &line, std::string &propName,
        std::map<std::string, std::string> &params, std::string &value);

    /// Build display name from N property value "family;given;middle;prefix;suffix".
    static std::string BuildDisplayNameFromN(const std::string &nValue);

    /// Unescape a vCard TEXT value per RFC 2426 §2.5 / RFC 6350 §3.4:
    /// "\\"→'\', "\\,"→',', "\\;"→';', "\\n"/"\\N"→LF.
    /// 未知转义 "\x" 按字面保留（防御不规范 PSE，不吞字符）。
    static std::string UnescapeTextValue(const std::string &value);

    /// 转义感知分割：'\x' 保护下一字符不作为分隔符（如 N 值中 "\;" 为字面
    /// 分号，不拆列）。用于结构性分割，必须先分割后反转义（顺序不可颠倒：
    /// 反转义产物中的 ';' 已是字面值，不可再用于分列）。
    static std::vector<std::string> SplitEscaped(const std::string &value, char delimiter);

    /// Split text into lines (\r\n or \n) and unfold vCard folded lines
    /// (a line beginning with a space continues the previous line).
    static std::vector<std::string> SplitAndUnfoldLines(const std::string &text);

    /// Handle the N property: "family;given;middle;prefix;suffix".
    static void HandleNProperty(PbapPceVCardContact &contact, const std::string &value);

    /// Handle unknown X- properties: rebuild full property name with params.
    static void HandleUnknownXProperty(PbapPceVCardContact &contact, const std::string &propName,
        const std::map<std::string, std::string> &params, const std::string &value);

    /// Find the first unquoted colon in the line.
    /// @return position of colon, or std::string::npos if none found.
    static size_t FindUnquotedColon(const std::string &line);

    /// Split property-params segments by unquoted semicolon and parse
    /// the PARAM=value pairs (starting from segments[1]) into params.
    static void ParseParams(const std::vector<std::string> &segments,
        std::map<std::string, std::string> &params);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_VCARD_PARSER_H
