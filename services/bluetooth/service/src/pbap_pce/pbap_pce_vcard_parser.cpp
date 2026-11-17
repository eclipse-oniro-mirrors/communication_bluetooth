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

#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_vcard_parser"
#endif

#include "pbap_pce_vcard_parser.h"

#include <algorithm>
#include <cctype>
#include "log.h"

namespace OHOS {
namespace bluetooth {

/// 防御上限（远端 PSE 为不可信数据源）：单逻辑行（含折叠展开）、总行数、总卡片数
/// 超限时截断，防止无换行巨流/深折叠/海量空卡导致内存耗尽。合法 vCard（单行75
/// 字节折叠、单批最多数百卡）远低于上限，正常数据永不触顶。
constexpr size_t PBAP_PCE_MAX_LINE_LEN = 8 * 1024;
constexpr size_t PBAP_PCE_MAX_LINE_COUNT = 500000;
constexpr size_t PBAP_PCE_MAX_CARD_COUNT = 100000;

/// Trim leading/trailing whitespace from a string.
static std::string Trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

/// Convert ASCII string to uppercase in-place.
static std::string ToUpper(const std::string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::vector<PbapPceVCardContact> PbapPceVCardParser::Parse(const std::vector<uint8_t> &data)
{
    return Parse(data, PbapPceParseScenario::CONTACTS);
}

std::vector<PbapPceVCardContact> PbapPceVCardParser::Parse(
    const std::vector<uint8_t> &data, PbapPceParseScenario scenario)
{
    std::vector<PbapPceVCardContact> contacts;
    if (data.empty()) {
        HILOGI("[PbapPceVCardParser] Parse: data is empty");
        return contacts;
    }

    // Convert byte vector to string for line-by-line processing.
    std::string text(data.begin(), data.end());
    std::vector<std::string> lines = SplitAndUnfoldLines(text);

    HILOGI("[PbapPceVCardParser] Parse: raw bytes=%{public}zu, unfolded lines=%{public}zu",
        text.size(), lines.size());

    // keep-all 策略：BEGIN/END 配对完整的卡片一律保留（含空卡、无名卡、仅邮件卡），
    // 保证"解析数 == 传输条数"，使 ListStartOffset 的推进与 PSE 位置空间（位置 0
    // 为 0.vcf 名片槽位，请求从位置 1 起拉）严格对齐。场景化过滤（空卡丢弃）由
    // 入库方执行，不在解析层丢弃（scenario 仅用于日志标注）。
    bool inVCard = false;
    ParseStats stats {};
    PbapPceVCardContact currentContact {};
    for (const std::string &line : lines) {
        std::string trimmed = Trim(line);
        if (trimmed.empty()) {
            continue;
        }
        std::string upperLine = ToUpper(trimmed);
        if (upperLine == "BEGIN:VCARD") {
            stats.begunCount++;
            inVCard = true;
            currentContact = PbapPceVCardContact {};
            continue;
        }
        if (upperLine == "END:VCARD") {
            HandleEndOfVCard(contacts, currentContact, inVCard, stats);
            continue;
        }
        if (!inVCard) {
            stats.lineOutsideCount++;
            continue;
        }
        ParseLine(trimmed, currentContact, inVCard);
    }
    stats.truncatedTail = inVCard;
    LogParseSummary(scenario, stats);
    return contacts;
}

void PbapPceVCardParser::HandleEndOfVCard(std::vector<PbapPceVCardContact> &contacts,
    PbapPceVCardContact &currentContact, bool &inVCard, ParseStats &stats)
{
    if (inVCard) {
        if (contacts.size() < PBAP_PCE_MAX_CARD_COUNT) {
            contacts.push_back(currentContact);
            stats.keptCount++;
        } else {
            // 卡片数超限：丢弃并计数，防远端海量空 BEGIN/END 对撑爆内存
            stats.cappedDroppedCount++;
        }
    } else {
        stats.endWithoutBeginCount++;
    }
    inVCard = false;
    currentContact = PbapPceVCardContact {};
}

void PbapPceVCardParser::LogParseSummary(PbapPceParseScenario scenario, const ParseStats &stats)
{
    if (stats.truncatedTail) {
        // 流截断：末尾卡片缺少 END:VCARD，丢弃（offset 错位 1 条由边界重拉自愈）
        HILOGW("[PbapPceVCardParser] Parse: truncated vCard without END:VCARD, dropped");
    }
    // 畸形行统计在循环外汇总输出，避免远端发送海量垃圾行时逐行打日志造成日志洪泛
    if (stats.endWithoutBeginCount > 0 || stats.lineOutsideCount > 0 || stats.cappedDroppedCount > 0) {
        HILOGW("[PbapPceVCardParser] Parse: malformed input summary, endWithoutBegin=%{public}zu, "
            "lineOutside=%{public}zu, cappedDroppedCards=%{public}zu",
            stats.endWithoutBeginCount, stats.lineOutsideCount, stats.cappedDroppedCount);
    }
    size_t incompleteCount = stats.begunCount > stats.keptCount ? stats.begunCount - stats.keptCount : 0;
    HILOGI("[PbapPceVCardParser] Parse: scenario=%{public}d, begun=%{public}zu, kept=%{public}zu, "
        "incomplete=%{public}zu, parsed vCard contacts",
        static_cast<int32_t>(scenario), stats.begunCount, stats.keptCount, incompleteCount);
}

std::vector<std::string> PbapPceVCardParser::SplitAndUnfoldLines(const std::string &text)
{
    // Split into lines, handling both \r\n and \n line endings.
    // 单行长度与总行数均有防御上限（PSE 为不可信数据源）：超限截断并计数，
    // 防止无换行巨流/海量折叠行导致内存无界增长。
    std::vector<std::string> rawLines;
    std::string currentLine;
    bool truncated = false;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            // Strip trailing \r if present
            if (!currentLine.empty() && currentLine.back() == '\r') {
                currentLine.pop_back();
            }
            if (rawLines.size() < PBAP_PCE_MAX_LINE_COUNT) {
                rawLines.push_back(currentLine);
            } else {
                truncated = true;
            }
            currentLine.clear();
        } else if (currentLine.size() < PBAP_PCE_MAX_LINE_LEN) {
            currentLine += text[i];
        } else {
            // 超长行：丢弃超出上限的字节，继续扫描至行尾
            truncated = true;
        }
    }
    if (!currentLine.empty()) {
        if (currentLine.back() == '\r') {
            currentLine.pop_back();
        }
        if (rawLines.size() < PBAP_PCE_MAX_LINE_COUNT) {
            rawLines.push_back(currentLine);
        } else {
            truncated = true;
        }
    }
    if (truncated) {
        HILOGW("[PbapPceVCardParser] SplitAndUnfoldLines: input exceeds line-len/line-count cap, truncated");
    }

