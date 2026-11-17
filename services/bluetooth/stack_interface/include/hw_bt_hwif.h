/*
 * Copyright (c) 2017-2020 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef VENDOR_INCLUDE_BT_HWIF_H
#define VENDOR_INCLUDE_BT_HWIF_H

#include <vector>
#include "hardware/bluetooth.h"
#include "types/raw_address.h"

#ifndef STACK
#define STACK
#endif

extern "C" { //__BEGIN_DECLS

#define BT_VENDOR_INTERFACE_ID "bt_vender_interface"
/* OH extension: vendor watch interface id */
#define BT_VENDOR_WATCH_ID "watch"
typedef enum {
    BT_STATC_ERRNO_MIN,
    BT_HCI_ERRNO = 100,
    BT_L2CAP_ERRNO = 200,
    BT_REMOTE_SEND_L2CAP_SECURITY_BLOCK,
    BT_RFCOMM_ERRNO = 300,
    BT_REMOTE_SEND_RFCOMM_NO_RESOURCES,
    BT_SDP_ERRNO = 400,
    BT_HFP_ERRNO = 500,
    BT_REMOTE_SENT_HFP_DM,
    BT_A2DP_ERRNO = 600,
    BT_REMOTE_SEND_AVDTP_NO_RESOURCES,
    BT_REMOTE_SEND_AVDTP_CLOSE,
    BT_AVRCP_ERRNO = 700,
    BT_REMOTE_SEND_AVRCP_NO_RESOURCES,
} BtStackErrno;

struct A2dpOffloadConfigCallback {
    uint16_t media_packet_header;
    uint8_t m_pt;
    uint32_t ssrc;
    uint8_t boundary_flag;
    uint8_t broadcast_flag;
    uint32_t codec_type;
    uint16_t max_latency;
    uint16_t scms_t_enable;
    uint32_t sample_rate;
    uint32_t encoded_audio_bitrate;
    uint8_t bits_per_sample;
    uint8_t ch_mode;
    uint16_t acl_hdl;
    uint16_t l2c_rcid;
    uint16_t mtu;
    uint8_t codec_specific_0;
    uint8_t codec_specific_1;
    uint8_t codec_specific_2;
    uint8_t codec_specific_3;
    uint8_t codec_specific_4;
    uint8_t codec_specific_5;
    uint8_t codec_specific_6;
    uint8_t codec_specific_7;
};

typedef struct {
    uint8_t codec;
    uint8_t sampleRate;
    uint8_t channel;
    uint8_t bitWidth;
} HdapConfigCallback;

typedef struct {
    STACK::RawAddress addr;
    uint64_t msb;
    uint64_t lsb;
    int status;
    int result;
    int type;
    int psm;
} StackCallbackParam;

// read rssi for unlock
typedef void (*ReadRssiCmdCallback)(int status, int rssi, STACK::RawAddress *rem_bda);

typedef void (*StackErrnoCallback)(bt_status_t status, STACK::RawAddress *remote_bd_addr, BtStackErrno state);

typedef void (*HwIcarryDeviceCallback)(STACK::RawAddress *addr, int action);

typedef void (*HwIcarryNameCallback)(STACK::RawAddress *addr, std::string name);

typedef void (*HwBatteryReportCallback)(STACK::RawAddress *addr, std::string str);

typedef void (*HwHiechoRspCallback)(STACK::RawAddress *addr, uint16_t len, uint8_t *state);

#ifdef COMMUNICATION_L2
typedef void (*HwHdapConnectCallback)(STACK::RawAddress *addr, bool isConnected, uint8_t featureBit,
    HdapConfigCallback config);
#else
typedef void (*HwHdapConnectCallback)(STACK::RawAddress *addr, bool isConnected, uint8_t featureBit);
#endif
typedef void (*HwProfileStateCallback)(STACK::RawAddress *addr, uint8_t a2dpState, uint8_t hfpState,
    std::string targetBtDevice, uint8_t a2dpServiceType);

typedef void (*HwShouldBlockBcc)(STACK::RawAddress *addr, bool *isShould);

typedef void (*HwAclDisconnReasonCallback)(STACK::RawAddress *addr, int reason);

typedef void (*HwGetSensorhubDevInfoCallback)(uint8_t *buffer, int length);

typedef void (*HwSensorhubResetCallback)(uint32_t state);

