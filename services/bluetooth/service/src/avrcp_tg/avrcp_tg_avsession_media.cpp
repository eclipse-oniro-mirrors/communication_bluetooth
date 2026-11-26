/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_avrcp_tg_media"
#endif

#include <unordered_map>
#include "avrcp_tg_avsession_media.h"
#include <set>
#include <map>
#ifdef MULTI_A2DP_ENABLED
#include "multi_a2dp_manager.h"
#endif
#include "avsession_errors.h"
#include "avsession_manager.h"
#include "bt_chr_ue_manager.h"
#include "avrc_defs.h"
#include "common_util.h"
#include "iservice_registry.h"
#include "log.h"
#include "log_utils.h"
#include "thread_util.h"
#include "hitrace_meter.h"
#include "avsession_pixel_map_adapter.h"
#include "avrcp_cover_art_storage.h"
#include "system_ability_subscriber.h"
#ifdef AUDIO_FRAMEWORK
#include "play_helper.h"
#endif
static const size_t MAX_TITLE_LENGTH = 255; // 标题最长255个字符
constexpr int32_t AVSESSION_SERVICE_ID = 3010;
constexpr int32_t COVER_ART_STORAGE_MAX_ITEMS = 32;
namespace OHOS {
namespace bluetooth {
struct AvrcpMediaInterfaceImpl::impl {
public:
    friend class AvrcpMediaInterfaceImpl;
    impl() = default;
    ~impl() = default;
    void Init(std::weak_ptr<AvrcpMediaInterfaceImpl::impl> impl);
    void CleanUp();
    void OnSessionRelease(const AVSession::AVSessionDescriptor &descriptor);
    void OnTopSessionChange(const AVSession::AVSessionDescriptor &descriptor);
    void OnMetaDataChange(const AVSession::AVMetaData &data);
    void OnPlaybackStateChange(const AVSession::AVPlaybackState &state);
#ifdef MULTI_A2DP_ENABLED
    void OnMetaDataChangeByAddr(const AVSession::AVMetaData &data, const std::string &macAddress);
    void OnPlaybackStateChangeByAddr(const AVSession::AVPlaybackState &state, const std::string &macAddress);
#endif
    void SendKeyEvent(uint8_t key, KeyState state);
    void GetSongInfo(SongInfoCallback cb);
    void GetPlayStatus(PlayStatusCallback cb);
    void GetNowPlayingList(NowPlayingCallback cb);
    void GetMediaPlayerList(MediaListCallback cb);
    void GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb);
    void SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb);
    void RegisterUpdateCallback(MediaCallbacks *callback);
    void UnregisterUpdateCallback(MediaCallbacks *callback);
    void PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId);
    void SetActiveDevice(const STACK::RawAddress &address);
    void AppSettingsSupport(AppSettingsCallback appCb);
    void SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode);
    void SetA2dpService(IProfileA2dp *a2dpService);
    void SetAudioManager(IProfileAudioManager *audioManager);
    void SendKeyEventByWearDetection(uint8_t key, KeyState state);
    void SendCtrlCommandToAvSession(uint8_t key);
    void PlaySilenceSound(void);
    void NotifyA2dpPlayStateChange(void);
    void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice);
    /**
     * @brief This class implement the <b>SessionListender</b> interface for observing the state change.
     */
    class AVSessionObserverImpl : public AVSession::SessionListener {
    public:
        explicit AVSessionObserverImpl(std::weak_ptr<AvrcpMediaInterfaceImpl::impl> impl) : pimpl_(impl) {}
        ~AVSessionObserverImpl() = default;

        void OnSessionCreate(const AVSession::AVSessionDescriptor &descriptor) override {}

        void OnSessionRelease(const AVSession::AVSessionDescriptor &descriptor) override
        {
            auto ptr = pimpl_.lock();
            if (ptr) {
                auto func = [ptr](const AVSession::AVSessionDescriptor &descriptor) {
                    ptr->OnSessionRelease(descriptor);
                };
                DoInAvrcpTgThread([func, descriptor]() { func(descriptor); });
            }
        }
        void OnTopSessionChange(const AVSession::AVSessionDescriptor &descriptor) override
        {
            auto ptr = pimpl_.lock();
            if (ptr) {
                auto func = [ptr](const AVSession::AVSessionDescriptor &descriptor) {
                    ptr->OnTopSessionChange(descriptor);
                };
                DoInAvrcpTgThread([func, descriptor]() { func(descriptor); });
            }
        }

    private:
        std::weak_ptr<AvrcpMediaInterfaceImpl::impl> pimpl_;
    };

    /**
     * @brief This class implement the <b>SessionListender</b> interface for observing the state change.
     */
    class AVControllerObserverImpl : public AVSession::AVControllerCallback {
    public:
#ifdef MULTI_A2DP_ENABLED
        explicit AVControllerObserverImpl(std::weak_ptr<AvrcpMediaInterfaceImpl::impl> impl,
                                         const std::string &macAddress = "")
            : pimpl_(impl), macAddress_(macAddress) {}
#else
        explicit AVControllerObserverImpl(std::weak_ptr<AvrcpMediaInterfaceImpl::impl> impl)
            : pimpl_(impl) {}
#endif
        ~AVControllerObserverImpl() = default;

        void OnSessionDestroy() override {}
        void OnMetaDataChange(const AVSession::AVMetaData &data) override
        {
            HILOGD("received metaDataChange event");
            auto ptr = pimpl_.lock();
            if (ptr) {
#ifdef MULTI_A2DP_ENABLED
                auto func = [ptr, this](const AVSession::AVMetaData &data) {
                    if (!macAddress_.empty()) {
                        ptr->OnMetaDataChangeByAddr(data, macAddress_);
                        return;
                    }
                    ptr->OnMetaDataChange(data);
                };
#else
                auto func = [ptr](const AVSession::AVMetaData &data) {
                    ptr->OnMetaDataChange(data);
                };
#endif
                DoInAvrcpTgThread([func, data]() { func(data); });
            }
        }
        void OnActiveStateChange(bool isActive) override {}
        void OnValidCommandChange(const std::vector<int32_t> &cmds) override {}
        void OnOutputDeviceChange(const int32_t, const AVSession::OutputDeviceInfo &) override {}
        void OnSessionEventChange(const std::string &event, const AAFwk::WantParams &args) override {}
        void OnQueueItemsChange(const std::vector<AVSession::AVQueueItem> &items) override {}
        void OnQueueTitleChange(const std::string &title) override {}
        void OnExtrasChange(const AAFwk::WantParams &extras) override {}
        void OnAVCallStateChange(const AVSession::AVCallState &avCallState) override {}
        void OnAVCallMetaDataChange(const AVSession::AVCallMetaData &avCallMetaData) override {}

        void OnPlaybackStateChange(const AVSession::AVPlaybackState &state) override
        {
            HILOGI("state:%{public}d, position:%{public}u %{public}u loopMode: %{public}d", state.GetState(),
                   state.GetPosition().elapsedTime_, state.GetPosition().updateTime_, state.GetLoopMode());
            auto ptr = pimpl_.lock();
            if (ptr) {
#ifdef MULTI_A2DP_ENABLED
                auto func = [ptr, this](const AVSession::AVPlaybackState &state) {
                    if (!macAddress_.empty()) {
                        ptr->OnPlaybackStateChangeByAddr(state, macAddress_);
                        return;
                    }
                    ptr->OnPlaybackStateChange(state);
                };
#else
                auto func = [ptr](const AVSession::AVPlaybackState &state) {
                    ptr->OnPlaybackStateChange(state);
                };
#endif
                DoInAvrcpTgThread([func, state]() { func(state); });
            }
        }

    private:
        std::weak_ptr<AvrcpMediaInterfaceImpl::impl> pimpl_;
        // Empty for the global top-session observer; bound to the per-device
        // controller's MAC for avSessionControllerMap_ observers (multi A2DP).
#ifdef MULTI_A2DP_ENABLED
        std::string macAddress_;
#endif
    };

private:
    void RegisterAvSessionListener();
    // avrcp connect after music play, try create avsession controller
    void AttemptCreateAvSessionController();
    void CreateAvSessionController(const std::string &sessionId);
#ifdef MULTI_A2DP_ENABLED
    bool CreateAvSessionControllerByAddr(const std::string &sessionId, const std::string &macAddress);
    bool IsTopSessionInCastByAddr(const std::string &macAddress);
    bool GetAvsessionControllerByAddr(const std::string &macAddress, uint8_t key,
        std::shared_ptr<AVSession::AVSessionController> &controller);
    void SendKeyEventByWearDetectionByAddr(uint8_t key, KeyState state,
        const STACK::RawAddress &address);
    void PlaySilenceSoundByAddr(const std::string &macAddress);
    void NotifyLoopModeChangedByAddr(const std::string &macAddress, const AVSession::AVPlaybackState &state);
    void TryBindSessionToMultiDevice(int32_t userId, const std::string &sessionId);
#endif
    void SetAvsessionFilter();
    void NotifyMediaStackChanged(uint8_t key);
    bool GetCurrSongInfo(SongInfo &song);
    bool ConvertMetaDataToSongInfo(const AVSession::AVMetaData &data, SongInfo &song);
    PlayState ConvertAvSessionStateToStack(int32_t state);
    void QueryAppSettingsMode();
    void ConvertAvSessionLoopModeToStack(AVSession::AVPlaybackState &state, uint8_t &shuffleMode, uint8_t &repeatMode);
    bool CheckTrackStateChanged(const AVSession::AVMetaData &data);
    bool CheckPlaybackStateChanged(const AVSession::AVPlaybackState &state);
    bool CheckLoopModeChanged(const AVSession::AVPlaybackState &state);
    void SendKeyEventToAvSession(uint8_t key);
#ifdef MULTI_A2DP_ENABLED
    void SendCtrlCommandToAvSession(uint8_t key, const RawAddress &address);
    void SetAvsessionFilterForController(const std::shared_ptr<AVSession::AVSessionController> &controller);
    void GetSongInfoByAddr(SongInfoCallback cb, const STACK::RawAddress &address);
    void GetPlayStatusByAddr(PlayStatusCallback cb, const STACK::RawAddress &address);
    void GetNowPlayingListByAddr(NowPlayingCallback cb, const STACK::RawAddress &address);
    void GetMediaPlayerListByAddr(MediaListCallback cb, const STACK::RawAddress &address);
    void GetFolderItemsByAddr(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb,
        const STACK::RawAddress &address);
    void SetBrowsedPlayerByAddr(uint16_t playerId, SetBrowsedPlayerCallback browseCb,
        const STACK::RawAddress &address);
    void PlayItemByAddr(uint16_t playerId, bool nowPlaying, std::string mediaId,
        const STACK::RawAddress &address);
    bool GetCurrSongInfoByAddr(SongInfo &song, const STACK::RawAddress &address);
    void AppSettingsSupportByAddr(AppSettingsCallback appCb, const STACK::RawAddress &address);
    void SetAppSettingsModeByAddr(uint8_t shuffleMode, uint8_t repeatMode, const STACK::RawAddress &address);
    void QueryAppSettingsModeByAddr(const STACK::RawAddress &address);
    void NotifyA2dpPlayStateChangeByAddr(const STACK::RawAddress &address);
#endif
    bool IsTopSessionInCast();