    // Unfold lines: vCard 3.0 spec says a line beginning with a single space
    // is a continuation of the previous line (the space is removed).
    // 折叠展开后单逻辑行同样受 PBAP_PCE_MAX_LINE_LEN 上限保护。
    std::vector<std::string> lines;
    for (const std::string &raw : rawLines) {
        if (!raw.empty() && raw[0] == ' ' && !lines.empty()) {
            // Continuation line: append to previous (skip the leading space)
            size_t appendLen = raw.size() - 1;
            size_t room = lines.back().size() >= PBAP_PCE_MAX_LINE_LEN ? 0 :
                PBAP_PCE_MAX_LINE_LEN - lines.back().size();
            lines.back() += raw.substr(1, std::min(appendLen, room));
        } else {
            lines.push_back(raw);
        }
    }
    return lines;
}

void PbapPceVCardParser::ParseLine(const std::string &line, PbapPceVCardContact &contact, bool &inVCard)
{
    std::string propName;
    std::map<std::string, std::string> params;
    std::string value;

    // BEGIN:VCARD/END:VCARD/卡片外行均由 Parse 循环处理，本函数仅处理卡片内
    // 的属性行；防御性保留卡片外忽略分支。
    if (!inVCard) {
        HILOGW("ParseLine: line outside vCard, ignored");
        return;
    }

    SplitContentLine(line, propName, params, value);

    if (propName == "VERSION") {
        contact.version = value;
    } else if (propName == "FN") {
        // FN 为 TEXT 值：按规范反转义（\, \; \\ \n），否则转义符残留入库
        contact.displayName = UnescapeTextValue(value);
    } else if (propName == "N") {
        HandleNProperty(contact, value);
    } else if (propName == "TEL") {
        // TEL 值非 TEXT 类型（PSE 不做转义），保持原样
        std::string phone = Trim(value);
        if (!phone.empty()) {
            contact.phones.push_back(phone);
        }
    } else if (propName == "EMAIL") {
        std::string email = UnescapeTextValue(Trim(value));
        if (!email.empty()) {
            contact.emails.push_back(email);
        }
    } else if (propName.size() > 2 && propName[0] == 'X' && propName[1] == '-') {
        HandleUnknownXProperty(contact, propName, params, value);
    }
}

void PbapPceVCardParser::HandleNProperty(PbapPceVCardContact &contact, const std::string &value)
{
    // N: family;given;middle;prefix;suffix
    if (contact.displayName.empty()) {
        contact.displayName = BuildDisplayNameFromN(value);
    }
    // 转义感知分割（"\;" 为字面分号不拆列）；先分割后反转义，顺序不可颠倒
    std::vector<std::string> parts = SplitEscaped(value, ';');
    if (parts.size() > 0) {
        contact.familyName = UnescapeTextValue(Trim(parts[0]));
    }
    if (parts.size() > 1) {
        contact.givenName = UnescapeTextValue(Trim(parts[1]));
    }
    if (parts.size() > 2) {
        contact.middleName = UnescapeTextValue(Trim(parts[2]));
    }
}

void PbapPceVCardParser::HandleUnknownXProperty(PbapPceVCardContact &contact, const std::string &propName,
    const std::map<std::string, std::string> &params, const std::string &value)
{
    // Only log the property count, not property names or values (privacy).
    HILOGD("ParseLine: unknown X- property, params count=%{public}zu", params.size());
    std::string fullPropName = propName;
    for (const auto &param : params) {
        fullPropName += ";" + param.first;
        if (!param.second.empty()) {
            fullPropName += "=" + param.second;
        }
    }
    contact.unknownXData.push_back(std::make_pair(fullPropName, value));
}

void PbapPceVCardParser::SplitContentLine(const std::string &line, std::string &propName,
    std::map<std::string, std::string> &params, std::string &value)
{
    // A vCard content line: PROPERTY;PARAM1=val1;PARAM2=val2:VALUE
    // The first colon (not inside quotes) separates "property+params" from "value".
    // Property name and params are separated by semicolons.

    // Find the first unquoted colon to split property-params from value.
    size_t colonPos = FindUnquotedColon(line);
    if (colonPos == std::string::npos) {
        HILOGW("SplitContentLine: no colon found in line, treated as property name only");
        propName = ToUpper(Trim(line));
        value = "";
        return;
    }

    std::string propAndParams = line.substr(0, colonPos);
    value = line.substr(colonPos + 1);

    // Split property and params by semicolon (unquoted)
    std::vector<std::string> segments;
    std::string current;
    bool inQuotes = false;
    for (size_t i = 0; i < propAndParams.size(); i++) {
        if (propAndParams[i] == '"') {
            inQuotes = !inQuotes;
            current += propAndParams[i];
        } else if (propAndParams[i] == ';' && !inQuotes) {
            segments.push_back(current);
            current.clear();
        } else {
            current += propAndParams[i];
        }
    }
    segments.push_back(current);

    if (segments.empty()) {
        propName = "";
        return;
    }

    // First segment is the property name
    propName = ToUpper(Trim(segments[0]));

    // Remaining segments are parameters: PARAM=value or just PARAM (e.g. TYPE param)
    ParseParams(segments, params);
}

size_t PbapPceVCardParser::FindUnquotedColon(const std::string &line)
{
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '"') {
            inQuotes = !inQuotes;
        } else if (line[i] == ':' && !inQuotes) {
            return i;
        }
    }
    return std::string::npos;
}

