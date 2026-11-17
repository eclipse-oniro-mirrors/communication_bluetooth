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

#ifndef LOG_TAG
#define LOG_TAG "avrcp_ct_bip_image_properties"
#endif

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "avrcp_ct_bip_image_properties.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {
namespace {
bool IsThumbnailNode(xmlNodePtr node)
{
    xmlChar *encoding = xmlGetProp(node, BAD_CAST "encoding");
    xmlChar *pixel = xmlGetProp(node, BAD_CAST "pixel");
    bool isThumbnail = false;
    if (encoding != nullptr && pixel != nullptr) {
        isThumbnail = std::string(reinterpret_cast<char *>(encoding)) == "JPEG" &&
                      std::string(reinterpret_cast<char *>(pixel)) == "200*200";
    }
    if (encoding != nullptr) {
        xmlFree(encoding);
    }
    if (pixel != nullptr) {
        xmlFree(pixel);
    }
    return isThumbnail;
}
}

bool BipImageProperties::IsValid() const
{
    return version_ == VERSION && !imageHandle_.empty() && hasThumbnailFormat_;
}

bool BipImageProperties::ParseFromData(const std::vector<uint8_t> &data)
{
    if (data.empty()) {
        HILOGE("Empty data");
        return false;
    }

    std::string xml(data.begin(), data.end());

    xmlDocPtr doc = xmlParseMemory(xml.c_str(), xml.size());
    if (doc == nullptr) {
        HILOGE("Failed to parse XML");
        return false;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        HILOGE("Empty XML document");
        xmlFreeDoc(doc);
        return false;
    }

    xmlChar *handle = xmlGetProp(root, BAD_CAST "handle");
    if (handle == nullptr) {
        HILOGW("No handle attribute found in XML");
        xmlFreeDoc(doc);
        return false;
    }
    imageHandle_ = reinterpret_cast<char *>(handle);
    xmlFree(handle);

    xmlChar *version = xmlGetProp(root, BAD_CAST "version");
    if (version != nullptr) {
        version_ = reinterpret_cast<char *>(version);
        xmlFree(version);
    }

    hasThumbnailFormat_ = false;
    for (xmlNodePtr child = root->children; child != nullptr; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && IsThumbnailNode(child)) {
            hasThumbnailFormat_ = true;
            break;
        }
    }

    HILOGI("Parsed: handle=%{public}s, version=%{public}s, hasThumbnailFormat=%{public}d", imageHandle_.c_str(),
           version_.c_str(), hasThumbnailFormat_);

    xmlFreeDoc(doc);
    return true;
}

std::string BipImageProperties::ToString() const
{
    return "BipImageProperties { handle=" + imageHandle_ + ", version=" + version_ +
           ", hasThumbnail=" + (hasThumbnailFormat_ ? "true" : "false") + " }";
}

}  // namespace bluetooth
}  // namespace OHOS