private:
    void SubscribeAvsessionSystemAbility();
    std::mutex avSessionControllerLock_;
    std::shared_ptr<AVSession::AVSessionController> avSessionController_{ nullptr };

#ifdef MULTI_A2DP_ENABLED
    std::weak_ptr<AvrcpMediaInterfaceImpl::impl> self_;
#endif
    std::shared_ptr<AVControllerObserverImpl> avControllerObserver_{ nullptr };
#ifdef MULTI_A2DP_ENABLED
    // Per-device observers: one per avSessionControllerMap_ entry, each carrying the
    // device MAC so media updates can be sent to the designated device.
    std::map<std::string, std::shared_ptr<AVControllerObserverImpl>> avControllerObserverMap_;
#endif
    std::shared_ptr<AVSessionObserverImpl> avSessionObserver_{ nullptr };
#ifdef AUDIO_FRAMEWORK
    std::shared_ptr<PlayHelper> playHelper_{ nullptr };
#endif
    std::mutex serviceCallbackLock_;
    MediaCallbacks *serviceCallback_ = nullptr;
    AppSettingsCallback appSettingsCallback_;

    IProfileA2dp *a2dpService_ = nullptr;
    IProfileAudioManager *audioManager_ = nullptr;
    std::string activeAddress_ = "00:00:00:00:00:00";
    std::mutex activeAddrLock_;
#ifdef MULTI_A2DP_ENABLED
    std::set<std::string> activeAddresses_;
    std::map<std::string, std::shared_ptr<AVSession::AVSessionController>> avSessionControllerMap_;
    std::map<std::string, AVSession::AVMetaData> currMateDataByAddr_;
    std::map<std::string, AVSession::AVPlaybackState> currPlaybackstateByAddr_;
    std::map<std::string, AppSettingsCallback> appSettingsCallbackMap_;
    std::map<std::string, std::string> lastPauseSessionIdByAddr_;
    bool CheckTrackStateChangedByAddr(const AVSession::AVMetaData &data,
        AVSession::AVMetaData &lastMetaData);
    bool CheckPlaybackStateChangedByAddr(const AVSession::AVPlaybackState &state,
        AVSession::AVPlaybackState &lastPlaybackState);
    bool CheckLoopModeChangedByAddr(const AVSession::AVPlaybackState &state,
        AVSession::AVPlaybackState &lastPlaybackState);
    void AddActiveDevice(const STACK::RawAddress &address) {
        std::lock_guard<std::mutex> lock(activeAddrLock_);
        activeAddresses_.insert(address.ToString());
    }
    void RemoveActiveDevice(const STACK::RawAddress &address) {
        {
            std::lock_guard<std::mutex> lock(activeAddrLock_);
            activeAddresses_.erase(address.ToString());
        }
        auto addr = address.ToString();
        auto wp = self_;
        std::shared_ptr<AVSession::AVSessionController> scheduledController;
        {
            std::lock_guard<std::mutex> lock(avSessionControllerLock_);
            auto it = avSessionControllerMap_.find(addr);
            if (it != avSessionControllerMap_.end()) {
                scheduledController = it->second;
            }
        }
        DoInAvrcpTgThread([wp, addr, scheduledController]() {
            auto ptr = wp.lock();
            if (ptr == nullptr) {
                return;
            }
            std::shared_ptr<AVSession::AVSessionController> controller;
            {
                std::lock_guard<std::mutex> lock(ptr->avSessionControllerLock_);
                auto it = ptr->avSessionControllerMap_.find(addr);
                if (it != ptr->avSessionControllerMap_.end() && it->second != scheduledController) {
                    HILOGI("controller changed for %{public}s, skip remove", GET_ENCRYPT_STR_ADDR(addr));
                    return;
                }
                if (it != ptr->avSessionControllerMap_.end()) {
                    controller = it->second;
                    if (ptr->avSessionController_ == controller) {
                        ptr->avSessionController_ = nullptr;
                    }
                    ptr->avSessionControllerMap_.erase(it);
                }
                ptr->avControllerObserverMap_.erase(addr);
                ptr->currMateDataByAddr_.erase(addr);
                ptr->currPlaybackstateByAddr_.erase(addr);
                ptr->lastPauseSessionIdByAddr_.erase(addr);
            }
            if (controller) {
                controller->Destroy();
            }
            std::lock_guard<std::mutex> cbLock(ptr->serviceCallbackLock_);
            ptr->appSettingsCallbackMap_.erase(addr);
        });
    }
    void SendKeyEventByAddr(uint8_t key, const STACK::RawAddress &address) {
        HITRACE_METER(BT_TRACE_TAG);
        auto iter = avrcpToAKeyEventCodeMap_.find(key);
        if (iter == avrcpToAKeyEventCodeMap_.end()) {
            HILOGW("don't deal key %{public}x.", key);
            return;
        }
        auto keyEvent = OHOS::MMI::KeyEvent::Create();
        if (keyEvent == nullptr) {
            HILOGE("KeyEvent::Create error.");
            return;
        }
        keyEvent->SetKeyCode(iter->second);
        keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
        auto keyItem = MMI::KeyEvent::KeyItem();
        keyItem.SetKeyCode(iter->second);
        keyItem.SetDownTime(1);
        keyItem.SetPressed(true);
        keyEvent->AddKeyItem(keyItem);
        AAFwk::Want addressParams;
        int32_t result;
        int32_t userId = Bluetooth::MultiA2dpManager::GetUserIdByAddr(address.ToString());
        if (userId < 0) {
            HILOGW("no userId for device %{public}s, skip send key", GET_ENCRYPT_STR_ADDR(address.ToString()));
            return;
        }
        addressParams.SetParam("userId", std::to_string(userId));
        result = AVSession::AVSessionManager::GetInstance().SendSystemAVKeyEvent(*keyEvent, addressParams);
        HILOGI("send avkey event userId %{public}d for device %{public}s, status %{public}d.", userId,
            GET_ENCRYPT_STR_ADDR(address.ToString()), result);
        if (result != AVSession::AVSESSION_SUCCESS) {
            HILOGE("excute SendSystemAVKeyEvent error");
        }
        if (key == AVRC_ID_PLAY && a2dpService_ != nullptr) {
            a2dpService_->NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION,
                ServiceUtil::AddrFromStack(address));
        }
    }
#endif
    sptr<SystemAbilitySubscriber> avsessionStatusChangeListener_ { nullptr };

    AVSession::AVMetaData currMateData_;
    AVSession::AVPlaybackState currPlaybackstate_;
    int32_t lastAudioManagerState_ = -1; // 记录上次的AVSession播放状态

    std::unordered_map<int32_t, int32_t> avrcpToAvSessionCommondMap_ = {
        { AVRC_ID_PLAY, AVSession::AVControlCommand::SESSION_CMD_PLAY },
        { AVRC_ID_STOP, AVSession::AVControlCommand::SESSION_CMD_STOP },
        { AVRC_ID_PAUSE, AVSession::AVControlCommand::SESSION_CMD_PAUSE },
        { AVRC_ID_FORWARD, AVSession::AVControlCommand::SESSION_CMD_PLAY_NEXT },
        { AVRC_ID_BACKWARD, AVSession::AVControlCommand::SESSION_CMD_PLAY_PREVIOUS },
        { AVRC_ID_REWIND, AVSession::AVControlCommand::SESSION_CMD_REWIND },
        { AVRC_ID_FAST_FOR, AVSession::AVControlCommand::SESSION_CMD_FAST_FORWARD }
    };
    std::unordered_map<int32_t, int32_t> avrcpToAKeyEventCodeMap_ = {
        { AVRC_ID_PLAY, MMI::KeyEvent::KEYCODE_MEDIA_PLAY },
        { AVRC_ID_STOP, MMI::KeyEvent::KEYCODE_MEDIA_STOP },
        { AVRC_ID_PAUSE, MMI::KeyEvent::KEYCODE_MEDIA_PAUSE },
        { AVRC_ID_FORWARD, MMI::KeyEvent::KEYCODE_MEDIA_NEXT },
        { AVRC_ID_BACKWARD, MMI::KeyEvent::KEYCODE_MEDIA_PREVIOUS },
        { AVRC_ID_REWIND, MMI::KeyEvent::KEYCODE_MEDIA_REWIND },
        { AVRC_ID_FAST_FOR, MMI::KeyEvent::KEYCODE_MEDIA_FAST_FORWARD }
    };
    std::string lastPauseSessionId_ = "";
};

void AvrcpMediaInterfaceImpl::impl::Init(std::weak_ptr<AvrcpMediaInterfaceImpl::impl> impl)
{
#ifdef MULTI_A2DP_ENABLED
    self_ = impl;
#endif
    avSessionObserver_ = std::make_shared<AVSessionObserverImpl>(impl);
    avControllerObserver_ = std::make_shared<AVControllerObserverImpl>(impl);
#ifdef AUDIO_FRAMEWORK
    playHelper_ = std::make_shared<PlayHelper>();
#endif
    RegisterAvSessionListener();
    AttemptCreateAvSessionController();
    SubscribeAvsessionSystemAbility();
}

void AvrcpMediaInterfaceImpl::impl::CleanUp()
{
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (avSessionController_) {
            avSessionController_->Destroy();
            avSessionController_ = nullptr;
        }
#ifdef MULTI_A2DP_ENABLED
        for (auto &entry : avSessionControllerMap_) {
            if (entry.second) {
                entry.second->Destroy();
            }
        }
        avSessionControllerMap_.clear();
        avControllerObserverMap_.clear();
        currMateDataByAddr_.clear();
        currPlaybackstateByAddr_.clear();
        lastPauseSessionIdByAddr_.clear();
#endif
    }
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        serviceCallback_ = nullptr;
#ifdef MULTI_A2DP_ENABLED
        appSettingsCallbackMap_.clear();
#endif
    }
}

void AvrcpMediaInterfaceImpl::impl::OnSessionRelease(const AVSession::AVSessionDescriptor &descriptor)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto sessionId = descriptor.sessionId_;
    HILOGI("sessionId:%{public}s", sessionId.c_str());
    std::lock_guard<std::mutex> lock(avSessionControllerLock_);
#ifdef MULTI_A2DP_ENABLED
    std::string releasedAddr = "";
    for (auto &entry : avSessionControllerMap_) {
        if (entry.second && entry.second->GetSessionId().compare(sessionId) == 0) {
            releasedAddr = entry.first;
            break;
        }
    }
    if (!releasedAddr.empty()) {
        auto it = avSessionControllerMap_.find(releasedAddr);
        if (it != avSessionControllerMap_.end() && it->second) {
            it->second->Destroy();
            avSessionControllerMap_.erase(it);
        }
        avControllerObserverMap_.erase(releasedAddr);
        currMateDataByAddr_.erase(releasedAddr);
        currPlaybackstateByAddr_.erase(releasedAddr);
        lastPauseSessionIdByAddr_.erase(releasedAddr);
        {
            std::lock_guard<std::mutex> cbLock(serviceCallbackLock_);
            appSettingsCallbackMap_.erase(releasedAddr);
            if (serviceCallback_ != nullptr) {
                serviceCallback_->SendMediaUpdateByAddress(true, false, false,
                    ServiceUtil::AddrToStack(RawAddress(releasedAddr)));
            }
        }
        HILOGI("per-device session released for %{public}s", GET_ENCRYPT_STR_ADDR(releasedAddr));
    }
