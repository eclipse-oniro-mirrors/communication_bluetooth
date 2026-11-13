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

#ifndef TEST_BT_UE_MANAGER_H
#define TEST_BT_UE_MANAGER_H

#include <string>
#include <cstdint>

#define CHR_UE_SENSORHUB_COLLABORATION_EVENT "sensorhub_ue"

struct SensorhubCollaborationUe {
    uint32_t sensorhubResetCnt;

    uint32_t scanCollCnt;
    uint32_t scanCollFailedCnt;
    uint32_t scanCollStartCnt;
    uint32_t scanCollAvgMs;
    uint32_t scanCollMaxMs;
    std::string shScanParam;

    uint32_t connCollCnt;
    uint32_t connCollFailedCnt;
    uint32_t connCollAvgMs;
    uint32_t connCollMaxMs;
};

class BtChrUeManager {
public:
    BtChrUeManager();
    static BtChrUeManager *GetInstance();

    void WriteSensorhubCollaborationUe(const std::string &eventName, const SensorhubCollaborationUe &shColl);
};

#endif