typedef void (*HwHiechoIndCallback)(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args, uint16_t argsLen);

typedef void (*HwBluetoothConnectionCallback)(const StackCallbackParam &param);

typedef void (*HwA2dpReconfigDoneCallback)(STACK::RawAddress *addr);

typedef void (*HwMediaPreemptCapCallback)(STACK::RawAddress *addr, bool isSupportMediaPreemptCap);

typedef void (*HwDeviceCallback)(STACK::RawAddress *addr, int deviceType);

typedef void (*HwObtainPeerStoredA2dpCodecType)(STACK::RawAddress *addr, int *codecType);

typedef void (*HwA2dpOffloadCallback)(STACK::RawAddress *addr, bool isOffload);

typedef void (*HwA2dpOffloadCodecCallback)(STACK::RawAddress *addr, A2dpOffloadConfigCallback offloadCodec);

typedef void (*HwHiechoDeviceAclEncryptionChangedCallback)(const STACK::RawAddress &addr);

typedef void (*HwReportA2dpAbnormalStatus)(const STACK::RawAddress &addr, int32_t errCode);

#ifdef CONTEXTHUB_BLE_V3
typedef void (*HwSensorhubCollaborationCallback)(const std::vector<uint8_t> &notifyValue);
#endif

//  extern function
extern bt_status_t BtifStackErrnoReport(STACK::RawAddress *remoteAddr, BtStackErrno state);

// Vendor Add callback structure.
typedef struct {
    // set to sizeof(bt_callbacks_t)
    size_t size;
    ReadRssiCmdCallback readRssiCmdCb; // read rssi for unlock
    StackErrnoCallback stackErrnoCb;
    HwIcarryNameCallback hwIcarryNameCb;
    HwHdapConnectCallback hwHdapConnectCb;
    HwProfileStateCallback hwProfileStateCb;
    HwShouldBlockBcc hwShouldBlockBcc;
    HwAclDisconnReasonCallback hwAclDisconnReasonCb;
    HwGetSensorhubDevInfoCallback hwGetSensorhubDevInfoCb;
    HwSensorhubResetCallback hwSensorhubResetCb;
    HwHiechoIndCallback hwHiechoIndCb;
    HwBluetoothConnectionCallback hwBluetoothConnCb;
    HwA2dpReconfigDoneCallback hwA2dpReconfigDoneCb;
    HwMediaPreemptCapCallback hwMediaPreemptCapCb;
    HwDeviceCallback hwDeviceCb;
    HwObtainPeerStoredA2dpCodecType hwObtainPeerStoredA2dpCodecType;
    HwA2dpOffloadCallback hwA2dpOffloadCb;
    HwA2dpOffloadCodecCallback hwA2dpOffloadCodecCb;
    HwHiechoDeviceAclEncryptionChangedCallback HwHiechoDeviceAclEncryptionChangeCb;
    HwReportA2dpAbnormalStatus hwReportA2dpAbnormalStatusCb;
#ifdef CONTEXTHUB_BLE_V3
    HwSensorhubCollaborationCallback hwSensorhubCollobarationCb;
#endif
    // Reserved for future use. Must be NULL.24 is total size, occupy 11
    void* reservedProc[24 - 10];
} BthwifCallbacks;

/* Vendor add */
/* define Bluetooth dynamic config type */
typedef enum {
    HW_INTEROP_CFG_ADD, // add interop databases from Bluetooth app to bt stack
} HwCfgType;

typedef enum {
    INTEROP_DATA_OP_USED,     // data is used in stack.
    INTEROP_DATA_OP_DISABLED, // disable the static interop data in stack, for futrue.
} HwOpCode;

typedef enum {
    INTEROP_DATA_ADDR,     // data used for addr elem,matched by addr and feature.
    INTEROP_DATA_NAME,     // data used for name elem,matched by name and feature.
    INTEROP_DATA_MANUFCTR, // data used for manufctr elem,matched by manufctr and feature.
} HwDataType;

typedef struct {
    STACK::RawAddress addr;
    size_t length;
    int feature;
} HwInteropAddrEntry;

typedef struct {
    char *name;
    int feature;
    uint8_t len;
    uint8_t case_sensitive;
} HwInteropNameEntry;

typedef struct {
    uint16_t manufacturer;
    int feature;
} HwInteropManufacturer;