#endif
    if (!avSessionController_) {
        return;
    }
    if (avSessionController_->GetSessionId().compare(sessionId) == 0) {
        avSessionController_->Destroy();
        avSessionController_ = nullptr;
        currPlaybackstate_.SetState(OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_STOP);
        currMateData_.Reset();
        {
            std::lock_guard<std::mutex> lock(serviceCallbackLock_);
            if (serviceCallback_ != nullptr) {
                serviceCallback_->SendMediaUpdate(true, false, false);
            }
        }
    }
}

void AvrcpMediaInterfaceImpl::impl::OnTopSessionChange(const AVSession::AVSessionDescriptor &descriptor)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto sessionId = descriptor.sessionId_;
    HILOGI("sessionId:%{public}s", sessionId.c_str());
    std::lock_guard<std::mutex> lock(avSessionControllerLock_);
#ifdef MULTI_A2DP_ENABLED
    {
        int32_t displayId = Bluetooth::MultiA2dpManager::GetDisplayIdByUserId(descriptor.userId_);
        std::string macAddress = (displayId >= 0) ? Bluetooth::MultiA2dpManager::GetAddrByDisplayId(displayId) : "";
        if (!macAddress.empty()) {
            CreateAvSessionControllerByAddr(sessionId, macAddress);
        } else {
            HILOGI("session userId %{public}d not bound to a multi-a2dp device, skip", descriptor.userId_);
        }
    }
    return;
#endif
    if (!avSessionController_) {
        CreateAvSessionController(sessionId);
        HILOGI("first create session controller sessionId:%{public}s", sessionId.c_str());
        return;
    }
    if (avSessionController_->GetSessionId().compare(sessionId) == 0) {
        HILOGI("same session, sessionId:%{public}s", sessionId.c_str());
        return;
    }

    avSessionController_->Destroy();
    avSessionController_ = nullptr;
    currPlaybackstate_.SetState(OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_STOP);
    CreateAvSessionController(sessionId);
    HILOGI("re create session controller sessionId:%{public}s", sessionId.c_str());
}

void AvrcpMediaInterfaceImpl::impl::OnMetaDataChange(const AVSession::AVMetaData &data)
{
    HITRACE_METER(BT_TRACE_TAG);
    bool trackChanged = CheckTrackStateChanged(data);
    if (!trackChanged) {
        HILOGD("MetaData not changed");
        return;
    }

    // storage image from avsession
    std::shared_ptr<AVSession::AVSessionPixelMap> innerPixelMap = data.GetMediaImage();
    std::shared_ptr<Media::PixelMap> pixelMap = AVSession::AVSessionPixelMapAdapter::ConvertFromInner(innerPixelMap);
    std::string handle = "";
    if (pixelMap != nullptr) {
        std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
        handle = AvrcpCoverArtStorage::GetInstance()->StoreImage(coverArt);
    }

    AVSession::AVPlaybackState state;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (!avSessionController_) {
            return;
        }
        int32_t ret = avSessionController_->GetAVPlaybackState(state);
        if (ret != AVSession::AVSESSION_SUCCESS) {
            HILOGE("GetAVPlaybackState error %{public}d.", ret);
            return;
        }
    }

    bool playStateChanged = CheckPlaybackStateChanged(state);
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        if (serviceCallback_ != nullptr && trackChanged) {
            HILOGI("track changed, send mediaUpdate to remote device");
            serviceCallback_->SendMediaUpdate(trackChanged, playStateChanged, false);
        }
    }
}

void AvrcpMediaInterfaceImpl::impl::OnPlaybackStateChange(const AVSession::AVPlaybackState &)
{
    HITRACE_METER(BT_TRACE_TAG);
    AVSession::AVPlaybackState state;
    AVSession::AVMetaData data;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (!avSessionController_) {
            return;
        }
        int32_t ret = avSessionController_->GetAVPlaybackState(state);
        if (ret != AVSession::AVSESSION_SUCCESS) {
            HILOGE("GetAVPlaybackState error %{public}d.", ret);
            return;
        }
        ret = avSessionController_->GetAVMetaData(data);
        if (ret != AVSession::AVSESSION_SUCCESS) {
            HILOGE("GetAVMetaData error %{public}d.", ret);
            return;
        }
    }

    bool trackChanged = CheckTrackStateChanged(data);
    bool playStateChanged = CheckPlaybackStateChanged(state);
    bool loopModeChanged = CheckLoopModeChanged(state);
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        if (serviceCallback_ != nullptr) {
            serviceCallback_->SendMediaUpdate(trackChanged, playStateChanged, false);
        }
    }
    if (loopModeChanged && appSettingsCallback_) {
        uint8_t shuffleMode;
        uint8_t repeatMode;
        ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
        appSettingsCallback_(shuffleMode, repeatMode);
    }
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "audioManager_ is null");
    int32_t currentState = state.GetState();
    if (currentState == OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY) {
        audioManager_->DealWithNewPlayingStatus();
    } else if (currentState == OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE &&
        lastAudioManagerState_ != OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE) {
        // 播放状态变更为pause时，处理pause
        audioManager_->DealWithPauseStatus();
    }
    lastAudioManagerState_ = currentState;
}

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::impl::OnMetaDataChangeByAddr(const AVSession::AVMetaData &data,
                                                           const std::string &macAddress)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::shared_ptr<AVSession::AVSessionController> controller;
    bool trackChanged = false;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(macAddress);
        if (it == avSessionControllerMap_.end() || !it->second) {
            HILOGW("no controller for %{public}s, skip metadata notify", GET_ENCRYPT_STR_ADDR(macAddress));
            return;
        }
        controller = it->second;
        trackChanged = CheckTrackStateChangedByAddr(data, currMateDataByAddr_[macAddress]);
    }
    if (!trackChanged) {
        HILOGD("MetaData not changed for %{public}s", GET_ENCRYPT_STR_ADDR(macAddress));
        return;
    }
    // storage image from avsession
    std::shared_ptr<AVSession::AVSessionPixelMap> innerPixelMap = data.GetMediaImage();
    std::shared_ptr<Media::PixelMap> pixelMap = AVSession::AVSessionPixelMapAdapter::ConvertFromInner(innerPixelMap);
    if (pixelMap != nullptr) {
        std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
        AvrcpCoverArtStorage::GetInstance()->StoreImage(coverArt);
    }
    AVSession::AVPlaybackState state;
    if (controller->GetAVPlaybackState(state) != AVSession::AVSESSION_SUCCESS) {
        HILOGE("GetAVPlaybackState error for %{public}s.", GET_ENCRYPT_STR_ADDR(macAddress));
        return;
    }
    bool playStateChanged = false;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        playStateChanged = CheckPlaybackStateChangedByAddr(state, currPlaybackstateByAddr_[macAddress]);
    }
    std::lock_guard<std::mutex> lock(serviceCallbackLock_);
    if (serviceCallback_ != nullptr) {
        serviceCallback_->SendMediaUpdateByAddress(true, playStateChanged, false,
            ServiceUtil::AddrToStack(RawAddress(macAddress)));
        HILOGI("per-device metadata notify sent to %{public}s", GET_ENCRYPT_STR_ADDR(macAddress));
    }
}

void AvrcpMediaInterfaceImpl::impl::NotifyLoopModeChangedByAddr(const std::string &macAddress,
                                                                const AVSession::AVPlaybackState &state)
{
    AppSettingsCallback appCb;
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        auto cbIt = appSettingsCallbackMap_.find(macAddress);
        if (cbIt != appSettingsCallbackMap_.end()) {
            appCb = cbIt->second;
        }
    }
    if (appCb) {
        uint8_t shuffleMode = SHUFFLE_OFF;
        uint8_t repeatMode = REPEAT_OFF;
        AVSession::AVPlaybackState stateCopy = state;
        ConvertAvSessionLoopModeToStack(stateCopy, shuffleMode, repeatMode);
        appCb(shuffleMode, repeatMode);
    }
}

void AvrcpMediaInterfaceImpl::impl::OnPlaybackStateChangeByAddr(const AVSession::AVPlaybackState &,
                                                                const std::string &macAddress)
{
    HITRACE_METER(BT_TRACE_TAG);
    AVSession::AVPlaybackState state;
    AVSession::AVMetaData data;
    bool trackChanged = false;
    bool playStateChanged = false;
    bool loopModeChanged = false;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(macAddress);
        if (it == avSessionControllerMap_.end() || !it->second) {
            HILOGW("no controller for %{public}s, skip playback notify", GET_ENCRYPT_STR_ADDR(macAddress));
            return;
        }
        if (it->second->GetAVPlaybackState(state) != AVSession::AVSESSION_SUCCESS) {
            HILOGE("GetAVPlaybackState error for %{public}s.", GET_ENCRYPT_STR_ADDR(macAddress));
            return;
        }
        if (it->second->GetAVMetaData(data) != AVSession::AVSESSION_SUCCESS) {
            HILOGE("GetAVMetaData error for %{public}s.", GET_ENCRYPT_STR_ADDR(macAddress));
            return;
        }
        trackChanged = CheckTrackStateChangedByAddr(data, currMateDataByAddr_[macAddress]);
        playStateChanged = CheckPlaybackStateChangedByAddr(state, currPlaybackstateByAddr_[macAddress]);
        loopModeChanged = CheckLoopModeChangedByAddr(state, currPlaybackstateByAddr_[macAddress]);
    }
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        if (serviceCallback_ != nullptr) {
            serviceCallback_->SendMediaUpdateByAddress(trackChanged, playStateChanged, false,
                ServiceUtil::AddrToStack(RawAddress(macAddress)));
            HILOGI("per-device playback notify for %{public}s", GET_ENCRYPT_STR_ADDR(macAddress));
        }
    }
    if (loopModeChanged) {
        NotifyLoopModeChangedByAddr(macAddress, state);
    }
    if (audioManager_ && state.GetState() == OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY) {
        audioManager_->DealWithNewPlayingStatus();
    }
}
#endif

