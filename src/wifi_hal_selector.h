#ifndef WIFI_HAL_SELECTOR_H
#define WIFI_HAL_SELECTOR_H

#include "wifi_hal.h"
#include "wifi_hal_priv.h"
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <linux/rtnetlink.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cJSON.h"


// Declarations for nl_hal_* APIs used in nl_hal_selector
int nl_hal_createVAP(wifi_radio_index_t index, wifi_vap_info_map_t *map);
int nl_hal_getHalCapability(wifi_hal_capability_t *hal);
int nl_hal_init(void);
int nl_hal_pre_init(void);
int nl_hal_post_init(wifi_vap_info_map_t *vap_map);
int nl_hal_send_mgmt_frame_response(int ap_index, int type, int status, int status_code, uint8_t *frame, uint8_t *mac, int len, int rssi);
void nl_hal_deauth(int vap_index, int status, uint8_t *mac);
int nl_hal_getInterfaceMap(wifi_interface_name_idex_map_t *if_map, unsigned int max_entries, unsigned int *if_map_size);
int nl_hal_connect(int ap_index, wifi_bss_info_t *bss);
int nl_hal_setRadioOperatingParameters(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam);
int nl_hal_kickAssociatedDevice(int ap_index, mac_address_t mac);
int nl_hal_startScan(wifi_radio_index_t index, wifi_neighborScanMode_t scan_mode, int dwell_time, unsigned int num, unsigned int *chan_list);
int nl_hal_disconnect(int ap_index);
int nl_hal_getRadioVapInfoMap(wifi_radio_index_t index, wifi_vap_info_map_t *map);
int nl_hal_setApWpsButtonPush(int apIndex);
int nl_hal_setApWpsPin(int ap_index, char *wps_pin);
int nl_hal_setApWpsCancel(int ap_index);
int nl_hal_set_acs_keep_out_chans(wifi_radio_operationParam_t *wifi_radio_oper_param, int radioIndex);
int nl_hal_sendDataFrame(int vap_id, unsigned char *dmac, unsigned char *data_buff, int data_len, bool insert_llc, int protocal, int priority);
int nl_hal_addApAclDevice(int apIndex, mac_address_t DeviceMacAddress);
int nl_hal_delApAclDevice(int apIndex, mac_address_t DeviceMacAddress);
int nl_hal_delApAclDevices(int apIndex);
int nl_hal_steering_eventRegister(wifi_steering_eventCB_t event_cb);
int nl_hal_setRadioTransmitPower(wifi_radio_index_t radioIndex, unsigned int txpower);
int nl_hal_setBTMRequest(unsigned int apIndex, mac_address_t peerMac, wifi_BTMRequest_t *request);
int nl_hal_setRMBeaconRequest(unsigned int apIndex, mac_address_t peer_mac, wifi_BeaconRequest_t *in_req, unsigned char *out_DialogToken);
int nl_hal_cancelRMBeaconRequest(unsigned int apIndex, unsigned char dialogToken);
int nl_hal_configNeighborReports(unsigned int apIndex, bool enable, bool auto_resp);
int nl_hal_setNeighborReports(unsigned int apIndex, unsigned int numNeighborReports, wifi_NeighborReport_t *neighborReports);
void nl_hal_newApAssociatedDevice_callback_register(wifi_newApAssociatedDevice_callback func);
void nl_hal_apDisassociatedDevice_callback_register(wifi_device_disassociated_callback func);
void nl_hal_stamode_callback_register(wifi_stamode_callback func);
void nl_hal_apStatusCode_callback_register(wifi_apStatusCode_callback func);
void nl_hal_radiusEapFailure_callback_register(wifi_radiusEapFailure_callback func);
void nl_hal_radiusFallback_failover_callback_register(wifi_radiusFallback_failover_callback func);
void nl_hal_apDeAuthEvent_callback_register(wifi_device_deauthenticated_callback func);
void nl_hal_ap_max_client_rejection_callback_register(wifi_apMaxClientRejection_callback func);
int nl_hal_BTMQueryRequest_callback_register(unsigned int apIndex, wifi_BTMQueryRequest_callback btmQueryCallback, wifi_BTMResponse_callback btmResponseCallback);
int nl_hal_RMBeaconRequestCallbackRegister(unsigned int apIndex, wifi_RMBeaconReport_callback beaconReportCallback);
int nl_hal_RMBeaconRequestCallbackUnregister(unsigned int apIndex, wifi_RMBeaconReport_callback beaconReportCallback);
void nl_hal_scanResults_callback_register(wifi_scanResults_callback func);