typedef union {
    HwInteropAddrEntry addr_data;
    HwInteropNameEntry name_data;
    HwInteropManufacturer manufctr_data;
} HwBtDataUnion;

typedef struct {
    HwOpCode op;
    HwDataType type;
    HwBtDataUnion data;
} HwBtCfg;

typedef struct {
    int32_t sessionId;
    int32_t streamType;
    int32_t sampleRate;
    int32_t isSpatialAudio;
    int32_t isPlaying;
} HwBtA2dpSteamSessionInfo;

typedef enum {
    HW_CONN_ATTR_TYPE_KEEP_BLE_SCAN_IN_CONN,
    HW_CONN_ATTR_TYPE_SET_LP_DEVICE_BUFFER_SIZE_AND_COUNT,
} HwConnAttrType;

typedef enum {
    HW_VOICE_COMBINE_STATE_NONE = 0x00,
    HW_VOICE_COMBINE_STATE_ABILITY_ON_NN_SWITCH_OFF = 0x01,
    HW_VOICE_COMBINE_STATE_ABILITY_ON_NN_SWITCH_ON = 0x02,
} HwVoiceCombineStateType;

typedef struct {
    HwConnAttrType type;
    union {
        bool keepBleScanInConn;
        uint64_t reserved;
    };
} HwConnAttr;

typedef struct {
    uint8_t enabled;
    int32_t transport;
    STACK::RawAddress rawAddr;
} ForceSleepFilterHidData;

typedef union {
    ForceSleepFilterHidData hidData;
} UnionCmdToHisi;

typedef struct {
    UnionCmdToHisi unionCmdData;
    uint16_t optcode;
} SpecificCmdToHisi;