void AvrcpMediaInterfaceImpl::impl::SendKeyEvent(uint8_t key, KeyState state)
{
    SendCtrlCommandToAvSession(key);
    NotifyMediaStackChanged(key);
}
void AvrcpMediaInterfaceImpl::impl::GetSongInfo(SongInfoCallback cb)
{
    SongInfo song;
    if (!GetCurrSongInfo(song)) {
        HILOGE("GetCurrSongInfo failed");
    }
    (cb)(song);
}
static bool IsNeedChangePlayState(int32_t avState, int a2dpPlayState, bool isInDenyList)
{
    return avState == OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY && a2dpPlayState
        == A2DP_NOT_PLAYING && isInDenyList;
}
void AvrcpMediaInterfaceImpl::impl::GetPlayStatus(PlayStatusCallback cb)
{
    PlayStatus status = {0, 0, PlayState::STOPPED};
    AVSession::AVPlaybackState state;
    AVSession::AVMetaData data;
    int64_t position = 0;
    bool valid = true;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (!avSessionController_) {
            valid = false;
        } else {
            avSessionController_->GetAVPlaybackState(state);
            avSessionController_->GetAVMetaData(data);
            if (state.GetState() == AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE) {
                position = state.GetPosition().elapsedTime_;
            } else {
                position = avSessionController_->GetRealPlaybackPosition();
            }
        }
    }
    if (!valid) {
        HILOGI_TIME_LIMIT(__func__, "GetPlayStatus: avsession not ok");
        (cb)(status);
        return;
    }
    if (a2dpService_ == nullptr) {
        HILOGE("a2dpService_ is nullptr");
        (cb)(status);
        return;
    }
    bool isInDenyList = a2dpService_->IsActiveDeviceInPlayToPauseDenyList();
    int a2dpPlayState = A2DP_IS_PLAYING;
    a2dpService_->GetPlayingState(a2dpService_->GetActiveSinkDevice(), a2dpPlayState);
    int32_t avState = state.GetState();
    if (IsNeedChangePlayState(avState, a2dpPlayState, isInDenyList)) {
        HILOGI("In black list, change state to pause");
        avState = OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE;
    }
    status.state = ConvertAvSessionStateToStack(avState);
    HILOGD("avsession state %{public}d. convert state %{public}d.", avState, status.state);
    status.duration = data.GetDuration();
    status.position = static_cast<uint32_t>(position);
    (cb)(status);
}
void AvrcpMediaInterfaceImpl::impl::GetNowPlayingList(NowPlayingCallback cb)
{
    std::vector<SongInfo> songs;
    std::string mediaId = "";
    SongInfo song;
    if (!GetCurrSongInfo(song)) {
        HILOGE("GetCurrSongInfo failed.");
    }
    mediaId = song.media_id;
    songs.emplace_back(song);

    (cb)(mediaId, std::move(songs));
}
void AvrcpMediaInterfaceImpl::impl::GetMediaPlayerList(MediaListCallback cb)
{
    std::vector<MediaPlayerInfo> players;
    MediaPlayerInfo player;
    player.id = 0;
    player.name = "Bluetooth Player";
    player.browsing_supported = false;
    players.emplace_back(player);
    HILOGI("players size: %{public}d.", players.size());
    (cb)(0, std::move(players));
}
void AvrcpMediaInterfaceImpl::impl::GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb)
{
    HILOGI("playerId: %{public}d. mediaId: %{public}s.", playerId, mediaId.c_str());
    std::vector<ListItem> listItems;
    (folderCb)(std::move(listItems));
}
void AvrcpMediaInterfaceImpl::impl::SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb)
{
    HILOGI("playerId: %{public}u.", playerId);
    (browseCb)(true, "", 0);
}
void AvrcpMediaInterfaceImpl::impl::RegisterUpdateCallback(MediaCallbacks *callback)
{
    std::lock_guard<std::mutex> lock(serviceCallbackLock_);
    serviceCallback_ = callback;
}
void AvrcpMediaInterfaceImpl::impl::UnregisterUpdateCallback(MediaCallbacks *callback)
{
    std::lock_guard<std::mutex> lock(serviceCallbackLock_);
    serviceCallback_ = nullptr;
}
void AvrcpMediaInterfaceImpl::impl::PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId)
{
    HILOGI("playerId: %{public}d. nowPlaying: %{public}d. mediaId: %{public}s.", playerId, nowPlaying,
           mediaId.c_str());
    std::lock_guard<std::mutex> lock(avSessionControllerLock_);
    if (!avSessionController_) {
        return;
    }
    std::vector<AVSession::AVQueueItem> items;
    int32_t ret = avSessionController_->GetAVQueueItems(items);
    if (ret != AVSession::AVSESSION_SUCCESS) {
        return;
    }
    for (const auto &item : items) {
        auto description = item.GetDescription();
        if (description->GetMediaId() == mediaId) {
            int32_t itemId = item.GetItemId();
            ret = avSessionController_->SkipToQueueItem(itemId);
            HILOGI("excute SkipToQueueItem item: %{public}d. status %{public}d.", itemId, ret);
            return;
        }
    }
}
void AvrcpMediaInterfaceImpl::impl::SetActiveDevice(const STACK::RawAddress &address)
{
    std::lock_guard<std::mutex> lock(activeAddrLock_);
    RawAddress rawAddr = ServiceUtil::AddrFromStack(address);
    if (activeAddress_ == rawAddr.GetAddress()) {
        HILOGI("the device is already active");
        return;
    }
    activeAddress_ = rawAddr.GetAddress();
}

void AvrcpMediaInterfaceImpl::impl::AppSettingsSupport(AppSettingsCallback appCb)
{
    appSettingsCallback_ = appCb;
}
void AvrcpMediaInterfaceImpl::impl::SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode)
{
    HILOGI("shuffleMode: %{public}d. repeatMode: %{public}u.", shuffleMode, repeatMode);
    int32_t musicMode = AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE;
    if ((shuffleMode == SHUFFLE_ALL_TRACKS) && (repeatMode == REPEAT_OFF || repeatMode == REPEAT_ALL_TRACKS)) {
        musicMode = AVSession::AVPlaybackState::LOOP_MODE_SHUFFLE;
    } else if ((shuffleMode == SHUFFLE_OFF) && (repeatMode == REPEAT_ALL_TRACKS)) {
        musicMode = AVSession::AVPlaybackState::LOOP_MODE_LIST;
    } else if (repeatMode == REPEAT_SINGLE_TRACK) {
        musicMode = AVSession::AVPlaybackState::LOOP_MODE_SINGLE;
    } else if (shuffleMode == SHUFFLE_QUERY) {
        QueryAppSettingsMode();
        BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, RawAddress(activeAddress_), SHUFFLE_QUERY,
            UE_COMMON_SCENE_CASE5);
        return;
    }
    BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, RawAddress(activeAddress_), musicMode,
        UE_COMMON_SCENE_CASE5);
    HILOGI("musicMode %{public}d.", musicMode);
    AVSession::AVControlCommand command;
    command.SetCommand(AVSession::AVControlCommand::SESSION_CMD_SET_TARGET_LOOP_MODE);
    command.SetLoopMode(musicMode);
    int32_t result = AVSession::AVSessionManager::GetInstance().SendSystemControlCommand(command);
    HILOGD("excute SendSystemControlCommand status %{public}d.", result);
}
void AvrcpMediaInterfaceImpl::impl::SetA2dpService(IProfileA2dp *a2dpService)
{
    a2dpService_ = a2dpService;
}
void AvrcpMediaInterfaceImpl::impl::SetAudioManager(IProfileAudioManager *audioManager)
{
    audioManager_ = audioManager;
}
void AvrcpMediaInterfaceImpl::impl::SendKeyEventByWearDetection(uint8_t key, KeyState state)
{
    HILOGI("key = %{public}x. state = %{public}d.", key, state);
    // key down do nothing
    if (state == KeyState::PUSHED) {
        return;
    }
    bool isNotNeedSendPauseToAvsession = (key == AVRC_ID_PAUSE && IsTopSessionInCast());
    if (isNotNeedSendPauseToAvsession) {
        HILOGI("topSession is in cast, not send pause to topSession");
        return;
    }
    // if avsession released
    std::lock_guard<std::mutex> lock(avSessionControllerLock_);
    if (!avSessionController_) {
        return;
    }
    // record last pause session
    if (key == AVRC_ID_PAUSE) {
        lastPauseSessionId_ = avSessionController_->GetSessionId();
    }
    if (key == AVRC_ID_PLAY && avSessionController_->GetSessionId().compare(lastPauseSessionId_) != 0) {
        HILOGI("topSession is not match with last pause session, not send play to topSession");
        return;
    }

    auto iter = avrcpToAKeyEventCodeMap_.find(key);
    if (iter == avrcpToAKeyEventCodeMap_.end()) {
        HILOGW("don't deal key %{public}x.", key);
        return;
    }
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        HILOGE("KeyEvent::Create error.");
        return;
    }
    keyEvent->SetKeyCode(iter->second);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    auto keyItem = MMI::KeyEvent::KeyItem();
    keyItem.SetKeyCode(iter->second);
    keyItem.SetDownTime(1);
    keyItem.SetPressed(true);
    keyEvent->AddKeyItem(keyItem);
    int32_t result = avSessionController_->SendAVKeyEvent(*keyEvent);
    HILOGI("send avkey event status %{public}d.", result);
    if (result != AVSession::AVSESSION_SUCCESS) {
        HILOGE("excute SendAVKeyEvent error");
    }
}

bool AvrcpMediaInterfaceImpl::impl::IsTopSessionInCast()
{
    std::string sessionId;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (!avSessionController_) {
            return false;
        }
        sessionId = avSessionController_->GetSessionId();
    }
    AVSession::AVSessionDescriptor topSessionDescriptor;
    auto res = AVSession::AVSessionManager::GetInstance().GetSessionDescriptorsBySessionId(
        sessionId, topSessionDescriptor);
    bool isDescriptorIllegal = 
        (res != AVSession::AVSESSION_SUCCESS || topSessionDescriptor.outputDeviceInfo_.deviceInfos_.empty());
    if (isDescriptorIllegal) {
        HILOGE("Descriptor is invalid or empty");
        return false;
    }
    return topSessionDescriptor.outputDeviceInfo_.deviceInfos_[0].castCategory_ ==
        AVSession::AVCastCategory::CATEGORY_REMOTE;
}

void AvrcpMediaInterfaceImpl::impl::RegisterAvSessionListener()
{
    auto res = AVSession::AVSessionManager::GetInstance().RegisterSessionListener(avSessionObserver_);
    if (res != AVSession::AVSESSION_SUCCESS) {
        HILOGE("RegisterSessionListener fail");
    }
}

#ifdef MULTI_A2DP_ENABLED
// Caller must hold avSessionControllerLock_.
bool AvrcpMediaInterfaceImpl::impl::CreateAvSessionControllerByAddr(const std::string &sessionId,
    const std::string &macAddress)
{
    auto oldControllerIt = avSessionControllerMap_.find(macAddress);
    if (oldControllerIt != avSessionControllerMap_.end() && oldControllerIt->second) {
        if (oldControllerIt->second->GetSessionId().compare(sessionId) == 0) {
            return true;
        }
    }
    std::shared_ptr<AVSession::AVSessionController> controller;
    auto ret = AVSession::AVSessionManager::GetInstance().CreateController(sessionId, controller);
    if ((ret != AVSession::AVSESSION_SUCCESS && ret != AVSession::ERR_CONTROLLER_IS_EXIST) ||
        controller == nullptr) {
        HILOGE("CreateController fail for %{public}s", sessionId.c_str());
        return false;
    }
    // Per-device observer carries the MAC so media updates are
    // sent to this device only (SendMediaUpdateByAddress).
    auto devObserver = std::make_shared<AVControllerObserverImpl>(self_, macAddress);
    controller->RegisterCallback(devObserver);
    SetAvsessionFilterForController(controller);
    if (oldControllerIt != avSessionControllerMap_.end() && oldControllerIt->second) {
        oldControllerIt->second->Destroy();
    }
    avControllerObserverMap_[macAddress] = devObserver;
    avSessionControllerMap_[macAddress] = controller;
    HILOGI("per-device controller created for %{public}s", GET_ENCRYPT_STR_ADDR(macAddress));
    return true;
}

