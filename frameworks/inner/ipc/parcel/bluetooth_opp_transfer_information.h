/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_PARCEL_OPP_TRANSFER_INFORMATION_H
#define BLUETOOTH_PARCEL_OPP_TRANSFER_INFORMATION_H


#include "parcel.h"
#include "opp_transfer_information.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothIOppTransferInformation : public Parcelable, public bluetooth::IOppTransferInformation {
public:
    BluetoothIOppTransferInformation() = default;
    ~BluetoothIOppTransferInformation() override = default;
    explicit BluetoothIOppTransferInformation(const bluetooth::IOppTransferInformation& other);

    bool Marshalling(Parcel &parcel) const override;

    static BluetoothIOppTransferInformation *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel) const;

    bool ReadFromParcel(Parcel &parcel);
};

class BluetoothIOppTransferFileHolder : public Parcelable, public bluetooth::IOppTransferFileHolder {
public:
    BluetoothIOppTransferFileHolder() = default;
    ~BluetoothIOppTransferFileHolder() override = default;

    explicit BluetoothIOppTransferFileHolder(const bluetooth::IOppTransferFileHolder& other);
    explicit BluetoothIOppTransferFileHolder(const std::string &filePath,
        const int64_t &fileSize, const int32_t &fileFd);
    
    bool Marshalling(Parcel &parcel) const override;

    static BluetoothIOppTransferFileHolder *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel) const;

    bool ReadFromParcel(Parcel &parcel);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_OPP_TRANSFER_INFORMATION_H