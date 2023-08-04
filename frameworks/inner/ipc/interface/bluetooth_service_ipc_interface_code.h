/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SERVICE_IPC_INTERFACE_CODE_H
#define BLUETOOTH_SERVICE_IPC_INTERFACE_CODE_H

#include "ipc_types.h"

/* SAID: 1130 */
namespace OHOS {
namespace Bluetooth {
enum BluetoothA2dpSinkObserverInterfaceCode {
    BT_A2DP_SINK_OBSERVER_CONNECTION_STATE_CHANGED = 0,
};

enum BluetoothA2dpSinkInterfaceCode {
    BT_A2DP_SINK_CONNECT = 0,
    BT_A2DP_SINK_DISCONNECT,
    BT_A2DP_SINK_REGISTER_OBSERVER,
    BT_A2DP_SINK_DEREGISTER_OBSERVER,
    BT_A2DP_SINK_GET_DEVICE_BY_STATES,
    BT_A2DP_SINK_GET_DEVICE_STATE,
    BT_A2DP_SINK_GET_PLAYING_STATE,
    BT_A2DP_SINK_SET_CONNECT_STRATEGY,
    BT_A2DP_SINK_GET_CONNECT_STRATEGY,
    BT_A2DP_SINK_SEND_DELAY,
};

enum BluetoothA2dpSourceObserverInterfaceCode {
    BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED,
    BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED,
};

enum BluetoothA2dpSrcInterfaceCode {
    BT_A2DP_SRC_CONNECT = 0,
    BT_A2DP_SRC_DISCONNECT,
    BT_A2DP_SRC_REGISTER_OBSERVER,
    BT_A2DP_SRC_DEREGISTER_OBSERVER,
    BT_A2DP_SRC_GET_DEVICE_BY_STATES,
    BT_A2DP_SRC_GET_DEVICE_STATE,
    BT_A2DP_SRC_GET_PLAYING_STATE,
    BT_A2DP_SRC_SET_CONNECT_STRATEGY,
    BT_A2DP_SRC_GET_CONNECT_STRATEGY,
    BT_A2DP_SRC_SET_ACTIVE_SINK_DEVICE,
    BT_A2DP_SRC_GET_ACTIVE_SINK_DEVICE,
    BT_A2DP_SRC_GET_CODEC_STATUS,
    BT_A2DP_SRC_SET_CODEC_PREFERENCE,
    BT_A2DP_SRC_SWITCH_OPTIONAL_CODECS,
    BT_A2DP_SRC_GET_OPTIONAL_CODECS_SUPPORT_STATE,
    BT_A2DP_SRC_START_PLAYING,
    BT_A2DP_SRC_SUSPEND_PLAYING,
    BT_A2DP_SRC_STOP_PLAYING,
    BT_A2DP_SRC_SET_AUDIO_CONFIGURE,
    BT_A2DP_SRC_WRITE_FRAME,
    BT_A2DP_SRC_GET_RENDER_POSITION,
};

enum class BluetoothAvrcpCtObserverInterfaceCode {
    AVRCP_CT_CONNECTION_STATE_CHANGED = 0,
    AVRCP_CT_PRESS_BUTTON,
    AVRCP_CT_RELEASE_BUTTON,
    AVRCP_CT_SET_BROWSED_PLAYER,
    AVRCP_CT_GET_CAPABILITIES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_SET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUE_TEXT,
    AVRCP_CT_ELEMENT_ATTRIBUTRES,
    AVRCP_CT_GET_PLAY_STATUS,
    AVRCP_CT_PLAY_ITEM,
    AVRCP_CT_GET_TOTAL_NUMBER_OF_ITEMS,
    AVRCP_CT_GET_ITEM_ATTRIBUTES,
    AVRCP_CT_SET_ABSOLUTE_VOLUME,
    AVRCP_CT_PLAYBACK_STATUS_CHANGED,
    AVRCP_CT_TRACK_CHANGED,
    AVRCP_CT_TRACK_REACHED_END,
    AVRCP_CT_TRACK_REACHED_START,
    AVRCP_CT_PLAYBACK_POS_CHANGED,
    AVRCP_CT_PLAY_APP_SETTING_CHANGED,
    AVRCP_CT_NOW_PLAYING_CONTENT_CHANGED,
    AVRCP_CT_AVAILABLE_PLAYER_CHANGED,
    AVRCP_CT_ADDRESSED_PLAYER_CHANGED,
    AVRCP_CT_UID_CHANGED,
    AVRCP_CT_VOLUME_CHANGED,
    AVRCP_CT_GET_MEDIA_PLAYERS,
    AVRCP_CT_GET_FOLDER_ITEMS,
};

enum class BluetoothAvrcpCtInterfaceCode {
    AVRCP_CT_REGISTER_OBSERVER = 0,
    AVRCP_CT_UNREGISTER_OBSERVER,
    AVRCP_CT_GET_CONNECTED_DEVICES,
    AVRCP_CT_GET_DEVICES_BY_STATES,
    AVRCP_CT_GET_DEVICE_STATE,
    AVRCP_CT_CONNECT,
    AVRCP_CT_DISCONNECT,
    AVRCP_CT_PRESS_BUTTON,
    AVRCP_CT_RELEASE_BUTTON,
    AVRCP_CT_GET_UNIT_INFO,
    AVRCP_CT_GET_SUB_UNIT_INFO,
    AVRCP_CT_GET_SUPPORTED_COMPANIES,
    AVRCP_CT_GET_SUPPORTED_EVENTS,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_SET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES_TEXT,
    AVRCP_CT_GET_ELEMENT_ATTRIBUTES,
    AVRCP_CT_GET_PLAYER_STATUS,
    AVRCP_CT_PLAY_ITEM,
    AVRCP_CT_GET_FOLDER_ITEMS,
    AVRCP_CT_GET_TOTAL_NUMBER_OF_ITEMS,
    AVRCP_CT_SET_ABSOLUTE_VOLUME,
    AVRCP_CT_ENABLE_NOTIFICATION,
    AVRCP_CT_DISABLE_NOTIFICATION,
    AVRCP_CT_GET_ITEM_ATTRIBUTES,
    AVRCP_CT_SET_BROWSERED_PLAYER,
    AVRCP_CT_MEDIA_PLAYER_LIST
};

enum BluetoothAvrcpTgObserverInterfaceCode {
    BT_AVRCP_TG_OBSERVER_CONNECTION_STATE_CHANGED = 0,
};

enum BluetoothAvrcpTgInterfaceCode {
    BT_AVRCP_TG_REGISTER_OBSERVER = 0,
    BT_AVRCP_TG_UNREGISTER_OBSERVER,
    BT_AVRCP_TG_SET_ACTIVE_DEVICE,
    BT_AVRCP_TG_CONNECT,
    BT_AVRCP_TG_DISCONNECT,
    BT_AVRCP_TG_GET_CONNECTED_DEVICES,
    BT_AVRCP_TG_GET_DEVICES_BY_STATES,
    BT_AVRCP_TG_GET_DEVICE_STATE,
    BT_AVRCP_TG_NOTIFY_PLAYBACK_STATUS_CHANGED,
    BT_AVRCP_TG_NOTIFY_TRACK_CHANGED,
    BT_AVRCP_TG_NOTIFY_TRACK_REACHED_END,
    BT_AVRCP_TG_NOTIFY_TRACK_REACHED_START,
    BT_AVRCP_TG_NOTIFY_PLAYBACK_POS_CHANGED,
    BT_AVRCP_TG_NOTIFY_PLAYER_APP_SETTING_CHANGED,
    BT_AVRCP_TG_NOTIFY_NOWPLAYING_CONTENT_CHANGED,
    BT_AVRCP_TG_NOTIFY_AVAILABLE_PLAYERS_CHANGED,
    BT_AVRCP_TG_NOTIFY_ADDRESSED_PLAYER_CHANGED,
    BT_AVRCP_TG_NOTIFY_UID_CHANGED,
    BT_AVRCP_TG_NOTIFY_VOLUME_CHANGED,
};

enum BluetoothBleAdvertiseCallbackInterfaceCode {
    BT_BLE_ADVERTISE_CALLBACK_AUTO_STOP_EVENT = 0,
    BT_BLE_ADVERTISE_CALLBACK_RESULT_EVENT,
    BT_BLE_ADVERTISE_CALLBACK_SET_ADV_DATA,
};

enum BluetoothBleAdvertiserInterfaceCode {
    BLE_REGISTER_BLE_ADVERTISER_CALLBACK = 0,
    BLE_DE_REGISTER_BLE_ADVERTISER_CALLBACK,
    BLE_START_ADVERTISING,
    BLE_START_ADVERTISING_WITH_RAWDATA,
    BLE_STOP_ADVERTISING,
    BLE_CLOSE,
    BLE_GET_ADVERTISER_HANDLE,
    BLE_SET_ADVERTISING_DATA,
};

enum BluetoothBleCentralManagerCallbackInterfaceCode {
    BT_BLE_CENTRAL_MANAGER_CALLBACK = 0,
    BT_BLE_CENTRAL_MANAGER_BLE_BATCH_CALLBACK,
    BT_BLE_CENTRAL_MANAGER_CALLBACK_SCAN_FAILED,
    BT_BLE_LPDEVICE_CALLBACK_NOTIFY_MSG_REPORT,
};

enum BluetoothBleCentralManagerInterfaceCode {
    BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK = 0,
    BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK,
    BLE_START_SCAN,
    BLE_START_SCAN_WITH_SETTINGS,
    BLE_STOP_SCAN,
    BLE_CONFIG_SCAN_FILTER,
    BLE_REMOVE_SCAN_FILTER,
    BLE_PROXY_UID,
    BLE_RESET_ALL_PROXY,
    BLE_SET_LPDEVICE_ADV_PARAM,
    BLE_SET_SCAN_REPORT_CHANNEL_TO_LPDEVICE,
    BLE_ENABLE_SYNC_DATA_TO_LPDEVICE,
    BLE_DISABLE_SYNC_DATA_TO_LPDEVICE,
    BLE_SEND_PARAMS_TO_LPDEVICE,
    BLE_IS_LPDEVICE_AVAILABLE,
    BLE_SET_LPDEVICE_PARAM,
    BLE_REMOVE_LPDEVICE_PARAM,
};

enum BluetoothBlePeripheralObserverInterfaceCode {
    BLE_ON_READ_REMOTE_RSSI_EVENT = 0,
    BLE_PAIR_STATUS_CHANGED,
    BLE_ACL_STATE_CHANGED,
};

enum BluetoothGattClientCallbackInterfaceCode {
    BT_GATT_CLIENT_CALLBACK_CONNECT_STATE_CHANGE = 0,
    BT_GATT_CLIENT_CALLBACK_CHARACTER_CHANGE,
    BT_GATT_CLIENT_CALLBACK_CHARACTER_READ,
    BT_GATT_CLIENT_CALLBACK_CHARACTER_WRITE,
    BT_GATT_CLIENT_CALLBACK_DESCRIPTOR_READ,
    BT_GATT_CLIENT_CALLBACK_DESCRIPTOR_WRITE,
    BT_GATT_CLIENT_CALLBACK_MTU_UPDATE,
    BT_GATT_CLIENT_CALLBACK_SERVICES_DISCOVER,
    BT_GATT_CLIENT_CALLBACK_CONNECTION_PARA_CHANGE,
    BT_GATT_CLIENT_CALLBACK_SERVICES_CHANGED,
    BT_GATT_CLIENT_CALLBACK_READ_REMOTE_RSSI_VALUE,
};

enum BluetoothGattClientInterfaceCode {
    BT_GATT_CLIENT_REGISTER_APP = 0,
    BT_GATT_CLIENT_DEREGISTER_APP,
    BT_GATT_CLIENT_CONNECT,
    BT_GATT_CLIENT_DIS_CONNECT,
    BT_GATT_CLIENT_DISCOVERY_SERVICES,
    BT_GATT_CLIENT_READ_CHARACTERISTIC,
    BT_GATT_CLIENT_WRITE_CHARACTERISTIC,
    BT_GATT_CLIENT_SIGNED_WRITE_CHARACTERISTIC,
    BT_GATT_CLIENT_READ_DESCRIPTOR,
    BT_GATT_CLIENT_WRITE_DESCRIPTOR,
    BT_GATT_CLIENT_REQUEST_EXCHANGE_MTU,
    BT_GATT_CLIENT_GET_ALL_DEVICE,
    BT_GATT_CLIENT_REQUEST_CONNECTION_PRIORITY,
    BT_GATT_CLIENT_GET_SERVICES,
    BT_GATT_CLIENT_REQUEST_FASTEST_CONNECTION,
    BT_GATT_CLIENT_READ_REMOTE_RSSI_VALUE,
    BT_GATT_CLIENT_REQUEST_NOTIFICATION,
};

enum BluetoothGattServerCallbackInterfaceCode {
    GATT_SERVER_CALLBACK_CHARACTERISTIC_READREQUEST = 0,
    GATT_SERVER_CALLBACK_CONNECTIONSTATE_CHANGED,
    GATT_SERVER_CALLBACK_ADD_SERVICE,
    GATT_SERVER_CALLBACK_CHARACTERISTIC_READ_REQUEST,
    GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE_REQUEST,
    GATT_SERVER_CALLBACK_DESCRIPTOR_READ_REQUEST,
    GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE_REQUEST,
    GATT_SERVER_CALLBACK_MTU_CHANGED,
    GATT_SERVER_CALLBACK_NOTIFY_CONFIRM,
    GATT_SERVER_CALLBACK_CONNECTION_PARAMETER_CHANGED,
};

enum BluetoothGattServerInterfaceCode {
    GATT_SERVER_CLEAR_SERVICES = 0,
    GATT_SERVER_ADD_SERVICE,
    GATT_SERVER_REGISTER,
    GATT_SERVER_DEREGISTER,
    GATT_SERVER_CANCEL_CONNECTION,
    GATT_SERVER_NOTIFY_CLIENT,
    GATT_SERVER_REMOVE_SERVICE,
    GATT_SERVER_RESPOND_CHARACTERISTIC_READ,
    GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE,
    GATT_SERVER_RESPOND_DESCRIPTOR_READ,
    GATT_SERVER_RESPOND_DESCRIPTOR_WRITE,
};

enum BluetoothHfpAgObserverInterfaceCode {
    BT_HFP_AG_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    BT_HFP_AG_OBSERVER_SCO_STATE_CHANGED,
    BT_HFP_AG_OBSERVER_ACTIVE_DEVICE_CHANGED,
    BT_HFP_AG_OBSERVER_HF_ENHANCED_DRIVER_SAFETY_CHANGED,
};

enum BluetoothHfpAgInterfaceCode {
    BT_HFP_AG_GET_CONNECT_DEVICES = 0,
    BT_HFP_AG_GET_DEVICES_BY_STATES,
    BT_HFP_AG_GET_DEVICE_STATE,
    BT_HFP_AG_CONNECT,
    BT_HFP_AG_DISCONNECT,
    BT_HFP_AG_GET_SCO_STATE,
    BT_HFP_AG_CONNECT_SCO,
    BT_HFP_AG_DISCONNECT_SCO,
    BT_HFP_AG_PHONE_STATE_CHANGED,
    BT_HFP_AG_CLCC_RESPONSE,
    BT_HFP_AG_OPEN_VOICE_RECOGNITION,
    BT_HFP_AG_CLOSE_VOICE_RECOGNITION,
    BT_HFP_AG_SET_ACTIVE_DEVICE,
    BT_HFP_AG_INTO_MOCK,
    BT_HFP_AG_SEND_NO_CARRIER,
    BT_HFP_AG_GET_ACTIVE_DEVICE,
    BT_HFP_AG_REGISTER_OBSERVER,
    BT_HFP_AG_DEREGISTER_OBSERVER,
    BT_HFP_AG_SET_CONNECT_STRATEGY,
    BT_HFP_AG_GET_CONNECT_STRATEGY,
};

enum BluetoothHfpHfObserverInterfaceCode {
    BT_HFP_HF_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    BT_HFP_HF_OBSERVER_SCO_STATE_CHANGED,
    BT_HFP_HF_OBSERVER_CALL_CHANGED,
    BT_HFP_HF_OBSERVER_SIGNAL_STRENGTH_CHANGED,
    BT_HFP_HF_OBSERVER_REGISTRATION_STATUS_CHANGED,
    BT_HFP_HF_OBSERVER_ROAMING_STATUS_CHANGED,
    BT_HFP_HF_OBSERVER_OPERATOR_SELECTION_CHANGED,
    BT_HFP_HF_OBSERVER_SUBSCRIBER_NUMBER_CHANGED,
    BT_HFP_HF_OBSERVER_VOICE_RECOGNITION_STATUS_CHANGED,
    BT_HFP_HF_OBSERVER_IN_BAND_RING_TONE_CHANGED,
};

enum BluetoothHfpHfInterfaceCode {
    BT_HFP_HF_CONNECT_SCO = 0,
    BT_HFP_HF_DISCONNECT_SCO,
    BT_HFP_HF_GET_DEVICES_BY_STATES,
    BT_HFP_HF_GET_DEVICE_STATE,
    BT_HFP_HF_GET_SCO_STATE,
    BT_HFP_HF_SEND_DTMF_TONE,
    BT_HFP_HF_CONNECT,
    BT_HFP_HF_DISCONNECT,
    BT_HFP_HF_OPEN_VOICE_RECOGNITION,
    BT_HFP_HF_CLOSE_VOICE_RECOGNITION,
    BT_HFP_HF_GET_CURRENT_CALL_LIST,
    BT_HFP_HF_ACCEPT_INCOMING_CALL,
    BT_HFP_HF_HOLD_ACTIVE_CALL,
    BT_HFP_HF_REJECT_INCOMING_CALL,
    BT_HFP_HF_HANDLE_INCOMING_CALL,
    BT_HFP_HF_HANDLE_MULLTI_CALL,
    BT_HFP_HF_DIAL_LAST_NUMBER,
    BT_HFP_HF_DIAL_MEMORY,
    BT_HFP_HF_FINISH_ATIVE_CALL,
    BT_HFP_HF_START_DIAL,
    BT_HFP_HF_REGISTER_OBSERVER,
    BT_HFP_HF_DEREGISTER_OBSERVER,
    BT_HFP_HF_SEND_VOICE_TAG,
    BT_HFP_HF_SEND_KEY_PRESSED
};

enum class BluetoothHidHostObserverInterfaceCode {
    COMMAND_ON_CONNECTION_STATE_CHANGED = MIN_TRANSACTION_ID + 0,
};

enum class BluetoothHidHostInterfaceCode {
    COMMAND_CONNECT = MIN_TRANSACTION_ID + 0,
    COMMAND_DISCONNECT,
    COMMAND_GET_DEVICE_STATE,
    COMMAND_GET_DEVICES_BY_STATES,
    COMMAND_REGISTER_OBSERVER,
    COMMAND_DEREGISTER_OBSERVER,
    COMMAND_VCUN_PLUG,
    COMMAND_SEND_DATA,
    COMMAND_SET_REPORT,
    COMMAND_GET_REPORT,
    COMMAND_SET_CONNECT_STRATEGY,
    COMMAND_GET_CONNECT_STRATEGY,
};

enum BluetoothHostObserverInterfaceCode {
    BT_HOST_OBSERVER_STATE_CHANGE = 0,
    // ON_DIS_STA_CHANGE_CODE
    BT_HOST_OBSERVER_DISCOVERY_STATE_CHANGE,
    BT_HOST_OBSERVER_DISCOVERY_RESULT,
    BT_HOST_OBSERVER_PAIR_REQUESTED,
    BT_HOST_OBSERVER_PAIR_CONFIRMED,
    BT_HOST_OBSERVER_SCAN_MODE_CHANGED,
    BT_HOST_OBSERVER_DEVICE_NAME_CHANGED,
    BT_HOST_OBSERVER_DEVICE_ADDR_CHANGED,
};

enum BluetoothHostInterfaceCode {
    BT_REGISTER_OBSERVER = 0,
    BT_DEREGISTER_OBSERVER,
    BT_ENABLE,
    BT_DISABLE,
    BT_GETSTATE,
    BT_GETPROFILE,
    BT_GET_BLE,
    BT_FACTORY_RESET,
    BT_DISABLE_BLE,
    BT_ENABLE_BLE,
    BT_IS_BR_ENABLED,
    BT_IS_BLE_ENABLED,
    BT_GET_PROFILE_LIST,
    BT_GET_MAXNUM_CONNECTED_AUDIODEVICES,
    BT_GET_BT_STATE,
    BT_GET_BT_PROFILE_CONNSTATE,
    BT_GET_LOCAL_DEVICE_CLASS,
    BT_SET_LOCAL_DEVICE_CLASS,
    BT_GET_LOCAL_ADDRESS,
    BT_GET_LOCAL_NAME,
    BT_SET_LOCAL_NAME,
    BT_GET_BT_SCAN_MODE,
    BT_SET_BT_SCAN_MODE,
    BT_GET_BONDABLE_MODE,
    BT_SET_BONDABLE_MODE,
    BT_START_BT_DISCOVERY,
    BT_CANCEL_BT_DISCOVERY,
    BT_IS_BT_DISCOVERING,
    BT_GET_BT_DISCOVERY_END_MILLIS,
    BT_GET_PAIRED_DEVICES,
    BT_REMOVE_PAIR,
    BT_REMOVE_ALL_PAIRS,
    BT_REGISTER_REMOTE_DEVICE_OBSERVER,
    BT_DEREGISTER_REMOTE_DEVICE_OBSERVER,
    BT_GET_BLE_MAX_ADVERTISING_DATALENGTH,
    GET_DEVICE_TYPE,
    GET_PHONEBOOK_PERMISSION,
    SET_PHONEBOOK_PERMISSION,
    GET_MESSAGE_PERMISSION,
    SET_MESSAGE_PERMISSION,
    GET_POWER_MODE,
    GET_DEVICE_NAME,
    GET_DEVICE_ALIAS,
    SET_DEVICE_ALIAS,
    GET_DEVICE_BATTERY_LEVEL,
    GET_PAIR_STATE,
    START_PAIR,
    CANCEL_PAIRING,
    IS_BONDED_FROM_LOCAL,
    IS_ACL_CONNECTED,
    IS_ACL_ENCRYPTED,
    GET_DEVICE_CLASS,
    SET_DEVICE_PIN,
    SET_DEVICE_PAIRING_CONFIRMATION,
    SET_DEVICE_PASSKEY,
    PAIR_REQUEST_PEPLY,
    READ_REMOTE_RSSI_VALUE,
    GET_LOCAL_SUPPORTED_UUIDS,
    GET_DEVICE_UUIDS,
    BT_REGISTER_BLE_ADAPTER_OBSERVER,
    BT_DEREGISTER_BLE_ADAPTER_OBSERVER,
    BT_REGISTER_BLE_PERIPHERAL_OBSERVER,
    BT_DEREGISTER_BLE_PERIPHERAL_OBSERVER,
    GET_LOCAL_PROFILE_UUIDS,
    BT_SET_FAST_SCAN,
    GET_RANDOM_ADDRESS,
    SYNC_RANDOM_ADDRESS,
};

enum BluetoothMapMceObserverInterfaceCode {
    MCE_ON_ACTION_COMPLETED = 0,
    MCE_ON_CONNECTION_STATE_CHANGED,
    MCE_ON_EVENT_REPORTED,
    MCE_ON_BMESSAGE_COMPLETED,
    MCE_ON_MESSAGE_LIST_COMPLETED,
    MCE_ON_CONVERSATION_LIST_COMPLETED,
};

enum BluetoothMapMceInterfaceCode {
    MCE_REGISTER_OBSERVER = 0,
    MCE_DE_REGISTER_OBSERVER,
    MCE_CONNECT,
    MCE_DISCONNECT,
    MCE_ISCONNECTED,
    MCE_GET_CONNECT_DEVICES,
    MCE_GET_DEVICES_BY_STATES,
    MCE_GET_CONNECTION_STATE,
    MCE_SET_CONNECTION_STRATEGY,
    MCE_GET_CONNECTION_STRATEGY,
    MCE_GET_UNREAD_MESSAGES,
    MCE_GET_SUPPORTED_FEATURES,
    MCE_SEND_MESSAGE,
    MCE_SET_NOTIFICATION_FILTER,
    MCE_GET_MESSAGES_LISTING,
    MCE_GET_MESSAGE,
    MCE_UPDATE_INBOX,
    MCE_GET_CONVERSATION_LISTING,
    MCE_SET_MESSAGE_STATUS,
    MCE_SET_OWNER_STATUS,
    MCE_GET_OWNER_STATUS,
    MCE_GET_MAS_INSTANCE_INFO,
};

enum BluetoothMapMseObserverInterfaceCode {
    MSE_ON_CONNECTION_STATE_CHANGED = 0,
    MSE_ON_PERMISSION,
};

enum BluetoothMapMseInterfaceCode {
    MSE_REGISTER_OBSERVER = 0,
    MSE_DE_REGISTER_OBSERVER,
    MSE_GET_STATE,
    MSE_DISCONNECT,
    MSE_ISCONNECTED,
    MSE_GET_CONNECTED_DEVICES,
    MSE_GET_DEVICES_BY_STATES,
    MSE_GET_CONNECTION_STATE,
    MSE_SET_CONNECTION_STRATEGY,
    MSE_GET_CONNECTION_STRATEGY,
    MSE_GRANT_PERMISSION,
};

enum class BluetoothOppObserverInterfaceCode {
    COMMAND_ON_RECEIVE_INCOMING_FILE_CHANGED = MIN_TRANSACTION_ID + 0,
    COMMAND_ON_TRANSFER_STATE_CHANGED,
};

enum class BluetoothOppInterfaceCode {
    COMMAND_SEND_FILE = MIN_TRANSACTION_ID + 0,
    COMMAND_SET_INCOMING_FILE_CONFIRMATION,
    COMMAND_GET_CURRENT_TRANSFER_INFORMATION,
    COMMAND_CANCEL_TRANSFER,
    COMMAND_REGISTER_OBSERVER,
    COMMAND_DEREGISTER_OBSERVER,
    COMMAND_GET_DEVICE_STATE,
    COMMAND_GET_DEVICES_BY_STATES,
};

enum class BluetoothPanObserverInterfaceCode {
    COMMAND_ON_CONNECTION_STATE_CHANGED = MIN_TRANSACTION_ID + 0,
};

enum class BluetoothPanInterfaceCode {
    COMMAND_DISCONNECT = MIN_TRANSACTION_ID + 0,
    COMMAND_GET_DEVICE_STATE,
    COMMAND_GET_DEVICES_BY_STATES,
    COMMAND_REGISTER_OBSERVER,
    COMMAND_DEREGISTER_OBSERVER,
    COMMAND_SET_TETHERING,
    COMMAND_IS_TETHERING_ON,
};


enum BluetoothPbapPceObserverInterfaceCode {
    PBAP_PCE_ON_SERVICE_CONNECTION_STATE_CHANGED = 0,
    PBAP_PCE_ON_SERVICE_PASSWORD_REQUIRED,
    PBAP_PCE_ON_ACTION_COMPLETED,
};

enum BluetoothPbapPceInterfaceCode {
    PBAP_PCE_GET_DEVICE_STATE = 0,
    PBAP_PCE_CONNECT,
    PBAP_PCE_PULL_PHONEBOOK,
    PBAP_PCE_SET_PHONEBOOK,
    PBAP_PCE_PULL_VCARD_LISTING,
    PBAP_PCE_PULL_VCARD_ENTRY,
    PBAP_PCE_PULL_ISDOWNLOAGING,
    PBAP_PCE_ABORT_DOWNLOADING,
    PBAP_PCE_SET_DEVICE_PASSWORD,
    PBAP_PCE_DISCONNECT,
    PBAP_PCE_SET_CONNECT_STRATEGY,
    PBAP_PCE_GET_CONNECT_STRATEGY,
    PBAP_PCE_GET_DEVICES_BY_STATE,
    PBAP_PCE_REGISTER_OBSERVER,
    PBAP_PCE_DEREGISTER_OBSERVER,
};

enum BluetoothPbapPseObserverInterfaceCode {
    PBAP_PSE_ON_SERVICE_CONNECTION_STATE_CHANGE = 0,
    PBAP_PSE_ON_SERVICE_PERMISSION,
    PBAP_PSE_ON_SERVICE_PASSWORD_REQUIRED,
};

enum BluetoothPbapPseInterfaceCode {
    PBAP_PSE_GET_DEVICE_STATE = 0,
    PBAP_PSE_GET_DEVICES_BY_STATES,
    PBAP_PSE_DISCONNECT,
    PBAP_PSE_SET_CONNECTION_STRATEGY,
    PBAP_PSE_GET_CONNECTION_STRATEGY,
    PBAP_PSE_GRANT_PERMISSION,
    PBAP_PSE_SET_DEVICE_PASSWORD,
    PBAP_PSE_REGISTER_OBSERVER,
    PBAP_PSE_DEREGISTER_OBSERVER,
};

enum BluetoothRemoteDeviceObserverInterfaceCode {
    BT_REMOTE_DEVICE_OBSERVER_ACL_STATE = 0,
    BT_REMOTE_DEVICE_OBSERVER_PAIR_STATUS,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_UUID,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_NAME,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_ALIAS,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_COD,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_BATTERY_LEVEL,
};

enum BluetoothSocketInterfaceCode {
    SOCKET_CONNECT = 0,
    SOCKET_LISTEN,
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_SERVICE_IPC_INTERFACE_CODE_H