bool AvrcpMediaInterfaceImpl::impl::IsTopSessionInCastByAddr(const std::string &macAddress)
{
    std::string sessionId;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(macAddress);
        if (it == avSessionControllerMap_.end() || !it->second) {
            return false;
        }
        sessionId = it->second->GetSessionId();
    }
    AVSession::AVSessionDescriptor topSessionDescriptor;
    auto res = AVSession::AVSessionManager::GetInstance().GetSessionDescriptorsBySessionId(
        sessionId, topSessionDescriptor);
    bool isDescriptorIllegal =
        (res != AVSession::AVSESSION_SUCCESS || topSessionDescriptor.outputDeviceInfo_.deviceInfos_.empty());
    if (isDescriptorIllegal) {
        HILOGE("Descriptor is invalid or empty");
        return false;
    }
    return topSessionDescriptor.outputDeviceInfo_.deviceInfos_[0].castCategory_ ==
        AVSession::AVCastCategory::CATEGORY_REMOTE;
}

bool AvrcpMediaInterfaceImpl::impl::GetAvsessionControllerByAddr(const std::string &macAddress, uint8_t key,
    std::shared_ptr<AVSession::AVSessionController> &controller)
{
    std::lock_guard<std::mutex> lock(avSessionControllerLock_);
    auto it = avSessionControllerMap_.find(macAddress);
    if (it == avSessionControllerMap_.end() || !it->second) {
        return false;
    }
    controller = it->second;
    // record last pause session
    if (key == AVRC_ID_PAUSE) {
        lastPauseSessionIdByAddr_[macAddress] = controller->GetSessionId();
    }
    auto lastPauseIt = lastPauseSessionIdByAddr_.find(macAddress);
    if (key == AVRC_ID_PLAY && (lastPauseIt == lastPauseSessionIdByAddr_.end() ||
        controller->GetSessionId().compare(lastPauseIt->second) != 0)) {
        HILOGI("topSession is not match with last pause session, not send play to topSession");
        return false;
    }
    return true;
}

void AvrcpMediaInterfaceImpl::impl::SendKeyEventByWearDetectionByAddr(uint8_t key, KeyState state,
    const STACK::RawAddress &address)
{
    HILOGI("key = %{public}x. state = %{public}d.", key, state);
    // key down do nothing
    if (state == KeyState::PUSHED) {
        return;
    }
    std::string macAddress = address.ToString();
    bool isNotNeedSendPauseToAvsession = (key == AVRC_ID_PAUSE && IsTopSessionInCastByAddr(macAddress));
    if (isNotNeedSendPauseToAvsession) {
        HILOGI("topSession is in cast, not send pause to topSession");
        return;
    }
    // if avsession released
    std::shared_ptr<AVSession::AVSessionController> controller;
    if (!GetAvsessionControllerByAddr(macAddress, key, controller)) {
        return;
    }

    auto iter = avrcpToAKeyEventCodeMap_.find(key);
    if (iter == avrcpToAKeyEventCodeMap_.end()) {
        HILOGW("don't deal key %{public}x.", key);
        return;
    }
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        HILOGE("KeyEvent::Create error.");
        return;
    }
    keyEvent->SetKeyCode(iter->second);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    auto keyItem = MMI::KeyEvent::KeyItem();
    keyItem.SetKeyCode(iter->second);
    keyItem.SetDownTime(1);
    keyItem.SetPressed(true);
    keyEvent->AddKeyItem(keyItem);
    int32_t result = controller->SendAVKeyEvent(*keyEvent);
    HILOGI("send avkey event status %{public}d.", result);
    if (result != AVSession::AVSESSION_SUCCESS) {
        HILOGE("excute SendAVKeyEvent error");
    }
}

void AvrcpMediaInterfaceImpl::impl::PlaySilenceSoundByAddr(const std::string &macAddress)
{
    AVSession::AVPlaybackState state {};
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(macAddress);
        if (it != avSessionControllerMap_.end() && it->second) {
            it->second->GetAVPlaybackState(state);
        }
    }

    if (state.GetState() == AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY) {
        return;
    }

#ifdef AUDIO_FRAMEWORK
    CHECK_AND_RETURN_LOG(playHelper_, "playHelper_ is null");
    playHelper_->PlaySilenceSound();
#endif
}
#endif

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::impl::TryBindSessionToMultiDevice(int32_t userId, const std::string &sessionId)
{
        int32_t displayId = Bluetooth::MultiA2dpManager::GetDisplayIdByUserId(userId);
        std::string macAddress = (displayId >= 0) ?
            Bluetooth::MultiA2dpManager::GetAddrByDisplayId(displayId) : "";
        if (!macAddress.empty()) {
            std::lock_guard<std::mutex> lock(avSessionControllerLock_);
            if (CreateAvSessionControllerByAddr(sessionId, macAddress)) {
                HILOGW("CreateAvSessionControllerByAddr ok for %{public}s", GET_ENCRYPT_STR_ADDR(macAddress));
            }
        } else {
            HILOGI("session userId %{public}d not bound to a multi-a2dp device, skip",
                userId);
        }
}
#endif

void AvrcpMediaInterfaceImpl::impl::AttemptCreateAvSessionController()
{
    std::vector<AVSession::AVSessionDescriptor> avSessionDescriptor{};
    auto res = AVSession::AVSessionManager::GetInstance().GetAllSessionDescriptors(avSessionDescriptor);
    if (res != AVSession::AVSESSION_SUCCESS) {
        HILOGE("GetAllSessionDescriptors fail");
        return;
    }

    // if session created, find top session to create controller
    std::string sessionId = "";
    for (auto descriptor : avSessionDescriptor) {
        HILOGI("descriptor list sessionId_:%{public}s, sessionType_:%{public}d, sessionTag_:%{public}s,"
            "isTopSession_:%{public}d.", descriptor.sessionId_.c_str(), descriptor.sessionType_,
            descriptor.sessionTag_.c_str(), descriptor.isTopSession_);
        if (descriptor.isTopSession_) {
            sessionId = descriptor.sessionId_;
#ifdef MULTI_A2DP_ENABLED
            TryBindSessionToMultiDevice(descriptor.userId_, sessionId);
#else
            CreateAvSessionController(sessionId);
            HILOGW("CreateAvSessionController ok.");
            break;
#endif
        }
    }
}

void AvrcpMediaInterfaceImpl::impl::CreateAvSessionController(const std::string &sessionId)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto ret = AVSession::AVSessionManager::GetInstance().CreateController(sessionId, avSessionController_);
    if ((ret != AVSession::AVSESSION_SUCCESS && ret != AVSession::ERR_CONTROLLER_IS_EXIST) ||
            avSessionController_ == nullptr) {
        HILOGE("AvSession CreateController failed, ret(%{public}d)", ret);
        return;
    }
    avSessionController_->RegisterCallback(avControllerObserver_);
    SetAvsessionFilter();

    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        if (serviceCallback_ != nullptr) {
            HILOGI("CreateAvSessionController:SendFolderUpdate %{public}s", sessionId.c_str());
            serviceCallback_->SendFolderUpdate(true, true, false);
        }
    }
 
    AVSession::AVPlaybackState state;
    AVSession::AVMetaData data;
    bool playStateChanged = false;
    bool trackChanged = false;
    ret = avSessionController_->GetAVPlaybackState(state);
    if (ret == AVSession::AVSESSION_SUCCESS) {
        playStateChanged = CheckPlaybackStateChanged(state);
    }
    ret = avSessionController_->GetAVMetaData(data);
    if (ret == AVSession::AVSESSION_SUCCESS) {
        trackChanged = CheckTrackStateChanged(data);
    }
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        if (serviceCallback_ != nullptr) {
            HILOGI("CreateAvSessionController:SendMediaUpdate %{public}s", sessionId.c_str());
            serviceCallback_->SendMediaUpdate(trackChanged, playStateChanged, false);
        }
    }
}

void AvrcpMediaInterfaceImpl::impl::SetAvsessionFilter()
{
    AVSession::AVPlaybackState::PlaybackStateMaskType filter;
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_STATE);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_SPEED);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_POSITION);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_BUFFERED_TIME);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_LOOP_MODE);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_ACTIVE_ITEM_ID);
    avSessionController_->SetPlaybackFilter(filter);
    AVSession::AVMetaData::MetaMaskType metaFilter;
    metaFilter.set(AVSession::AVMetaData::META_KEY_ASSET_ID);
    metaFilter.set(AVSession::AVMetaData::META_KEY_ARTIST);
    metaFilter.set(AVSession::AVMetaData::META_KEY_ALBUM);
    metaFilter.set(AVSession::AVMetaData::META_KEY_TITLE);
    metaFilter.set(AVSession::AVMetaData::META_KEY_SINGLE_LYRIC_TEXT);
    metaFilter.set(AVSession::AVMetaData::META_KEY_MEDIA_IMAGE);
    avSessionController_->SetMetaFilter(metaFilter);
}

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::impl::SetAvsessionFilterForController(
    const std::shared_ptr<AVSession::AVSessionController> &controller)
{
    AVSession::AVPlaybackState::PlaybackStateMaskType filter;
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_STATE);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_SPEED);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_POSITION);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_BUFFERED_TIME);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_LOOP_MODE);
    filter.set(AVSession::AVPlaybackState::PLAYBACK_KEY_ACTIVE_ITEM_ID);
    controller->SetPlaybackFilter(filter);
    AVSession::AVMetaData::MetaMaskType metaFilter;
    metaFilter.set(AVSession::AVMetaData::META_KEY_ASSET_ID);
    metaFilter.set(AVSession::AVMetaData::META_KEY_ARTIST);
    metaFilter.set(AVSession::AVMetaData::META_KEY_ALBUM);
    metaFilter.set(AVSession::AVMetaData::META_KEY_TITLE);
    metaFilter.set(AVSession::AVMetaData::META_KEY_SINGLE_LYRIC_TEXT);
    metaFilter.set(AVSession::AVMetaData::META_KEY_MEDIA_IMAGE);
    controller->SetMetaFilter(metaFilter);
}
#endif

void AvrcpMediaInterfaceImpl::impl::SendCtrlCommandToAvSession(uint8_t key)
{
    SendKeyEventToAvSession(key);
}

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::impl::SendCtrlCommandToAvSession(uint8_t key, const RawAddress &address)
{
    SendKeyEventByAddr(key, ServiceUtil::AddrToStack(address));
}
#endif

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::impl::GetSongInfoByAddr(SongInfoCallback cb, const STACK::RawAddress &address)
{
    SongInfo song;
    if (!GetCurrSongInfoByAddr(song, address)) {
        HILOGE("GetCurrSongInfoByAddr failed");
    }
    cb(song);
}