typedef int (*hal_selector_createVAP_t)(wifi_radio_index_t index, wifi_vap_info_map_t *map);
typedef int (*halselector_getHalCap_t)(wifi_hal_capability_t *hal);
typedef int (*hal_selector_init_t)();
typedef int (*hal_selector_pre_init_t)();
typedef int (*hal_selector_post_init_t)(wifi_vap_info_map_t *vap_map);
typedef int (*hal_selector_send_mgmt_frame_response_t)(int ap_index, int type, int status, int status_code, uint8_t *frame, uint8_t *mac, int len, int rssi);
typedef void (*hal_selector_deauth_t)(int vap_index, int status, uint8_t *mac);
typedef int (*hal_selector_getInterfaceMap_t)(wifi_interface_name_idex_map_t *if_map, unsigned int max_entries, unsigned int *if_map_size);
typedef int (*hal_selector_connect_t)(int ap_index, wifi_bss_info_t *bss);
typedef int (*hal_selector_setRadioOperatingParameters_t)(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam);
typedef int (*hal_selector_kickAssociatedDevice_t)(int ap_index, mac_address_t mac);
typedef int (*hal_selector_startScan_t)(wifi_radio_index_t index, wifi_neighborScanMode_t scan_mode, int dwell_time, unsigned int num, unsigned int *chan_list);
typedef int (*hal_selector_disconnect_t)(int ap_index);
typedef int (*hal_selector_getRadioVapInfoMap_t)(wifi_radio_index_t index, wifi_vap_info_map_t *map);
typedef int (*hal_selector_setApWpsButtonPush_t)(int apIndex);
typedef int (*hal_selector_setApWpsPin_t)(int ap_index, char *wps_pin);
typedef int (*hal_selector_setApWpsCancel_t)(int ap_index);
typedef int (*hal_selector_set_acs_keep_out_chans_t)(wifi_radio_operationParam_t *wifi_radio_oper_param, int radioIndex);
typedef int (*hal_selector_sendDataFrame_t)(int vap_id, unsigned char *dmac, unsigned char *data_buff, int data_len, bool insert_llc, int protocal, int priority);
#ifdef WIFI_HAL_VERSION_3_PHASE2
typedef int (*hal_selector_addApAclDevice_t)(int apIndex, mac_address_t DeviceMacAddress);
typedef int (*hal_selector_delApAclDevice_t)(int apIndex, mac_address_t DeviceMacAddress);
#else
typedef int (*hal_selector_addApAclDevice_t)(int apIndex, char *DeviceMacAddress);
typedef int (*hal_selector_delApAclDevice_t)(int apIndex, char *DeviceMacAddress);
#endif
typedef int (*hal_selector_delApAclDevices_t)(int apIndex);
typedef int (*hal_selector_steering_eventRegister_t)(wifi_steering_eventCB_t event_cb);
typedef int (*hal_selector_setRadioTransmitPower_t)(wifi_radio_index_t radioIndex, unsigned int txpower);
typedef int (*hal_selector_getRadioTransmitPower_t)(int radioIndex, unsigned long *tx_power);
typedef int (*hal_selector_startNeighborScan_t)(int apIndex, wifi_neighborScanMode_t scan_mode, int dwell_time, unsigned int chan_num, unsigned int *chan_list);
typedef int (*hal_selector_getNeighboringWiFiStatus_t)(int radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array, unsigned int *output_array_size);
typedef int (*hal_selector_getNeighboringWiFiStatus_test_t)(int radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array, unsigned int *output_array_size);
typedef int (*hal_selector_setBTMRequest_t)(unsigned int apIndex, mac_address_t peerMac, wifi_BTMRequest_t *request);
typedef int (*hal_selector_setRMBeaconRequest_t)(unsigned int apIndex, mac_address_t peer_mac, wifi_BeaconRequest_t *in_req, unsigned char *out_DialogToken);
typedef int (*hal_selector_cancelRMBeaconRequest_t)(unsigned int apIndex, unsigned char dialogToken);
typedef int (*hal_selector_configNeighborReports_t)(unsigned int apIndex, bool enable, bool auto_resp);
typedef int (*hal_selector_setNeighborReports_t)(unsigned int apIndex, unsigned int numNeighborReports, wifi_NeighborReport_t *neighborReports);
typedef void (*hal_selector_newApAssociatedDevice_callback_register_t)(wifi_newApAssociatedDevice_callback func);
typedef void (*hal_selector_apDisassociatedDevice_callback_register_t)(wifi_device_disassociated_callback func);
typedef void (*hal_selector_stamode_callback_register_t)(wifi_stamode_callback func);
typedef void (*hal_selector_apStatusCode_callback_register_t)(wifi_apStatusCode_callback func);
typedef void (*hal_selector_radiusEapFailure_callback_register_t)(wifi_radiusEapFailure_callback func);
typedef void (*hal_selector_radiusFallback_failover_callback_register_t)(wifi_radiusFallback_failover_callback func);
typedef void (*hal_selector_apDeAuthEvent_callback_register_t)(wifi_device_deauthenticated_callback func);
typedef void (*hal_selector_ap_max_client_rejection_callback_register_t)(wifi_apMaxClientRejection_callback func);
typedef int (*hal_selector_BTMQueryRequest_callback_register_t)(unsigned int apIndex, wifi_BTMQueryRequest_callback btmQueryCallback, wifi_BTMResponse_callback btmResponseCallback);
typedef int (*hal_selector_RMBeaconRequestCallbackRegister_t)(unsigned int apIndex, wifi_RMBeaconReport_callback beaconReportCallback);
typedef int (*hal_selector_RMBeaconRequestCallbackUnregister_t)(unsigned int apIndex, wifi_RMBeaconReport_callback beaconReportCallback);
typedef void (*hal_selector_scanResults_callback_register_t)(wifi_scanResults_callback func);
typedef int (*hal_selector_getRadioTemperature_t)(wifi_radio_index_t radioIndex,
    wifi_radioTemperature_t *radioPhyTemperature);