void PbapPceVCardParser::ParseParams(const std::vector<std::string> &segments,
    std::map<std::string, std::string> &params)
{
    for (size_t i = 1; i < segments.size(); i++) {
        std::string seg = segments[i];
        size_t eqPos = seg.find('=');
        if (eqPos != std::string::npos) {
            std::string paramKey = ToUpper(Trim(seg.substr(0, eqPos)));
            std::string paramVal = Trim(seg.substr(eqPos + 1));
            // Strip surrounding quotes from value
            if (paramVal.size() >= 2 && paramVal.front() == '"' && paramVal.back() == '"') {
                paramVal = paramVal.substr(1, paramVal.size() - 2);
            }
            params[paramKey] = paramVal;
        } else {
            // Parameter without value (e.g. "MISSED" in X-IRMC-CALL-DATETIME;MISSED)
            // Store with empty value, key is the segment itself.
            std::string paramKey = Trim(seg);
            if (!paramKey.empty()) {
                params[paramKey] = "";
            }
        }
    }
}

std::string PbapPceVCardParser::BuildDisplayNameFromN(const std::string &nValue)
{
    // N: family;given;middle;prefix;suffix
    // Build display name as "given family" (common format)
    std::vector<std::string> parts = SplitEscaped(nValue, ';');
    std::string family = (parts.size() > 0) ? UnescapeTextValue(Trim(parts[0])) : "";
    std::string given = (parts.size() > 1) ? UnescapeTextValue(Trim(parts[1])) : "";

    std::string displayName;
    if (!given.empty() && !family.empty()) {
        displayName = given + " " + family;
    } else if (!given.empty()) {
        displayName = given;
    } else if (!family.empty()) {
        displayName = family;
    }
    return displayName;
}