void AvrcpMediaInterfaceImpl::impl::GetPlayStatusByAddr(PlayStatusCallback cb,
                                                        const STACK::RawAddress &address)
{
    PlayStatus status = {0, 0, PlayState::STOPPED};
    AVSession::AVPlaybackState state;
    AVSession::AVMetaData data;
    int64_t position = 0;
    bool valid = true;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(address.ToString());
        if (it == avSessionControllerMap_.end() || !it->second) {
            valid = false;
        } else {
            it->second->GetAVPlaybackState(state);
            it->second->GetAVMetaData(data);
            if (state.GetState() == AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE) {
                position = state.GetPosition().elapsedTime_;
            } else {
                position = it->second->GetRealPlaybackPosition();
            }
        }
    }
    if (!valid) {
        HILOGI_TIME_LIMIT(__func__, "GetPlayStatusByAddr: no controller for the device");
        cb(status);
        return;
    }
    if (a2dpService_ == nullptr) {
        HILOGE("a2dpService_ is nullptr");
        cb(status);
        return;
    }
    RawAddress rawAddr = ServiceUtil::AddrFromStack(address);
    bool isInDenyList = rawAddr.GetAddress() == a2dpService_->GetActiveSinkDevice().GetAddress()
        && a2dpService_->IsActiveDeviceInPlayToPauseDenyList();
    int a2dpPlayState = A2DP_IS_PLAYING;
    a2dpService_->GetPlayingState(rawAddr, a2dpPlayState);
    int32_t avState = state.GetState();
    if (IsNeedChangePlayState(avState, a2dpPlayState, isInDenyList)) {
        HILOGI("In black list, change state to pause");
        avState = OHOS::AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE;
    }
    status.state = ConvertAvSessionStateToStack(avState);
    HILOGD("avsession state %{public}d. convert state %{public}d.", avState, status.state);
    status.duration = data.GetDuration();
    status.position = static_cast<uint32_t>(position);
    cb(status);
}

void AvrcpMediaInterfaceImpl::impl::GetNowPlayingListByAddr(NowPlayingCallback cb,
                                                            const STACK::RawAddress &address)
{
    std::vector<SongInfo> songs;
    std::string mediaId = "";
    SongInfo song;
    if (!GetCurrSongInfoByAddr(song, address)) {
        HILOGE("GetCurrSongInfoByAddr failed.");
    }
    mediaId = song.media_id;
    songs.emplace_back(song);

    cb(mediaId, std::move(songs));
}

void AvrcpMediaInterfaceImpl::impl::GetMediaPlayerListByAddr(MediaListCallback cb,
                                                             const STACK::RawAddress &address)
{
    std::vector<MediaPlayerInfo> players;
    MediaPlayerInfo player;
    player.id = 0;
    player.name = "Bluetooth Player";
    player.browsing_supported = false;
    players.emplace_back(player);
    HILOGI("players size: %{public}d.", players.size());
    cb(0, std::move(players));
}

void AvrcpMediaInterfaceImpl::impl::GetFolderItemsByAddr(uint16_t playerId, std::string mediaId,
                                                         FolderItemsCallback folderCb,
                                                         const STACK::RawAddress &address)
{
    HILOGI("playerId: %{public}d. mediaId: %{public}s. device: %{public}s.", playerId, mediaId.c_str(),
           GET_ENCRYPT_STR_ADDR(address.ToString()));
    std::vector<ListItem> listItems;
    folderCb(std::move(listItems));
}

void AvrcpMediaInterfaceImpl::impl::SetBrowsedPlayerByAddr(uint16_t playerId, SetBrowsedPlayerCallback browseCb,
                                                           const STACK::RawAddress &address)
{
    HILOGI("playerId: %{public}u. device: %{public}s.", playerId, GET_ENCRYPT_STR_ADDR(address.ToString()));
    browseCb(true, "", 0);
}

void AvrcpMediaInterfaceImpl::impl::PlayItemByAddr(uint16_t playerId, bool nowPlaying, std::string mediaId,
                                                   const STACK::RawAddress &address)
{
    HILOGI("playerId: %{public}d. nowPlaying: %{public}d. mediaId: %{public}s.", playerId, nowPlaying,
           mediaId.c_str());
    std::shared_ptr<AVSession::AVSessionController> controller;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(address.ToString());
        if (it == avSessionControllerMap_.end() || !it->second) {
            HILOGW("no controller for %{public}s, skip play item", GET_ENCRYPT_STR_ADDR(address.ToString()));
            return;
        }
        controller = it->second;
    }
    std::vector<AVSession::AVQueueItem> items;
    int32_t ret = controller->GetAVQueueItems(items);
    if (ret != AVSession::AVSESSION_SUCCESS) {
        return;
    }
    for (const auto &item : items) {
        auto description = item.GetDescription();
        if (description->GetMediaId() == mediaId) {
            int32_t itemId = item.GetItemId();
            ret = controller->SkipToQueueItem(itemId);
            HILOGI("excute SkipToQueueItem item: %{public}d. status %{public}d.", itemId, ret);
            return;
        }
    }
}

bool AvrcpMediaInterfaceImpl::impl::GetCurrSongInfoByAddr(SongInfo &song, const STACK::RawAddress &address)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::shared_ptr<AVSession::AVSessionController> controller;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(address.ToString());
        if (it == avSessionControllerMap_.end() || !it->second) {
            HILOGW("no controller for %{public}s, skip get song info", GET_ENCRYPT_STR_ADDR(address.ToString()));
            return false;
        }
        controller = it->second;
    }
    OHOS::AVSession::AVMetaData data;
    int32_t ret = controller->GetAVMetaData(data);
    if (ret != OHOS::AVSession::AVSESSION_SUCCESS) {
        HILOGE("GetAVMetaData error %{public}d.", ret);
        return false;
    }
    return ConvertMetaDataToSongInfo(data, song);
}

void AvrcpMediaInterfaceImpl::impl::AppSettingsSupportByAddr(AppSettingsCallback appCb,
                                                             const STACK::RawAddress &address)
{
    std::lock_guard<std::mutex> lock(serviceCallbackLock_);
    appSettingsCallbackMap_[address.ToString()] = appCb;
}

void AvrcpMediaInterfaceImpl::impl::QueryAppSettingsModeByAddr(const STACK::RawAddress &address)
{
    HITRACE_METER(BT_TRACE_TAG);
    AppSettingsCallback appCb;
    {
        std::lock_guard<std::mutex> lock(serviceCallbackLock_);
        auto cbIt = appSettingsCallbackMap_.find(address.ToString());
        if (cbIt == appSettingsCallbackMap_.end() || !cbIt->second) {
            HILOGE("no app settings callback for %{public}s", GET_ENCRYPT_STR_ADDR(address.ToString()));
            return;
        }
        appCb = cbIt->second;
    }
    uint8_t shuffleMode = SHUFFLE_OFF;
    uint8_t repeatMode = REPEAT_OFF;
    AVSession::AVPlaybackState state;
    bool valid = true;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(address.ToString());
        if (it == avSessionControllerMap_.end() || !it->second) {
            valid = false;
        } else if (it->second->GetAVPlaybackState(state) != AVSession::AVSESSION_SUCCESS) {
            valid = false;
        }
    }
    if (!valid) {
        appCb(shuffleMode, repeatMode);
        return;
    }
    ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
    appCb(shuffleMode, repeatMode);
}

void AvrcpMediaInterfaceImpl::impl::SetAppSettingsModeByAddr(uint8_t shuffleMode, uint8_t repeatMode,
                                                             const STACK::RawAddress &address)
{
    HILOGI("shuffleMode: %{public}d. repeatMode: %{public}u.", shuffleMode, repeatMode);
    RawAddress rawAddr = ServiceUtil::AddrFromStack(address);
    int32_t musicMode = AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE;
    if ((shuffleMode == SHUFFLE_ALL_TRACKS) && (repeatMode == REPEAT_OFF || repeatMode == REPEAT_ALL_TRACKS)) {
        musicMode = AVSession::AVPlaybackState::LOOP_MODE_SHUFFLE;
    } else if ((shuffleMode == SHUFFLE_OFF) && (repeatMode == REPEAT_ALL_TRACKS)) {
        musicMode = AVSession::AVPlaybackState::LOOP_MODE_LIST;
    } else if (repeatMode == REPEAT_SINGLE_TRACK) {
        musicMode = AVSession::AVPlaybackState::LOOP_MODE_SINGLE;
    } else if (shuffleMode == SHUFFLE_QUERY) {
        QueryAppSettingsModeByAddr(address);
        BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, rawAddr, SHUFFLE_QUERY,
            UE_COMMON_SCENE_CASE5);
        return;
    }
    BtChrUeManager::GetInstance()->WriteCommandUe(CHR_UE_SEND_MEDIA_CMD, rawAddr, musicMode,
        UE_COMMON_SCENE_CASE5);
    HILOGI("musicMode %{public}d.", musicMode);
    std::shared_ptr<AVSession::AVSessionController> controller;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        auto it = avSessionControllerMap_.find(address.ToString());
        if (it != avSessionControllerMap_.end() && it->second) {
            controller = it->second;
        }
    }
    AVSession::AVControlCommand command;
    command.SetCommand(AVSession::AVControlCommand::SESSION_CMD_SET_TARGET_LOOP_MODE);
    command.SetLoopMode(musicMode);
    int32_t result = -1;
    if (controller != nullptr) {
        result = controller->SendControlCommand(command);
        HILOGI("excute controller SendControlCommand status %{public}d.", result);
    }
    if (controller == nullptr || result != AVSession::AVSESSION_SUCCESS) {
        result = AVSession::AVSessionManager::GetInstance().SendSystemControlCommand(command);
        HILOGW("fallback SendSystemControlCommand status %{public}d.", result);
    }
}
#endif

void AvrcpMediaInterfaceImpl::impl::PlaySilenceSound()
{
    AVSession::AVPlaybackState state {};
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (avSessionController_) {
            avSessionController_->GetAVPlaybackState(state);
        }
    }

    if (state.GetState() == AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY) {
        return;
    }

#ifdef AUDIO_FRAMEWORK
    CHECK_AND_RETURN_LOG(playHelper_, "playHelper_ is null");
    playHelper_->PlaySilenceSound();
#endif
}

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::impl::NotifyA2dpPlayStateChangeByAddr(const STACK::RawAddress &address)
{
    std::lock_guard<std::mutex> lock(serviceCallbackLock_);
    if (serviceCallback_ != nullptr) {
        serviceCallback_->SendMediaUpdateByAddress(false, true, false, address);
    }
}
#endif

void AvrcpMediaInterfaceImpl::impl::NotifyA2dpPlayStateChange()
{
    std::lock_guard<std::mutex> lock(serviceCallbackLock_);
    if (serviceCallback_ != nullptr) {
        serviceCallback_->SendMediaUpdate(false, true, false); // 根据流是否起播，通知媒体信息是否更新
    }
}

void AvrcpMediaInterfaceImpl::impl::HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    auto bipService = BipService::GetInstance();
    bipService->HandlerConnectEvt(socketDevice);
}

void AvrcpMediaInterfaceImpl::impl::NotifyMediaStackChanged(uint8_t key)
{
    if (key != AVRC_ID_PLAY) {
        HILOGD("Ignore, not play command");
        return;
    }
    if (a2dpService_ == nullptr) {
        HILOGE("Can't get the instance of A2dpService");
        return;
    }
    a2dpService_->NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, RawAddress(activeAddress_));
}

bool AvrcpMediaInterfaceImpl::impl::GetCurrSongInfo(SongInfo &song)
{
    HITRACE_METER(BT_TRACE_TAG);
    std::lock_guard<std::mutex> lock(avSessionControllerLock_);
    if (!avSessionController_) {
        return false;
    }
    OHOS::AVSession::AVMetaData data;
    int32_t ret = avSessionController_->GetAVMetaData(data);
    if (ret != OHOS::AVSession::AVSESSION_SUCCESS) {
        HILOGE("GetAVMetaData error %{public}d.", ret);
        return false;
    }
    HILOGD("MediaId %{public}s,Title %{public}s,Artist %{public}s,Album %{public}s,Time %{public}s,lyric %{public}s",
           data.GetAssetId().c_str(), data.GetTitle().c_str(), data.GetArtist().c_str(), data.GetAlbum().c_str(),
           std::to_string(data.GetDuration()).c_str(), data.GetSingleLyricText().c_str());
    return ConvertMetaDataToSongInfo(data, song);
}

