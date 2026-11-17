/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef VCARD_UTIL_H
#define VCARD_UTIL_H

#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace stub {
/**
 * @brief Enumeration of vCard versions.
 */
enum class VCardVersion : uint8_t { VER_OTHER = 0x00, VER_2_1 = 0x21, VER_3_0 = 0x30 };

/**
 * @brief Type of vCard property parameters.
 *        The key is the parameter name and the value is the parameter value.
 *        A multimap is used because one parameter name may have multiple values.
 */
typedef std::multimap<std::string, std::string> VCardParams;

/**
 * @brief Represents a single vCard property (such as TEL, EMAIL or ADR).
 *        A property consists of a group, a name, values and parameters.
 */
class VCardProperty {
public:
    /**
     * @brief Construct a new VCardProperty object with a raw parameter string.
     *
     * @param ver vCard version.
     * @param group Group name of the property.
     * @param name Name of the property, such as "TEL".
     * @param value Value of the property.
     * @param paramsStr Raw parameter string of the property.
     */
    VCardProperty(VCardVersion ver, std::string group, std::string name, std::string value, std::string paramsStr)
    {}
    /**
     * @brief Construct a new VCardProperty object with parsed parameters.
     *
     * @param group Group name of the property.
     * @param name Name of the property, such as "TEL".
     * @param value Value of the property.
     * @param params Parsed parameters of the property.
     */
    VCardProperty(std::string group, std::string name, std::string value, VCardParams params)
    {}
    /**
     * @brief Destroy the VCardProperty object.
     */
    virtual ~VCardProperty() = default;
    /**
     * @brief Get the group name of the property.
     *
     * @return Group name of the property.
     */
    std::string GetGroup() const
    {
        return "";
    }
    /**
     * @brief Get the name of the property.
     *
     * @return Name of the property, such as "TEL".
     */
    std::string GetName() const
    {
        return "";
    }
    /**
     * @brief Get all values of the property.
     *
     * @return Reference to the value list of the property.
     */
    const std::vector<std::string> &GetValues() const
    {
        static std::vector<std::string> v;
        return v;
    }
    /**
     * @brief Get all parameters of the property.
     *
     * @return Reference to the parameter multimap of the property.
     */
    const VCardParams &GetParams() const
    {
        static VCardParams v;
        return v;
    }
    /**
     * @brief Build the parameter string of the property in vCard format.
     *
     * @param ver Target vCard version.
     * @param outList Output byte list of the built parameter string.
     */
    void BuildParamString(VCardVersion ver, std::vector<uint8_t> &outList) const
    {}
    /**
     * @brief Build the value string of the property in vCard format.
     *
     * @param outList Output byte list of the built value string.
     */
    void BuildValueString(std::vector<uint8_t> &outList) const
    {}

private:
    std::string group_ = "";               // Group name of the property
    std::string name_ = "";                // Name of the property
    std::vector<std::string> values_ {};   // Values of the property
    VCardParams params_ {};                // Parameters of the property
};

/**
 * @brief Represents a vCard, which is a collection of vCard properties.
 *        Used by the PBAP profile to exchange phonebook contacts.
 */
class VCard {
public:
    /**
     * @brief Construct a new VCard object.
     */
    VCard() = default;
    /**
     * @brief Destroy the VCard object.
     */
    virtual ~VCard() = default;
    /**
     * @brief Get the version of the vCard.
     *
     * @return Version of the vCard.
     */
    VCardVersion GetVersion() const
    {
        static VCardVersion v;
        return v;
    }
    /**
     * @brief Set the version of the vCard.
     *
     * @param ver Version to set.
     */
    void SetVersion(const VCardVersion ver)
    {}
    /**
     * @brief Convert a vCard version to its string representation.
     *
     * @param ver vCard version to convert.
     * @return String representation of the version, such as "2.1" or "3.0".
     */
    static std::string Version2Str(VCardVersion ver)
    {
        return "";
    }
    /**
     * @brief Convert a version string to a vCard version.
     *
     * @param str Version string to convert, such as "2.1" or "3.0".
     * @return Converted vCard version, or VER_OTHER if unrecognized.
     */
    static VCardVersion Str2Version(const std::string &str)
    {
        static VCardVersion v;
        return v;
    }
    /**
     * @brief Get all properties of the vCard.
     *
     * @return Reference to the property list of the vCard.
     */
    const std::vector<VCardProperty> &Properties() const
    {
        static std::vector<VCardProperty> v;
        return v;
    }
    /**
     * @brief Add a property to the vCard.
     *
     * @param property Property to add.
     */
    void AddProperty(const VCardProperty &property)
    {}
    /**
     * @brief Check whether the vCard contains a property with the specified id.
     *
     * @param propertyId Property id to check, such as "VERSION" or "N".
     * @return true if the vCard contains the property, otherwise false.
     */
    bool HasProperty(std::string propertyId)
    {
        return true;
    }
};

/**
 * @brief Utility class for building and parsing vCards.
 */
class VCardUtil {
public:
    /**
     * @brief Build the byte stream of a single vCard in the specified version.
     *
     * @param vCard vCard to build.
     * @param ver Target vCard version.
     * @return Byte stream of the built vCard.
     */
    static std::vector<uint8_t> Build(const VCard &vCard, VCardVersion ver)
    {
        static std::vector<uint8_t> v;
        return v;
    }
    /**
     * @brief Build the byte stream of multiple vCards in the specified version.
     *
     * @param vCards vCards to build.
     * @param ver Target vCard version.
     * @return Byte stream of the built vCards.
     */
    static std::vector<uint8_t> Build(const std::vector<VCard> &vCards, VCardVersion ver)
    {
        static std::vector<uint8_t> v;
        return v;
    }
    /**
     * @brief Parse vCards from an input stream.
     *
     * @param is Input stream containing vCard data.
     * @return Parsed vCard list.
     */
    static std::vector<VCard> Parse(std::istream &is)
    {
        std::vector<VCard> v;
        return v;
    }
    /**
     * @brief Parse vCards from a byte array.
     *
     * @param datas Byte array containing vCard data.
     * @return Parsed vCard list.
     */
    static std::vector<VCard> Parse(const std::vector<uint8_t> &datas)
    {
        std::vector<VCard> v;
        return v;
    }

private:
    /**
     * @brief Construct a new VCardUtil object. Private to prevent instantiation.
     */
    VCardUtil() = default;
    /**
     * @brief Destroy the VCardUtil object.
     */
    virtual ~VCardUtil() = default;
};
}  // namespace stub
#endif // VCARD_UTIL_H