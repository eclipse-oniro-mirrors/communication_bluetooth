/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "enterprise_device_mgr_stub_mock.h"

namespace OHOS {
namespace EDM {
int EnterpriseDeviceMgrStubMock::InvokeSendRequestSetPolicy(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    GTEST_LOG_(INFO) << "mock EnterpriseDeviceMgrStubMock InvokeSendRequestGetPolicy code :" << code;
    code_ = code;
    std::vector<std::string> writeArray{ "11:22:33:44:55:66", "11:22:33:44:55:GG" };
    reply.WriteInt32(ERR_OK);
    reply.WriteInt32(writeArray.size());
    reply.WriteStringVector(writeArray);
    return 0;
}

int EnterpriseDeviceMgrStubMock::InvokeSendRequestRemovePolicy(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    GTEST_LOG_(INFO) << "mock EnterpriseDeviceMgrStubMock InvokeSendRequestRemovePolicy code :" << code;
    code_ = code;
    std::vector<std::string> writeArray{};
    reply.WriteInt32(ERR_OK);
    reply.WriteInt32(writeArray.size());
    reply.WriteStringVector(writeArray);
    return 0;
}

} // namespace EDM
} // namespace OHOS