typedef int (*hal_selector_setApMacAddressControlMode_t)(uint32_t apIndex, uint32_t mac_filter_mode);   

typedef struct {
    hal_selector_createVAP_t hal_selector_createVAP;
    halselector_getHalCap_t hal_selector_getHalCap;
    hal_selector_init_t hal_selector_init;
    hal_selector_pre_init_t hal_selector_pre_init;
    hal_selector_post_init_t hal_selector_post_init;
    hal_selector_send_mgmt_frame_response_t hal_selector_send_mgmt_frame_response;
    hal_selector_deauth_t hal_selector_deauth;
    hal_selector_getInterfaceMap_t hal_selector_getInterfaceMap;
    hal_selector_connect_t hal_selector_connect;
    hal_selector_setRadioOperatingParameters_t hal_selector_setRadioOperatingParameters;
    hal_selector_kickAssociatedDevice_t hal_selector_kickAssociatedDevice;
    hal_selector_startScan_t hal_selector_startScan;
    hal_selector_disconnect_t hal_selector_disconnect;
    hal_selector_getRadioVapInfoMap_t hal_selector_getRadioVapInfoMap;
    hal_selector_setApWpsButtonPush_t hal_selector_setApWpsButtonPush;
    hal_selector_setApWpsPin_t hal_selector_setApWpsPin;
    hal_selector_setApWpsCancel_t hal_selector_setApWpsCancel;
    hal_selector_set_acs_keep_out_chans_t hal_selector_set_acs_keep_out_chans;
    hal_selector_sendDataFrame_t hal_selector_sendDataFrame;
#ifdef WIFI_HAL_VERSION_3_PHASE2
    hal_selector_addApAclDevice_t hal_selector_addApAclDevice;
    hal_selector_delApAclDevice_t hal_selector_delApAclDevice;
#else
    hal_selector_addApAclDevice_t hal_selector_addApAclDevice;
    hal_selector_delApAclDevice_t hal_selector_delApAclDevice;
#endif
    hal_selector_delApAclDevices_t hal_selector_delApAclDevices;
    hal_selector_steering_eventRegister_t hal_selector_steering_eventRegister;
    hal_selector_setRadioTransmitPower_t hal_selector_setRadioTransmitPower;
    hal_selector_getRadioTransmitPower_t hal_selector_getRadioTransmitPower;
    hal_selector_startNeighborScan_t hal_selector_startNeighborScan;
    hal_selector_getNeighboringWiFiStatus_t hal_selector_getNeighboringWiFiStatus;
    hal_selector_getNeighboringWiFiStatus_test_t hal_selector_getNeighboringWiFiStatus_test;
    hal_selector_setBTMRequest_t hal_selector_setBTMRequest;
    hal_selector_setRMBeaconRequest_t hal_selector_setRMBeaconRequest;
    hal_selector_cancelRMBeaconRequest_t hal_selector_cancelRMBeaconRequest;
    hal_selector_configNeighborReports_t hal_selector_configNeighborReports;
    hal_selector_setNeighborReports_t hal_selector_setNeighborReports;
    hal_selector_newApAssociatedDevice_callback_register_t hal_selector_newApAssociatedDevice_callback_register;
    hal_selector_apDisassociatedDevice_callback_register_t hal_selector_apDisassociatedDevice_callback_register;
    hal_selector_stamode_callback_register_t hal_selector_stamode_callback_register;
    hal_selector_apStatusCode_callback_register_t hal_selector_apStatusCode_callback_register;
    hal_selector_radiusEapFailure_callback_register_t hal_selector_radiusEapFailure_callback_register;
    hal_selector_radiusFallback_failover_callback_register_t hal_selector_radiusFallback_failover_callback_register;
    hal_selector_apDeAuthEvent_callback_register_t hal_selector_apDeAuthEvent_callback_register;
    hal_selector_ap_max_client_rejection_callback_register_t hal_selector_ap_max_client_rejection_callback_register;
    hal_selector_BTMQueryRequest_callback_register_t hal_selector_BTMQueryRequest_callback_register;
    hal_selector_RMBeaconRequestCallbackRegister_t hal_selector_RMBeaconRequestCallbackRegister;
    hal_selector_RMBeaconRequestCallbackUnregister_t hal_selector_RMBeaconRequestCallbackUnregister;
    hal_selector_scanResults_callback_register_t hal_selector_scanResults_callback_register;
    hal_selector_getRadioTemperature_t hal_selector_getRadioTemperature;
    hal_selector_setApMacAddressControlMode_t hal_selector_setApMacAddressControlMode;
} wifi_hal_selector_info_t;

#endif //WIFI_HAL_SELECTOR_H