bool AvrcpMediaInterfaceImpl::impl::ConvertMetaDataToSongInfo(const AVSession::AVMetaData &data, SongInfo &song)
{
    // storage image from avsession
    std::shared_ptr<AVSession::AVSessionPixelMap> innerPixelMap = data.GetMediaImage();
    std::shared_ptr<Media::PixelMap> pixelMap = AVSession::AVSessionPixelMapAdapter::ConvertFromInner(innerPixelMap);
    std::string handle = "";
    if (pixelMap != nullptr) {
        std::shared_ptr<CoverArt> coverArt = std::make_shared<CoverArt>(pixelMap);
        handle = AvrcpCoverArtStorage::GetInstance()->StoreImage(coverArt);
    }
    song.media_id = data.GetAssetId();
    song.attributes.clear();
    if (data.GetSingleLyricText() != "") {
        song.attributes.insert(
            AttributeEntry(Attribute::TITLE, TruncateString(data.GetSingleLyricText(), MAX_TITLE_LENGTH)));
        std::string titleAndArtist = data.GetTitle() + " - " + data.GetArtist();
        song.attributes.insert(AttributeEntry(Attribute::ARTIST_NAME, titleAndArtist));
    } else {
        song.attributes.insert(AttributeEntry(Attribute::TITLE, TruncateString(data.GetTitle(), MAX_TITLE_LENGTH)));
        song.attributes.insert(AttributeEntry(Attribute::ARTIST_NAME, data.GetArtist()));
    }
    song.attributes.insert(AttributeEntry(Attribute::ALBUM_NAME, data.GetAlbum()));
    song.attributes.insert(AttributeEntry(Attribute::TRACK_NUMBER, std::string("1")));
    song.attributes.insert(AttributeEntry(Attribute::TOTAL_NUMBER_OF_TRACKS, std::string("1")));
    song.attributes.insert(AttributeEntry(Attribute::GENRE, std::string("")));
    song.attributes.insert(AttributeEntry(Attribute::PLAYING_TIME, std::to_string(data.GetDuration())));
    if (!handle.empty()) {
        song.attributes.insert(AttributeEntry(Attribute::DEFAULT_COVER_ART, handle));
    }
    return true;
}

PlayState AvrcpMediaInterfaceImpl::impl::ConvertAvSessionStateToStack(int32_t state)
{
    PlayState ret = PlayState::ERROR;
    switch (state) {
        case AVSession::AVPlaybackState::PLAYBACK_STATE_PREPARE:
        case AVSession::AVPlaybackState::PLAYBACK_STATE_MAX:
            break;
        case AVSession::AVPlaybackState::PLAYBACK_STATE_PLAY:
        case AVSession::AVPlaybackState::PLAYBACK_STATE_BUFFERING:
            ret = PlayState::PLAYING;
            break;
        case AVSession::AVPlaybackState::PLAYBACK_STATE_PAUSE:
            ret = PlayState::PAUSED;
            break;
        case AVSession::AVPlaybackState::PLAYBACK_STATE_FAST_FORWARD:
            ret = PlayState::FWD_SEEK;
            break;
        case AVSession::AVPlaybackState::PLAYBACK_STATE_REWIND:
            ret = PlayState::REV_SEEK;
            break;
        case AVSession::AVPlaybackState::PLAYBACK_STATE_STOP:
        case AVSession::AVPlaybackState::PLAYBACK_STATE_INITIAL:
            ret = PlayState::STOPPED;
            break;
        default:
            break;
    }
    return ret;
}

void AvrcpMediaInterfaceImpl::impl::QueryAppSettingsMode()
{
    HITRACE_METER(BT_TRACE_TAG);
    if (!appSettingsCallback_) {
        HILOGE("appSettingsCallback in null");
        return;
    }
    uint8_t shuffleMode = SHUFFLE_OFF;
    uint8_t repeatMode = REPEAT_OFF;
    AVSession::AVPlaybackState state;
    bool vaild = true;
    {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        if (!avSessionController_) {
            HILOGE("avsession controller wasn't prepared");
            vaild = false;
        } else {
            int32_t ret = avSessionController_->GetAVPlaybackState(state);
            HILOGI("GetAVPlaybackState avstatus: %{public}d, ret:%{public}d", state.GetState(), ret);
            if (ret != AVSession::AVSESSION_SUCCESS) {
                vaild = false;
            }
        }
    }
    if (!vaild) {
        appSettingsCallback_(shuffleMode, repeatMode);
        return;
    }
    ConvertAvSessionLoopModeToStack(state, shuffleMode, repeatMode);
    appSettingsCallback_(shuffleMode, repeatMode);
}

void AvrcpMediaInterfaceImpl::impl::ConvertAvSessionLoopModeToStack(AVSession::AVPlaybackState &state,
                                                                    uint8_t &shuffleMode, uint8_t &repeatMode)
{
    switch (state.GetLoopMode()) {
        case AVSession::AVPlaybackState::LOOP_MODE_SEQUENCE: {
            shuffleMode = SHUFFLE_OFF;
            repeatMode = REPEAT_OFF;
            break;
        }
        case AVSession::AVPlaybackState::LOOP_MODE_SINGLE: {
            shuffleMode = SHUFFLE_OFF;
            repeatMode = REPEAT_SINGLE_TRACK;
            break;
        }
        case AVSession::AVPlaybackState::LOOP_MODE_LIST: {
            shuffleMode = SHUFFLE_OFF;
            repeatMode = REPEAT_ALL_TRACKS;
            break;
        }
        case AVSession::AVPlaybackState::LOOP_MODE_SHUFFLE: {
            shuffleMode = SHUFFLE_ALL_TRACKS;
            repeatMode = REPEAT_ALL_TRACKS;
            break;
        }
        default:
            break;
    }
}

bool AvrcpMediaInterfaceImpl::impl::CheckTrackStateChanged(const AVSession::AVMetaData &data)
{
    bool isAssetIdChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_ASSET_ID) &&
        currMateData_.GetAssetId() != data.GetAssetId();
    bool isArtistChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_ARTIST) &&
        currMateData_.GetArtist() != data.GetArtist();
    bool isAlbumChange = currMateData_.GetAlbum() != data.GetAlbum();
    bool isTitleChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_TITLE) &&
        currMateData_.GetTitle() != data.GetTitle();
    bool isLyricChange = currMateData_.GetSingleLyricText() != data.GetSingleLyricText(); //存在用户手动关闭蓝牙歌词开关，该字段变为空的情况。
    bool isMediaImageChange = false;
    auto currImg = currMateData_.GetMediaImage();
    auto newImg = data.GetMediaImage();
    if (currImg != nullptr && newImg != nullptr) {
        isMediaImageChange = !currImg->Equals(*newImg);
    } else {
        isMediaImageChange = (currImg != nullptr) != (newImg != nullptr);
    }

    bool isNeedSendChange = isAssetIdChange || isArtistChange || isAlbumChange || isTitleChange
        || isLyricChange || isMediaImageChange;
    if (isNeedSendChange) {
        currMateData_ = data;
        if (isLyricChange && data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_SINGLE_LYRIC_TEXT)) {
            currMateData_.SetTitle(data.GetSingleLyricText());
            std::string titleAndArtist = data.GetTitle() + " - " + data.GetArtist();
            currMateData_.SetArtist(titleAndArtist);
        }
        currMateData_.SetTitle(TruncateString(currMateData_.GetTitle(), MAX_TITLE_LENGTH));
        return true;
    }
    return false;
}

bool AvrcpMediaInterfaceImpl::impl::CheckPlaybackStateChanged(const AVSession::AVPlaybackState &state)
{
    if (state.GetMask().test(AVSession::AVPlaybackState::PLAYBACK_KEY_STATE) &&
        currPlaybackstate_.GetState() != state.GetState()) {
        currPlaybackstate_ = state;
        return true;
    }
    return false;
}

bool AvrcpMediaInterfaceImpl::impl::CheckLoopModeChanged(const AVSession::AVPlaybackState &state)
{
    if (state.GetMask().test(AVSession::AVPlaybackState::PLAYBACK_KEY_LOOP_MODE) &&
        currPlaybackstate_.GetLoopMode() != state.GetLoopMode()) {
        currPlaybackstate_ = state;
        return true;
    }
    return false;
}

#ifdef MULTI_A2DP_ENABLED
bool AvrcpMediaInterfaceImpl::impl::CheckTrackStateChangedByAddr(const AVSession::AVMetaData &data,
                                                           AVSession::AVMetaData &lastMetaData)
{
    bool isAssetIdChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_ASSET_ID) &&
        lastMetaData.GetAssetId() != data.GetAssetId();
    bool isArtistChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_ARTIST) &&
        lastMetaData.GetArtist() != data.GetArtist();
    bool isAlbumChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_ALBUM) &&
        lastMetaData.GetAlbum() != data.GetAlbum();
    bool isTitleChange = data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_TITLE) &&
        lastMetaData.GetTitle() != data.GetTitle();
    bool isLyricChange = lastMetaData.GetSingleLyricText() != data.GetSingleLyricText(); //存在用户手动关闭蓝牙歌词开关，该字段变为空的情况。
    bool isMediaImageChange = false;
    auto currImg = lastMetaData.GetMediaImage();
    auto newImg = data.GetMediaImage();
    if (currImg != nullptr && newImg != nullptr) {
        isMediaImageChange = !currImg->Equals(*newImg);
    } else {
        isMediaImageChange = (currImg != nullptr) != (newImg != nullptr);
    }

    bool isNeedSendChange = isAssetIdChange || isArtistChange || isAlbumChange || isTitleChange
        || isLyricChange || isMediaImageChange;
    if (isNeedSendChange) {
        lastMetaData = data;
        if (isLyricChange && data.GetMetaMask().test(AVSession::AVMetaData::META_KEY_SINGLE_LYRIC_TEXT)) {
            lastMetaData.SetTitle(data.GetSingleLyricText());
            std::string titleAndArtist = data.GetTitle() + " - " + data.GetArtist();
            lastMetaData.SetArtist(titleAndArtist);
        }
        lastMetaData.SetTitle(TruncateString(lastMetaData.GetTitle(), MAX_TITLE_LENGTH));
        return true;
    }
    return false;
}

bool AvrcpMediaInterfaceImpl::impl::CheckPlaybackStateChangedByAddr(const AVSession::AVPlaybackState &state,
                                                              AVSession::AVPlaybackState &lastPlaybackState)
{
    if (state.GetMask().test(AVSession::AVPlaybackState::PLAYBACK_KEY_STATE) &&
        lastPlaybackState.GetState() != state.GetState()) {
        lastPlaybackState = state;
        return true;
    }
    return false;
}

bool AvrcpMediaInterfaceImpl::impl::CheckLoopModeChangedByAddr(const AVSession::AVPlaybackState &state,
                                                         AVSession::AVPlaybackState &lastPlaybackState)
{
    if (state.GetMask().test(AVSession::AVPlaybackState::PLAYBACK_KEY_LOOP_MODE) &&
        lastPlaybackState.GetLoopMode() != state.GetLoopMode()) {
        lastPlaybackState = state;
        return true;
    }
    return false;
}