typedef struct {
    // set to sizeof(BtHfInterface)
    size_t size;

    // Register the Bthwif callbacks
    bt_status_t (*Init)(BthwifCallbacks *callbacks);

    // Read rssi for unlock
    int (*readRssi)(STACK::RawAddress *bdAddr);

    // Create a fast connection to a remote LE.
    bt_status_t (*FastConnect)(int clientIf, const STACK::RawAddress *bdAddr, bool isDirect, int transport);

    /* Get supported features of remote device */
    int (*hf_get_remote_features)(STACK::RawAddress *bdAddr);

    // This ensures the chip is Powered ON  to support other radios in the combo chip.
    // If the chip is OFF it set the chip to ON, if it is already ON it just increases the radio ref count
    // to keep track when to Power OFF
    int (*enableRadio)();

    // This decreases radio ref count  and ensures that chip is Powered OFF
    // when the radio ref count becomes zero.
    int (*disableRadio)();

    // Get FM module interface
    const void *(*getFmInterface)();

    // Write cfg data to bt stack.
    int (*hwDynamicConfig)(void);

    int (*getLdacQuality)(void); // get_ldac

    bool (*getHighQuality)(STACK::RawAddress *bdAddr);
    bool (*setHighQuality)(STACK::RawAddress *bdAddr, bool enable);
    bool (*getInteropDataMatchAddr)(const STACK::RawAddress *bdAddr, std::string feature);
    bool (*setInteropDataMatchAddr)(const STACK::RawAddress *bdAddr, std::string feature, bool enable);
    /* check if current device is bonded */
    bool (*checkRemoteDeviceBonded)(const STACK::RawAddress &remoteAddr);
    void (*setInNaviMode)(bool isNavi);
    void (*setHiDeviceScanFlag)(bool flag);

    void (*setBurstParam)(int duration, int maxExtAdvEvents, int burstWindow, int burstInterval, int advId);
    void (*sendLowLatencyStatus)(bool isEnabled);
    void (*hwBtifA2dpQueueRemove)(STACK::RawAddress &bdAddr);
    void (*hwBtifHfpQueueRemove)(STACK::RawAddress &bdAddr);
    int (*disconnectRcuInStr)(const STACK::RawAddress &bdAddr);
    void (*hwBtsndHcicDisconnect)(const STACK::RawAddress& bdAddr);
    bool (*hwGetDeviceInfo)(const STACK::RawAddress& bdAddr, bt_property_t &prop);
    void (*hwSendSceneType)(int type);
    int (*hwGetCurrentSceneType)();
    STACK::RawAddress (*HwGetPrimaryTwsAddress)(const STACK::RawAddress &addr);
    bool (*getTwsMonoMode)();
    void (*setHdapActive)(const STACK::RawAddress &bdAddr);
    void (*sendDeviceSelection)(const STACK::RawAddress &bdAddr, int useA2dp, int useHfp, int userSelection);
    STACK::RawAddress (*HwGetIdentityAddress)(const STACK::RawAddress &addr);
    bool (*isIncomingConnection)(const STACK::RawAddress& bdAddr);
    void (*leConnectFastest)(const STACK::RawAddress &addr);
    void (*setCocPreferredPhy)(const STACK::RawAddress &addr, int txPhy, int rxPhy, int options);
    void (*updateCocConnectionParams)(const STACK::RawAddress &bdAddr, const int* params, int len);
    bool (*getSensorhubCap)();
    bool (*changeAdvReportPath)(bool isWake);
    uint8_t (*getSensorhubAdvHandle)();
    void (*resetSensorhub)(bool enableSensorhub);
    void (*sendParamsToSensorhub)(const char *params, char type, int length);
    void (*deleteSensorhubFilter)(char filterID);
    bool (*hwSetAdvKey)(const STACK::RawAddress& addr, const char& irk, const char& hbk,
                        const uint8_t version, const char& feature);
    bool (*hwRemoveAdvKey)(const STACK::RawAddress& addr);
    void (*hwGetAdvIrk)(const STACK::RawAddress& addr, std::vector<uint8_t>& irk);
    void (*hwSetHDCallState)(bool isEnable);
    void (*sendHiechoData)(uint8_t type, uint8_t *source, uint16_t len, uint8_t *args);
    bool (*isDeviceSupportEcho)(const STACK::RawAddress& bdAddr);
    void (*hwSetConnReasonFlag)(const STACK::RawAddress &addr, int connType, const uint8_t *connReasonData,
        int connReasonDataLen);
    bool (*hwBtifIsHdapSupported)();
    void (*registerConnection)(const STACK::RawAddress& addr, const ::bluetooth::Uuid& uuid);
    void (*unRegisterConnection)(const STACK::RawAddress& addr, const ::bluetooth::Uuid& uuid);
    void (*hwRecordRmtDeviceSpecificCap)(const STACK::RawAddress &addr, int capID);
    void (*hwClearRmtDeviceSpecificCap)(const STACK::RawAddress &addr, int capID);
    void (*hwSetLowLatencySwitchStatusByAddr)(const STACK::RawAddress &addr, bool isOpen);
    void (*hwClearLowLatencySwitchStatus)(const STACK::RawAddress &addr);
    void (*setBtPower)(int power);
    void (*hwSetShareAudioStatus)(bool enable, const STACK::RawAddress &shareAddr,
        const STACK::RawAddress &activeAddr);
    bool (*hwBtifIsA2dpShareSupported)(const STACK::RawAddress &activeAddress);
    void (*setBtFreezeApp)(int uid, bool isFreeze);
    void (*clearBtFreezeApp)(int uid);
    bool (*hwBtifIsA2dpShortTimeShareSupported)(const STACK::RawAddress &activeAddress);
    bool (*hwBtifIsShareAudioTwsDdmSupport)(const STACK::RawAddress &shareAddr);
    void (*hwBtifTwsSetShareAudioState)(int state);
    void (*hwBtifBtsnoopCapture)(bool captureFlag);
    bool (*checkInterOpMatchModelId)(const std::string &feature, const std::string &modelId);
    void (*hwUpdateBtAudioLatency)(const STACK::RawAddress& peer_address, int latency);
    int (*resumeBackGroundConnectionsInStr)();
    void (*setScanReportChannelSensorHub)(uint8_t filterIdx, uint8_t isToAp);
    bool (*isBondingOrSdp)();
    void (*hwSatelliteModeSwitch)(int handoverDirection);
    void (*hwBtifHfpScoOccupiedClean)(STACK::RawAddress& addr);
    bool (*isHwDualConnDevice)(const STACK::RawAddress &addr);
    bool (*needSyncPairedInfo)(std::vector<std::string> &pairedInfo);
    int (*a2dpOffloadSetSinkStreamStart)(const STACK::RawAddress &addr);
    int (*a2dpOffloadSetSinkStreamStop)(const STACK::RawAddress &addr);
    int (*a2dpOffloadGetSinkStreamEncodingPath)(const STACK::RawAddress &addr,
        const std::vector<HwBtA2dpSteamSessionInfo> &sessionsInfo, bool needStart);
    uint32_t (*a2dpOffloadGetLatency)();
    bool (*a2dpOffloadIsRunning)(const STACK::RawAddress &addr);
    void (*hwSetAdvIndReportFlag)(int filterIndex, bool advIndReport);
    void (*hwDeletAdvIndReportFlag)(int filterIndex);
    void (*notifyAudioRenderState)(bool isRenderActive);
    void (*hwSetSpatialAudioModeEnabled)(const bool enabled);
    void (*hwSetSpatialAudioHeadTrackingEnabled)(const bool enabled);
    void (*HwSetCollAudioEnableState)(bool state);
    bool (*interopMatch)(const uint16_t feature, const STACK::RawAddress &bdAddr);
    void (*changeCodec)(int32_t appType, int32_t sampleRate, uint8_t fastFlag);
    void (*hwSetActiveMode)(const STACK::RawAddress &addr);
    bool (*getA2dpOffloadCodecConfig)(const STACK::RawAddress &addr, A2dpOffloadConfigCallback &config);
    void (*hdapStartSession)(uint8_t sampleRate, uint8_t sampleLen, uint8_t channel);
    void (*hwSetAudioSourceType)(const uint8_t mode);
    void (*hwSetAdaptiveSwitchStatus)(const bool state);
    /* set connection attribution. supported attribution see the defination of HwConnAttrType */
    void (*hwConnAttrSet)(uint8_t transport, const STACK::RawAddress &addr, const HwConnAttr *attr);
    void (*hwVendorSpecificCommand)(SpecificCmdToHisi *cmd);
    void (*hwEnableHisiPcm)(bool state);
    void (*hwSetLocalVoiceCombineFeatureState)(const int featureState);
    int (*hwGetLocalVoiceCombineFeatureState)();
    bool (*hwGetRemoteVoiceCombineAbility)(const STACK::RawAddress &addr);
    void (*hwSetLocalVoiceCombineNnState)(const int state);
    void (*hwSetLeConnectionScanToFast)(bool enable);
    void (*hwSetVoiceCombineScenarioEnabled)(const STACK::RawAddress &addr, const bool isEnabled);
    bool (*hwIsSupportEchoDevice)(const STACK::RawAddress &addr);
    uint8_t (*hwGetLocalDeviceEchoType)();
    void (*hwSetMacId)(int state);
#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
    int (*hwSendChannelRssiReadCmd)(const STACK::RawAddress &addr, uint8_t transport);
    int (*hwGetUnionHisiJamChrInfo)(std::vector<uint32_t> &rssiIdx, std::vector<int64_t> &actualRssiValue,
        int64_t &timestamp);
    void (*hwSetBleScanEnhanceModeStatus)(bool isEnable);
    bool (*hwIsBleScanEnhanceModeEnable)();
    void (*hwAdjustTwsBitRateInBleScanEnhanceMode)(bool isSpeedDown);
    void (*hwSetBrTransControl)(bool isEnable, int workingInterval, int pendingInterval, int sleepingInterval);
    void (*hwSetBrLinkSniffMode)(bool isEnable, const STACK::RawAddress &addr, int8_t index);
    std::pair<uint32_t, uint32_t> (*hwGetBrTransTxRxRate)();
    void (*hwSendChannelScanEnableCmd)(bool isEnable, uint32_t interval);
    void (*hwSendReadTransPerformCmd)(uint16_t handle);
    bool (*hwGetTransPerformCache)(std::vector<int16_t> &bleAdvChannelRssi);
#endif
    int (*getCloudPushPowerLevel)(void);
#ifdef MULTI_A2DP_ENABLED
    /* Multi A2DP: bluetooth audio HDI (bluetooth_audio_session_service) is ready,
       bring up the per-peer A2DP source HAL. */
    void (*notifyA2dpPeerHdiReady)(const RawAddress &bdAddr);
#endif
    /** Reserved for future use. Must be NULL **/
    void* reservedProc[32 - 31];  // total fuction is 32,now used 31,reserved 32-31
} bthwif_interface_t;

} //__END_DECLS
#endif /* VENDOR_INCLUDE_BT_HF_H */