std::string PbapPceVCardParser::UnescapeTextValue(const std::string &value)
{
    // RFC 2426 §2.5 / RFC 6350 §3.4: TEXT 值转义集为 \\ \, \; \n \N。
    // 规范外的 "\x" 按字面保留（不吞字符），兼容 PSE 端不规范输出；
    // 末尾孤立 '\' 同样按字面保留。
    std::string result;
    result.reserve(value.size());
    size_t i = 0;
    while (i < value.size()) {
        if (value[i] != '\\' || i + 1 >= value.size()) {
            result += value[i];
            i++;
            continue;
        }
        char next = value[i + 1];
        switch (next) {
            case '\\':
            case ',':
            case ';':
                result += next;
                break;
            case 'n':
            case 'N':
                result += '\n';
                break;
            default:
                result += value[i];
                result += next;
                break;
        }
        i += 2;
    }
    return result;
}

std::vector<std::string> PbapPceVCardParser::SplitEscaped(const std::string &value, char delimiter)
{
    // 转义感知分割：'\x' 使下一字符不作为分隔符（N 值中 "\;" 为字面分号，
    // 不得拆列）。分割结果仍含转义序列，调用方需再对每段做 UnescapeTextValue。
    std::vector<std::string> parts;
    std::string current;
    size_t i = 0;
    while (i < value.size()) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            current += value[i];
            current += value[i + 1];
            i += 2;
        } else if (value[i] == delimiter) {
            parts.push_back(current);
            current.clear();
            i++;
        } else {
            current += value[i];
            i++;
        }
    }
    parts.push_back(current);
    return parts;
}

}  // namespace bluetooth
}  // namespace OHOS
