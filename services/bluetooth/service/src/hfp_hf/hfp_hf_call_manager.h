/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef HFP_HF_CALL_MANAGER_H
#define HFP_HF_CALL_MANAGER_H

#include <string>
#include <vector>

#include "safe_map.h"
#include "hands_free_unit_calls.h"
#ifdef BLUETOOTH_CALL_FOCUS_ENABLE
#include "hfp_call_focus_manager.h"
#endif

namespace OHOS {
namespace bluetooth {
/**
 * @brief This class provides a set of methods that is used to process the current calls.
 */
class HfpHfCallManager {
public:
    /**
     * @brief Construct a new HfpHfCallManager object.
     *
     * @param address Remote device address.
     * @since 12
     */
    explicit HfpHfCallManager(const std::string &address);

    /**
     * @brief Destroy the HfpHfCallManager object.
     * @since 12
     */
    ~HfpHfCallManager();

    /**
     * @brief Has the call of the state.
     *
     * @param state : The call state.
     * @return Returns <b>true</b> for call that find the state;
     *         returns <b>false</b> for call that do not find the state.
     * @since 12
     */
    bool HasCallState(int state);

    /**
     * @brief Get the current calls.
     *
     * @return the current calls.
     * @since 12
     */
    std::vector<HandsFreeUnitCalls> GetCurrentCalls();

    /**
     * @brief Update the call.
     *
     * @param call The call.
     * @since 12
     */
    void UpdateCall(HandsFreeUnitCalls &call);

    /**
     * @brief Complete call update.
     * @since 12
     */
    void UpdateCallDone();

    /**
     * @brief Notify the call change.
     *
     * @param call The call.
     * @since 12
     */
    void NotifyCallChanged(const HandsFreeUnitCalls &call);

    /**
     * @brief Set sco connected flag.
     *
     * @param isScoConnected is sco connected.
     * @since 12
     */
    void SetScoConnected(bool isScoConnected);

#ifdef BLUETOOTH_CALL_FOCUS_ENABLE
    /**
     * @brief Init call focus manager.
     *
     * @param callback Callback invoked when the audio focus is interrupted by other audio streams.
     * @return Returns 0 on success, or a negative error code on failure.
     * @since 12
     */
    int32_t InitFocusManager(std::function<void(AudioStandard::InterruptHint)> callback);

    /**
     * @brief Send call state changed intent and handle audio focus.
     *
     * @since 12
     */
    void sendCallChangedIntent();

    /**
     * @brief Request audio focus.
     *
     * @since 12
     */
    void RequestCallFocusIfNeeded();

    /**
     * @brief Release audio focus.
     *
     * @since 12
     */
    void ReleaseCallFocusIfNeeded();
#endif

private:
    // The device address.
    std::string address_;
    // The current calls.
    SafeMap<int, HandsFreeUnitCalls> calls_ {};
    // The update calls.
    SafeMap<int, HandsFreeUnitCalls> updateCalls_ {};

    bool isScoConnected_{false};

    int64_t dialingCallTime_{0};
#ifdef BLUETOOTH_CALL_FOCUS_ENABLE
    HfpCallFocusManager focusManager_;

    /**
     * @brief Check whether there is any ongoing call.
     *
     * @return true if there is an ongoing call, false otherwise.
     * @since 12
     */
    bool haveCurrentCall();
#endif

    void AddNewCall();
    void ProcessUpdateCallTimeout();

};
}  // namespace bluetooth
}  // namespace OHOS
#endif // HFP_HF_CALL_MANAGER_H