#endif

void AvrcpMediaInterfaceImpl::impl::SendKeyEventToAvSession(uint8_t key)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto iter = avrcpToAKeyEventCodeMap_.find(key);
    if (iter == avrcpToAKeyEventCodeMap_.end()) {
        HILOGW("don't deal key %{public}x.", key);
        return;
    }
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        HILOGE("KeyEvent::Create error.");
        return;
    }
    keyEvent->SetKeyCode(iter->second);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);

    auto keyItem = MMI::KeyEvent::KeyItem();
    keyItem.SetKeyCode(iter->second);
    keyItem.SetDownTime(1);
    keyItem.SetPressed(true);
    keyEvent->AddKeyItem(keyItem);
    int32_t result;
    if (activeAddress_ != "00:00:00:00:00:00") { // 判断激活设备是否为空
        AAFwk::Want addressParams;
        addressParams.SetParam("deviceId", activeAddress_);
        result = AVSession::AVSessionManager::GetInstance().SendSystemAVKeyEvent(*keyEvent, addressParams);
    } else {
        result = AVSession::AVSessionManager::GetInstance().SendSystemAVKeyEvent(*keyEvent);
    }
    HILOGI("send avkey event status %{public}d.", result);
    if (result != AVSession::AVSESSION_SUCCESS) {
        HILOGE("excute SendSystemAVKeyEvent error");
    }
}

void AvrcpMediaInterfaceImpl::impl::SubscribeAvsessionSystemAbility()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgrProxy) {
        HILOGE("failed to get samgrProxy");
        return;
    }
    if (avsessionStatusChangeListener_) {
        HILOGE("has been subscribed");
        return;
    }
    const std::set<int> saIdSet = {AVSESSION_SERVICE_ID};
    auto addFunc = [this](int32_t systemAbilityId) {
        RegisterAvSessionListener();
        AttemptCreateAvSessionController();
    };
    auto removeFunc = [this](int32_t systemAbilityId) {
        std::lock_guard<std::mutex> lock(avSessionControllerLock_);
        avSessionController_ = nullptr;
#ifdef MULTI_A2DP_ENABLED
        avSessionControllerMap_.clear();
        avControllerObserverMap_.clear();
        currMateDataByAddr_.clear();
        currPlaybackstateByAddr_.clear();
        lastPauseSessionIdByAddr_.clear();
        {
            std::lock_guard<std::mutex> cbLock(serviceCallbackLock_);
            appSettingsCallbackMap_.clear();
        }
#endif
    };
    avsessionStatusChangeListener_ = new SystemAbilitySubscriber(saIdSet, addFunc, removeFunc);
    int32_t ret = samgrProxy->SubscribeSystemAbility(AVSESSION_SERVICE_ID, avsessionStatusChangeListener_);
    if (ret != ERR_OK) {
        HILOGE("subscribe systemAbilityId: avsession service failed!");
        avsessionStatusChangeListener_ = nullptr;
        return;
    }
}

AvrcpMediaInterfaceImpl::AvrcpMediaInterfaceImpl() : pimpl_(std::make_shared<AvrcpMediaInterfaceImpl::impl>()) {}

void AvrcpMediaInterfaceImpl::Init()
{
    pimpl_->Init(pimpl_);
}

void AvrcpMediaInterfaceImpl::CleanUp()
{
    pimpl_->CleanUp();
}

void AvrcpMediaInterfaceImpl::SendKeyEvent(uint8_t key, KeyState state)
{
    pimpl_->SendKeyEvent(key, state);
}
void AvrcpMediaInterfaceImpl::GetSongInfo(SongInfoCallback cb)
{
    pimpl_->GetSongInfo(cb);
}
void AvrcpMediaInterfaceImpl::GetPlayStatus(PlayStatusCallback cb)
{
    pimpl_->GetPlayStatus(cb);
}
void AvrcpMediaInterfaceImpl::GetNowPlayingList(NowPlayingCallback cb)
{
    pimpl_->GetNowPlayingList(cb);
}
void AvrcpMediaInterfaceImpl::GetMediaPlayerList(MediaListCallback cb)
{
    pimpl_->GetMediaPlayerList(cb);
}
void AvrcpMediaInterfaceImpl::GetFolderItems(uint16_t playerId, std::string mediaId, FolderItemsCallback folderCb)
{
    pimpl_->GetFolderItems(playerId, mediaId, folderCb);
}
void AvrcpMediaInterfaceImpl::SetBrowsedPlayer(uint16_t playerId, SetBrowsedPlayerCallback browseCb)
{
    pimpl_->SetBrowsedPlayer(playerId, browseCb);
}
void AvrcpMediaInterfaceImpl::RegisterUpdateCallback(MediaCallbacks *callback)
{
    pimpl_->RegisterUpdateCallback(callback);
}
void AvrcpMediaInterfaceImpl::UnregisterUpdateCallback(MediaCallbacks *callback)
{
    pimpl_->UnregisterUpdateCallback(callback);
}
void AvrcpMediaInterfaceImpl::PlayItem(uint16_t playerId, bool nowPlaying, std::string mediaId)
{
    pimpl_->PlayItem(playerId, nowPlaying, mediaId);
}
void AvrcpMediaInterfaceImpl::SetActiveDevice(const STACK::RawAddress &address)
{
    pimpl_->SetActiveDevice(address);
}

#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::SendKeyEventByAddr(uint8_t key, KeyState state, const STACK::RawAddress &address)
{
    pimpl_->SendKeyEventByAddr(key, address);
}

void AvrcpMediaInterfaceImpl::SendCtrlCommandToAvSession(uint8_t key, const RawAddress &address)
{
    pimpl_->SendCtrlCommandToAvSession(key, address);
}

void AvrcpMediaInterfaceImpl::AddActiveDevice(const STACK::RawAddress &address)
{
    pimpl_->AddActiveDevice(address);
}

void AvrcpMediaInterfaceImpl::RemoveActiveDevice(const STACK::RawAddress &address)
{
    pimpl_->RemoveActiveDevice(address);
}
void AvrcpMediaInterfaceImpl::GetSongInfoByAddr(SongInfoCallback cb, const STACK::RawAddress &address)
{
    pimpl_->GetSongInfoByAddr(cb, address);
}

void AvrcpMediaInterfaceImpl::GetPlayStatusByAddr(PlayStatusCallback cb, const STACK::RawAddress &address)
{
    pimpl_->GetPlayStatusByAddr(cb, address);
}

void AvrcpMediaInterfaceImpl::GetNowPlayingListByAddr(NowPlayingCallback cb, const STACK::RawAddress &address)
{
    pimpl_->GetNowPlayingListByAddr(cb, address);
}

void AvrcpMediaInterfaceImpl::GetMediaPlayerListByAddr(MediaListCallback cb, const STACK::RawAddress &address)
{
    pimpl_->GetMediaPlayerListByAddr(cb, address);
}

void AvrcpMediaInterfaceImpl::GetFolderItemsByAddr(uint16_t playerId, std::string mediaId,
                                                   FolderItemsCallback folderCb,
                                                   const STACK::RawAddress &address)
{
    pimpl_->GetFolderItemsByAddr(playerId, mediaId, folderCb, address);
}

void AvrcpMediaInterfaceImpl::SetBrowsedPlayerByAddr(uint16_t playerId, SetBrowsedPlayerCallback browseCb,
                                                     const STACK::RawAddress &address)
{
    pimpl_->SetBrowsedPlayerByAddr(playerId, browseCb, address);
}

void AvrcpMediaInterfaceImpl::PlayItemByAddr(uint16_t playerId, bool nowPlaying, std::string mediaId,
                                             const STACK::RawAddress &address)
{
    pimpl_->PlayItemByAddr(playerId, nowPlaying, mediaId, address);
}

void AvrcpMediaInterfaceImpl::AppSettingsSupportByAddr(AppSettingsCallback appCb,
                                                       const STACK::RawAddress &address)
{
    pimpl_->AppSettingsSupportByAddr(appCb, address);
}

void AvrcpMediaInterfaceImpl::SetAppSettingsModeByAddr(uint8_t shuffleMode, uint8_t repeatMode,
                                                       const STACK::RawAddress &address)
{
    pimpl_->SetAppSettingsModeByAddr(shuffleMode, repeatMode, address);
}
#endif
void AvrcpMediaInterfaceImpl::AppSettingsSupport(AppSettingsCallback appCb)
{
    pimpl_->AppSettingsSupport(appCb);
}
void AvrcpMediaInterfaceImpl::SetAppSettingsMode(uint8_t shuffleMode, uint8_t repeatMode)
{
    pimpl_->SetAppSettingsMode(shuffleMode, repeatMode);
}
void AvrcpMediaInterfaceImpl::SetA2dpService(IProfileA2dp *a2dpService)
{
    pimpl_->SetA2dpService(a2dpService);
}
void AvrcpMediaInterfaceImpl::SetAudioManager(IProfileAudioManager *audioManager)
{
    pimpl_->SetAudioManager(audioManager);
}
void AvrcpMediaInterfaceImpl::SendKeyEventByWearDetection(uint8_t key, KeyState state)
{
    pimpl_->SendKeyEventByWearDetection(key, state);
}
void AvrcpMediaInterfaceImpl::SendCtrlCommandToAvSession(uint8_t key)
{
    pimpl_->SendCtrlCommandToAvSession(key);
}
void AvrcpMediaInterfaceImpl::PlaySilenceSound(void)
{
    pimpl_->PlaySilenceSound();
}
#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::SendKeyEventByWearDetectionByAddr(uint8_t key, KeyState state,
    const STACK::RawAddress &address)
{
    pimpl_->SendKeyEventByWearDetectionByAddr(key, state, address);
}

void AvrcpMediaInterfaceImpl::PlaySilenceSoundByAddr(const STACK::RawAddress &address)
{
    pimpl_->PlaySilenceSoundByAddr(address.ToString());
}
#endif
void AvrcpMediaInterfaceImpl::NotifyA2dpPlayStateChange(void)
{
    pimpl_->NotifyA2dpPlayStateChange();
}
#ifdef MULTI_A2DP_ENABLED
void AvrcpMediaInterfaceImpl::NotifyA2dpPlayStateChangeByAddr(const STACK::RawAddress &address)
{
    pimpl_->NotifyA2dpPlayStateChangeByAddr(address);
}
#endif

void AvrcpMediaInterfaceImpl::HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice)
{
    pimpl_->HandlerConnectEvt(socketDevice);
}

extern "C" {
MediaInterface *CreateAvrcpMediaInterface(void)
{
    auto ptr = new AvrcpMediaInterfaceImpl();
    ptr->Init();
    return ptr;
}

void DestroyAvrcpMediaInterface(MediaInterface *interface)
{
    HITRACE_METER(BT_TRACE_TAG);
    AVSession::AVSessionManager::GetInstance().Close();

    if (interface == nullptr) {
        HILOGE("mediaInterface is nullptr");
        return;
    }
    AvrcpMediaInterfaceImpl *mediaInterface = static_cast<AvrcpMediaInterfaceImpl *>(interface);
    mediaInterface->CleanUp();
    delete mediaInterface;
}
}
}  // namespace bluetooth
}  // namespace OHOS
