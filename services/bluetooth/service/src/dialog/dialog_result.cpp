/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_dialog_result"
#endif

#include "bluetooth_dialog.h"
#include "bluetooth_errorcode.h"
#include "bt_def.h"
#include "dialog_result.h"
#include "permission_manager.h"
#include "bt_chr_ue_manager.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

DialogResult::DialogResult()
{}

DialogResult::~DialogResult()
{}

int32_t DialogResult::HandleBluetoothSwitchDialogResult(bool isAsync)
{
    HILOGI("function async flag: %{public}d", isAsync);
    // sync func, return
    if (!isAsync) {
        return BT_ERR_DIALOG_FOR_USER_CONFIRM;
    }

    IsDialogResultReceived(false);
    // async func, wait for dialog result
    WaitAdapterManagerDialogResultComplete();
    std::string callingName = PermissionManager::GetCallingName();
    std::lock_guard<std::mutex> lock(getDialogResultMutex_);
    if (!isDialogResult_) {
        HILOGI("[DialogResult] No dialog result within %{public}d milliseconds", DIALOG_RESULT_DELAY_MS);
        BtChrUeManager::GetInstance()->WriteSwitchDialogResultUe(UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE2, callingName);
        return BT_ERR_DIALOG_FOR_USER_NOT_RESPOND;
    }
    if (dialogResult_) {
        BtChrUeManager::GetInstance()->WriteSwitchDialogResultUe(UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE1, callingName);
        return BT_ERR_DIALOG_FOR_USER_CONFIRM;
    } else {
        BtChrUeManager::GetInstance()->WriteSwitchDialogResultUe(UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE0, callingName);
        return BT_ERR_DIALOG_FOR_USER_REFUSE;
    }
}

void DialogResult::WaitAdapterManagerDialogResultComplete(void)
{
    std::unique_lock<std::mutex> lock(getDialogResultMutex_);
    if (isDialogResult_) {
        HILOGI("[DialogResult] no need to wait for dialog result");
        return;
    }
    // waiting for the dialog result to be received or timed out
    if (!getDialogResultConditionVariable_.wait_for(lock, std::chrono::milliseconds(DIALOG_RESULT_DELAY_MS),
        [this]() { return isDialogResult_; })) {
        HILOGW("get dialog result failed");
        return;
    }
}

void DialogResult::NotifyBluetoothSwitchDialogResult(uint32_t dialogType, bool dialogResult)
{
    std::lock_guard<std::mutex> lock(getDialogResultMutex_);
    if (dialogType == static_cast<uint32_t>(DialogBoxType::BLUETOOTH_SWITCH)) {
        isDialogResult_ = true; // dialog result is received
        dialogResult_ = dialogResult;
        HILOGI("[DialogResult] dialogResult_ is %{public}d", dialogResult_);
        getDialogResultConditionVariable_.notify_all(); // notify waiting thread
    }
}

void DialogResult::NotifyCancelPairDialogResult(uint32_t dialogType, bool dialogResult)
{
    std::lock_guard<std::mutex> lock(cancelPairDialogMutex_);
    if (dialogType == static_cast<uint32_t>(DialogBoxType::CANCEL_PAIR)) {
        isCancelPairDialogResultReceived_ = true; // dialog result is received
        cancelPairDialogResult_ = dialogResult;
        HILOGI("[DialogResult] cancelPairDialogResult_ is %{public}d", cancelPairDialogResult_);
        cancelPairDialogConditionVariable_.notify_all(); // notify waiting thread
    }
}

int32_t DialogResult::HandleCancelPairDialogResult(void)
{
    HILOGI("handle cancel pair dialog result");
    {
        std::lock_guard<std::mutex> lock(cancelPairDialogMutex_);
        isCancelPairDialogResultReceived_ = false;
    }
    WaitCancelPairDialogResultComplete();
    std::string callingName = PermissionManager::GetCallingName();
    std::lock_guard<std::mutex> lock(cancelPairDialogMutex_);
    if (!isCancelPairDialogResultReceived_) {
        HILOGI("[DialogResult] No cancel pair dialog result within %{public}d milliseconds", DIALOG_RESULT_DELAY_MS);
        BtChrUeManager::GetInstance()->WriteSwitchDialogResultUe(UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE2, callingName);
        return BT_ERR_DIALOG_FOR_USER_NOT_RESPOND;
    }
    if (cancelPairDialogResult_) {
        BtChrUeManager::GetInstance()->WriteSwitchDialogResultUe(UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE1, callingName);
        return BT_ERR_DIALOG_FOR_USER_CONFIRM;
    } else {
        BtChrUeManager::GetInstance()->WriteSwitchDialogResultUe(UE_COMMON_SCENE_CASE3,
            UE_COMMON_SCENE_CASE0, callingName);
        return BT_ERR_DIALOG_FOR_USER_REFUSE;
    }
}

void DialogResult::WaitCancelPairDialogResultComplete(void)
{
    std::unique_lock<std::mutex> lock(cancelPairDialogMutex_);
    if (isCancelPairDialogResultReceived_) {
        HILOGI("[DialogResult] no need to wait for cancel pair dialog result");
        return;
    }
    // waiting for the cancel pair dialog result to be received or timed out
    if (!cancelPairDialogConditionVariable_.wait_for(lock, std::chrono::milliseconds(DIALOG_RESULT_DELAY_MS),
        [this]() { return isCancelPairDialogResultReceived_; })) {
        HILOGW("get cancel pair dialog result failed");
        return;
    }
}

bool DialogResult::GetIsDialogResultReceived(void)
{
    std::lock_guard<std::mutex> lock(getDialogResultMutex_);
    return isDialogResult_;
}

void DialogResult::IsDialogResultReceived(bool isDialogResult)
{
    std::lock_guard<std::mutex> lock(getDialogResultMutex_);
    isDialogResult_ = isDialogResult;
}

bool DialogResult::GetDialogResult(void)
{
    std::lock_guard<std::mutex> lock(getDialogResultMutex_);
    return dialogResult_;
}

void DialogResult::SetDialogResult(bool dialogResult)
{
    std::lock_guard<std::mutex> lock(getDialogResultMutex_);
    dialogResult_ = dialogResult;
}

} // namespace bluetooth
} // namespace OHOS