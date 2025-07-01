#include "wifi_hal_selector.h"

#define HAL_SELECTOR_FILE "/nvram/wifiHalSelector.json"

const wifi_hal_selector_info_t vendor_hal_selector = {
    .hal_selector_createVAP = wifi_test_createVAP,
    .hal_selector_getHalCap = wifi_test_getHalCapability,
    .hal_selector_init = wifi_init_test,
    .hal_selector_pre_init = NULL,
    .hal_selector_post_init = NULL,
    .hal_selector_send_mgmt_frame_response = NULL,
    .hal_selector_deauth = NULL,
    .hal_selector_getInterfaceMap = NULL,
    .hal_selector_connect = NULL,
    .hal_selector_setRadioOperatingParameters = NULL,
    .hal_selector_kickAssociatedDevice = NULL,
    .hal_selector_startScan = NULL,
    .hal_selector_disconnect = NULL,
    .hal_selector_getRadioVapInfoMap = NULL,
    .hal_selector_setApWpsButtonPush = NULL,
    .hal_selector_setApWpsPin = NULL,
    .hal_selector_setApWpsCancel = NULL,
    .hal_selector_set_acs_keep_out_chans = NULL,
    .hal_selector_sendDataFrame = NULL,
    .hal_selector_addApAclDevice = NULL,
    .hal_selector_delApAclDevice = NULL,
    .hal_selector_delApAclDevices = NULL,
    .hal_selector_steering_eventRegister = NULL,
    .hal_selector_setRadioTransmitPower = NULL,
    .hal_selector_getRadioTransmitPower = NULL,
    .hal_selector_startNeighborScan = NULL,
    .hal_selector_getNeighboringWiFiStatus = NULL,
    .hal_selector_setBTMRequest = NULL,
    .hal_selector_setRMBeaconRequest = NULL,
    .hal_selector_cancelRMBeaconRequest = NULL,
    .hal_selector_configNeighborReports = NULL,
    .hal_selector_setNeighborReports = NULL,
    .hal_selector_newApAssociatedDevice_callback_register = NULL,
    .hal_selector_apDisassociatedDevice_callback_register = NULL,
    .hal_selector_stamode_callback_register = NULL,
    .hal_selector_apStatusCode_callback_register = NULL,
    .hal_selector_radiusEapFailure_callback_register = NULL,
    .hal_selector_radiusFallback_failover_callback_register = NULL,
    .hal_selector_apDeAuthEvent_callback_register = NULL,
    .hal_selector_ap_max_client_rejection_callback_register = NULL,
    .hal_selector_BTMQueryRequest_callback_register = NULL,
    .hal_selector_RMBeaconRequestCallbackRegister = NULL,
    .hal_selector_RMBeaconRequestCallbackUnregister = NULL,
    .hal_selector_scanResults_callback_register = NULL,
    .hal_selector_getRadioTemperature = NULL,
    .hal_selector_setApMacAddressControlMode = NULL,
};

const wifi_hal_selector_info_t nl_hal_selector = {
    .hal_selector_createVAP = nl_hal_createVAP,
    .hal_selector_getHalCap = nl_hal_getHalCapability,
    .hal_selector_init = nl_hal_init,
    .hal_selector_pre_init = nl_hal_pre_init,
    .hal_selector_post_init = nl_hal_post_init,
    .hal_selector_send_mgmt_frame_response = nl_hal_send_mgmt_frame_response,
    .hal_selector_deauth = nl_hal_deauth,
    .hal_selector_getInterfaceMap = nl_hal_getInterfaceMap,
    .hal_selector_connect = nl_hal_connect,
    .hal_selector_setRadioOperatingParameters = nl_hal_setRadioOperatingParameters,
    .hal_selector_kickAssociatedDevice = nl_hal_kickAssociatedDevice,
    .hal_selector_startScan = nl_hal_startScan,
    .hal_selector_disconnect = nl_hal_disconnect,
    .hal_selector_getRadioVapInfoMap = nl_hal_getRadioVapInfoMap,
    .hal_selector_setApWpsButtonPush = nl_hal_setApWpsButtonPush,
    .hal_selector_setApWpsPin = nl_hal_setApWpsPin,
    .hal_selector_setApWpsCancel = nl_hal_setApWpsCancel,
    .hal_selector_set_acs_keep_out_chans = nl_hal_set_acs_keep_out_chans,
    .hal_selector_sendDataFrame = nl_hal_sendDataFrame,
    .hal_selector_addApAclDevice = nl_hal_addApAclDevice,
    .hal_selector_delApAclDevice = nl_hal_delApAclDevice,
    .hal_selector_delApAclDevices = nl_hal_delApAclDevices,
    .hal_selector_steering_eventRegister = nl_hal_steering_eventRegister,
    .hal_selector_setRadioTransmitPower = nl_hal_setRadioTransmitPower,
    .hal_selector_getRadioTransmitPower = NULL,
    .hal_selector_startNeighborScan = wifi_startNeighborScan,
    .hal_selector_getNeighboringWiFiStatus = wifi_getNeighboringWiFiStatus,
    .hal_selector_setBTMRequest = nl_hal_setBTMRequest,
    .hal_selector_setRMBeaconRequest = nl_hal_setRMBeaconRequest,
    .hal_selector_cancelRMBeaconRequest = nl_hal_cancelRMBeaconRequest,
    .hal_selector_configNeighborReports = nl_hal_configNeighborReports,
    .hal_selector_setNeighborReports = nl_hal_setNeighborReports,
    .hal_selector_newApAssociatedDevice_callback_register = nl_hal_newApAssociatedDevice_callback_register,
    .hal_selector_apDisassociatedDevice_callback_register = nl_hal_apDisassociatedDevice_callback_register,
    .hal_selector_stamode_callback_register = nl_hal_stamode_callback_register,
    .hal_selector_apStatusCode_callback_register = nl_hal_apStatusCode_callback_register,
    .hal_selector_radiusEapFailure_callback_register = nl_hal_radiusEapFailure_callback_register,
    .hal_selector_radiusFallback_failover_callback_register = nl_hal_radiusFallback_failover_callback_register,
    .hal_selector_apDeAuthEvent_callback_register = nl_hal_apDeAuthEvent_callback_register,
    .hal_selector_ap_max_client_rejection_callback_register = nl_hal_ap_max_client_rejection_callback_register,
    .hal_selector_BTMQueryRequest_callback_register = nl_hal_BTMQueryRequest_callback_register,
    .hal_selector_RMBeaconRequestCallbackRegister = nl_hal_RMBeaconRequestCallbackRegister,
    .hal_selector_RMBeaconRequestCallbackUnregister = nl_hal_RMBeaconRequestCallbackUnregister,
    .hal_selector_scanResults_callback_register = nl_hal_scanResults_callback_register,
    .hal_selector_getRadioTemperature = NULL,
    .hal_selector_setApMacAddressControlMode = NULL,
};

wifi_hal_selector_info_t g_hal_selector;

void init_hal_selector()
{
    bool use_nl = false;
    long filesize = 0;

    memset(g_hal_selector, 0, sizeof(wifi_hal_selector_info_t));
    FILE *file = fopen(HAL_SELECTOR_FILE, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);

    // Allocate buffer and read contents
    char *buffer = (char *)malloc(filesize + 1);
    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0';
    fclose(file);

    // Parse JSON
    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        fprintf(stderr, "Error parsing JSON\n");
        free(buffer);
        return;
    }

    // Get version
    cJSON *version = cJSON_GetObjectItem(root, "version");
    if (cJSON_IsString(version)) {
        printf("Version: %s\n", version->valuestring);
    }

    // Get SupportedAPIs
    cJSON *apis = cJSON_GetObjectItem(root, "SupportedAPIs");
    if (cJSON_IsArray(apis)) {
        int size = cJSON_GetArraySize(apis);
        for (int i = 0; i < size; i++) {
            cJSON *api = cJSON_GetArrayItem(apis, i);
            cJSON *name = cJSON_GetObjectItem(api, "name");
            cJSON *use_nl = cJSON_GetObjectItem(api, "use_nl");
            int use_nl_val = 0;
            if (cJSON_IsBool(use_nl)) {
                use_nl_val = use_nl->valueint;
            }
            if (cJSON_IsString(name)) {
                if (!strncmp(name->valuestring, "createVAP", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_createVAP = use_nl_val ?
                        nl_hal_selector.hal_selector_createVAP :
                        vendor_hal_selector.hal_selector_createVAP;
                } else if (!strncmp(name->valuestring, "getHalCapability",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_getHalCap = use_nl_val ?
                        nl_hal_selector.hal_selector_getHalCap :
                        vendor_hal_selector.hal_selector_getHalCap;
                } else if (!strncmp(name->valuestring, "init", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_init = use_nl_val ?
                        nl_hal_selector.hal_selector_init :
                        vendor_hal_selector.hal_selector_init;
                } else if (!strncmp(name->valuestring, "pre_init", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_pre_init = use_nl_val ?
                        nl_hal_selector.hal_selector_pre_init :
                        vendor_hal_selector.hal_selector_pre_init;
                } else if (!strncmp(name->valuestring, "post_init", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_post_init = use_nl_val ?
                        nl_hal_selector.hal_selector_post_init :
                        vendor_hal_selector.hal_selector_post_init;
                } else if (!strncmp(name->valuestring, "send_mgmt_frame_response",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_send_mgmt_frame_response = use_nl_val ?
                        nl_hal_selector.hal_selector_send_mgmt_frame_response :
                        vendor_hal_selector.hal_selector_send_mgmt_frame_response;
                } else if (!strncmp(name->valuestring, "deauth", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_deauth = use_nl_val ?
                        nl_hal_selector.hal_selector_deauth :
                        vendor_hal_selector.hal_selector_deauth;
                } else if (!strncmp(name->valuestring, "getInterfaceMap",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_getInterfaceMap = use_nl_val ?
                        nl_hal_selector.hal_selector_getInterfaceMap :
                        vendor_hal_selector.hal_selector_getInterfaceMap;
                } else if (!strncmp(name->valuestring, "connect", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_connect = use_nl_val ?
                        nl_hal_selector.hal_selector_connect :
                        vendor_hal_selector.hal_selector_connect;
                } else if (!strncmp(name->valuestring, "setRadioOperatingParameters",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setRadioOperatingParameters = use_nl_val ?
                        nl_hal_selector.hal_selector_setRadioOperatingParameters :
                        vendor_hal_selector.hal_selector_setRadioOperatingParameters;
                } else if (!strncmp(name->valuestring, "kickAssociatedDevice",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_kickAssociatedDevice = use_nl_val ?
                        nl_hal_selector.hal_selector_kickAssociatedDevice :
                        vendor_hal_selector.hal_selector_kickAssociatedDevice;
                } else if (!strncmp(name->valuestring, "startScan", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_startScan = use_nl_val ?
                        nl_hal_selector.hal_selector_startScan :
                        vendor_hal_selector.hal_selector_startScan;
                } else if (!strncmp(name->valuestring, "disconnect", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_disconnect = use_nl_val ?
                        nl_hal_selector.hal_selector_disconnect :
                        vendor_hal_selector.hal_selector_disconnect;
                } else if (!strncmp(name->valuestring, "getRadioVapInfoMap",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_getRadioVapInfoMap = use_nl_val ?
                        nl_hal_selector.hal_selector_getRadioVapInfoMap :
                        vendor_hal_selector.hal_selector_getRadioVapInfoMap;
                } else if (!strncmp(name->valuestring, "setApWpsButtonPush",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setApWpsButtonPush = use_nl_val ?
                        nl_hal_selector.hal_selector_setApWpsButtonPush :
                        vendor_hal_selector.hal_selector_setApWpsButtonPush;
                } else if (!strncmp(name->valuestring, "setApWpsPin", strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setApWpsPin = use_nl_val ?
                        nl_hal_selector.hal_selector_setApWpsPin :
                        vendor_hal_selector.hal_selector_setApWpsPin;
                } else if (!strncmp(name->valuestring, "setApWpsCancel",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setApWpsCancel = use_nl_val ?
                        nl_hal_selector.hal_selector_setApWpsCancel :
                        vendor_hal_selector.hal_selector_setApWpsCancel;
                } else if (!strncmp(name->valuestring, "set_acs_keep_out_chans",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_set_acs_keep_out_chans = use_nl_val ?
                        nl_hal_selector.hal_selector_set_acs_keep_out_chans :
                        vendor_hal_selector.hal_selector_set_acs_keep_out_chans;
                } else if (!strncmp(name->valuestring, "sendDataFrame",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_sendDataFrame = use_nl_val ?
                        nl_hal_selector.hal_selector_sendDataFrame :
                        vendor_hal_selector.hal_selector_sendDataFrame;
                } else if (!strncmp(name->valuestring, "addApAclDevice",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_addApAclDevice = use_nl_val ?
                        nl_hal_selector.hal_selector_addApAclDevice :
                        vendor_hal_selector.hal_selector_addApAclDevice;
                } else if (!strncmp(name->valuestring, "delApAclDevice",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_delApAclDevice = use_nl_val ?
                        nl_hal_selector.hal_selector_delApAclDevice :
                        vendor_hal_selector.hal_selector_delApAclDevice;
                } else if (!strncmp(name->valuestring, "delApAclDevices",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_delApAclDevices = use_nl_val ?
                        nl_hal_selector.hal_selector_delApAclDevices :
                        vendor_hal_selector.hal_selector_delApAclDevices;
                } else if (!strncmp(name->valuestring, "steering_eventRegister",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_steering_eventRegister = use_nl_val ?
                        nl_hal_selector.hal_selector_steering_eventRegister :
                        vendor_hal_selector.hal_selector_steering_eventRegister;
                } else if (!strncmp(name->valuestring, "setRadioTransmitPower",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setRadioTransmitPower = use_nl_val ?
                        nl_hal_selector.hal_selector_setRadioTransmitPower :
                        vendor_hal_selector.hal_selector_setRadioTransmitPower;
                } else if (!strncmp(name->valuestring, "getRadioTransmitPower",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_getRadioTransmitPower = use_nl_val ?
                        nl_hal_selector.hal_selector_getRadioTransmitPower :
                        vendor_hal_selector.hal_selector_getRadioTransmitPower;
                } else if (!strncmp(name->valuestring, "startNeighborScan",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_startNeighborScan = use_nl_val ?
                        nl_hal_selector.hal_selector_startNeighborScan :
                        vendor_hal_selector.hal_selector_startNeighborScan;
                } else if (!strncmp(name->valuestring, "getNeighboringWiFiStatus_test",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_getNeighboringWiFiStatus_test = use_nl_val ?
                        nl_hal_selector.hal_selector_getNeighboringWiFiStatus_test :
                        vendor_hal_selector.hal_selector_getNeighboringWiFiStatus_test;
                } else if (!strncmp(name->valuestring, "getNeighboringWiFiStatus",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_getNeighboringWiFiStatus = use_nl_val ?
                        nl_hal_selector.hal_selector_getNeighboringWiFiStatus :
                        vendor_hal_selector.hal_selector_getNeighboringWiFiStatus;
                } else if (!strncmp(name->valuestring, "setBTMRequest",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setBTMRequest = use_nl_val ?
                        nl_hal_selector.hal_selector_setBTMRequest :
                        vendor_hal_selector.hal_selector_setBTMRequest;
                } else if (!strncmp(name->valuestring, "setRMBeaconRequest",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setRMBeaconRequest = use_nl_val ?
                        nl_hal_selector.hal_selector_setRMBeaconRequest :
                        vendor_hal_selector.hal_selector_setRMBeaconRequest;
                } else if (!strncmp(name->valuestring, "cancelRMBeaconRequest",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_cancelRMBeaconRequest = use_nl_val ?
                        nl_hal_selector.hal_selector_cancelRMBeaconRequest :
                        vendor_hal_selector.hal_selector_cancelRMBeaconRequest;
                } else if (!strncmp(name->valuestring, "configNeighborReports",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_configNeighborReports = use_nl_val ?
                        nl_hal_selector.hal_selector_configNeighborReports :
                        vendor_hal_selector.hal_selector_configNeighborReports;
                } else if (!strncmp(name->valuestring, "setNeighborReports",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_setNeighborReports = use_nl_val ?
                        nl_hal_selector.hal_selector_setNeighborReports :
                        vendor_hal_selector.hal_selector_setNeighborReports;
                } else if (!strncmp(name->valuestring, "newApAssociatedDevice_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_newApAssociatedDevice_callback_register =
                        use_nl_val ?
                        nl_hal_selector.hal_selector_newApAssociatedDevice_callback_register :
                        vendor_hal_selector.hal_selector_newApAssociatedDevice_callback_register;
                } else if (!strncmp(name->valuestring, "apDisassociatedDevice_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_apDisassociatedDevice_callback_register =
                        use_nl_val ?
                        nl_hal_selector.hal_selector_apDisassociatedDevice_callback_register :
                        vendor_hal_selector.hal_selector_apDisassociatedDevice_callback_register;
                } else if (!strncmp(name->valuestring, "stamode_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_stamode_callback_register = use_nl_val ?
                        nl_hal_selector.hal_selector_stamode_callback_register :
                        vendor_hal_selector.hal_selector_stamode_callback_register;
                } else if (!strncmp(name->valuestring, "apStatusCode_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_apStatusCode_callback_register = use_nl_val ?
                        nl_hal_selector.hal_selector_apStatusCode_callback_register :
                        vendor_hal_selector.hal_selector_apStatusCode_callback_register;
                } else if (!strncmp(name->valuestring, "radiusEapFailure_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_radiusEapFailure_callback_register = use_nl_val ?
                        nl_hal_selector.hal_selector_radiusEapFailure_callback_register :
                        vendor_hal_selector.hal_selector_radiusEapFailure_callback_register;
                } else if (!strncmp(name->valuestring, "radiusFallback_failover_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_radiusFallback_failover_callback_register =
                        use_nl_val ?
                        nl_hal_selector.hal_selector_radiusFallback_failover_callback_register :
                        vendor_hal_selector.hal_selector_radiusFallback_failover_callback_register;
                } else if (!strncmp(name->valuestring, "apDeAuthEvent_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_apDeAuthEvent_callback_register = use_nl_val ?
                        nl_hal_selector.hal_selector_apDeAuthEvent_callback_register :
                        vendor_hal_selector.hal_selector_apDeAuthEvent_callback_register;
                } else if (!strncmp(name->valuestring, "ap_max_client_rejection_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_ap_max_client_rejection_callback_register =
                        use_nl_val ?
                        nl_hal_selector.hal_selector_ap_max_client_rejection_callback_register :
                        vendor_hal_selector.hal_selector_ap_max_client_rejection_callback_register;
                } else if (!strncmp(name->valuestring, "BTMQueryRequest_callback_register",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_BTMQueryRequest_callback_register = use_nl_val ?
                        nl_hal_selector.hal_selector_BTMQueryRequest_callback_register :
                        vendor_hal_selector.hal_selector_BTMQueryRequest_callback_register;
                } else if (!strncmp(name->valuestring, "RMBeaconRequestCallbackRegister",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_RMBeaconRequestCallbackRegister = use_nl_val ?
                        nl_hal_selector.hal_selector_RMBeaconRequestCallbackRegister :
                        vendor_hal_selector.hal_selector_RMBeaconRequestCallbackRegister;
                } else if (!strncmp(name->valuestring, "RMBeaconRequestCallbackUnregister",
                               strlen(name->valuestring))) {
                    g_hal_selector.hal_selector_RMBeaconRequestCallbackUnregister = use_nl_val ?
                        nl_hal_selector.hal_selector_RMBeaconRequestCallbackUnregister :
                        vendor_hal_selector.hal_selector_RMBeaconRequestCallbackUnregister;
                }
            }
        }
    }

    cJSON_Delete(root);
    free(buffer);
}

INT wifi_hal_createVAP(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
    if (g_hal_selector.hal_selector_createVAP != NULL) {
        return g_hal_selector.hal_selector_createVAP(index, map);
    }

    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_getHalCapability(wifi_hal_capability_t *hal)
{
    if (g_hal_selector.hal_selector_getHalCap != NULL) {
        return g_hal_selector.hal_selector_getHalCap(hal);
    }

    wifi_hal_error_print("%s:%d: NULL Function Pointer \n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_init() {
    if (g_hal_selector.hal_selector_init != NULL) {
        return g_hal_selector.hal_selector_init();
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_pre_init() {
    if (g_hal_selector.hal_selector_pre_init != NULL) {
        return g_hal_selector.hal_selector_pre_init();
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_post_init(wifi_vap_info_map_t *vap_map) {
    if (g_hal_selector.hal_selector_post_init != NULL) {
        return g_hal_selector.hal_selector_post_init(vap_map);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_send_mgmt_frame_response(int ap_index, int type, int status, int status_code, uint8_t *frame, uint8_t *mac, int len, int rssi) {
    if (g_hal_selector.hal_selector_send_mgmt_frame_response != NULL) {
        return g_hal_selector.hal_selector_send_mgmt_frame_response(ap_index, type, status, status_code, frame, mac, len, rssi);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

void wifi_hal_deauth(int vap_index, int status, uint8_t *mac) {
    if (g_hal_selector.hal_selector_deauth != NULL) {
        g_hal_selector.hal_selector_deauth(vap_index, status, mac);
    }
}

INT wifi_hal_getInterfaceMap(wifi_interface_name_idex_map_t *if_map, unsigned int max_entries, unsigned int *if_map_size) {
    if (g_hal_selector.hal_selector_getInterfaceMap != NULL) {
        return g_hal_selector.hal_selector_getInterfaceMap(if_map, max_entries, if_map_size);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_connect(INT ap_index, wifi_bss_info_t *bss) {
    if (g_hal_selector.hal_selector_connect != NULL) {
        return g_hal_selector.hal_selector_connect(ap_index, bss);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setRadioOperatingParameters(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam) {
    if (g_hal_selector.hal_selector_setRadioOperatingParameters != NULL) {
        return g_hal_selector.hal_selector_setRadioOperatingParameters(index, operationParam);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_kickAssociatedDevice(INT ap_index, mac_address_t mac) {
    if (g_hal_selector.hal_selector_kickAssociatedDevice != NULL) {
        return g_hal_selector.hal_selector_kickAssociatedDevice(ap_index, mac);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_startScan(wifi_radio_index_t index, wifi_neighborScanMode_t scan_mode, INT dwell_time, UINT num, UINT *chan_list) {
    if (g_hal_selector.hal_selector_startScan != NULL) {
        return g_hal_selector.hal_selector_startScan(index, scan_mode, dwell_time, num, chan_list);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_disconnect(INT ap_index) {
    if (g_hal_selector.hal_selector_disconnect != NULL) {
        return g_hal_selector.hal_selector_disconnect(ap_index);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_getRadioVapInfoMap(wifi_radio_index_t index, wifi_vap_info_map_t *map) {
    if (g_hal_selector.hal_selector_getRadioVapInfoMap != NULL) {
        return g_hal_selector.hal_selector_getRadioVapInfoMap(index, map);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setApWpsButtonPush(INT apIndex) {
    if (g_hal_selector.hal_selector_setApWpsButtonPush != NULL) {
        return g_hal_selector.hal_selector_setApWpsButtonPush(apIndex);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setApWpsPin(INT ap_index, char *wps_pin) {
    if (g_hal_selector.hal_selector_setApWpsPin != NULL) {
        return g_hal_selector.hal_selector_setApWpsPin(ap_index, wps_pin);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setApWpsCancel(INT ap_index) {
    if (g_hal_selector.hal_selector_setApWpsCancel != NULL) {
        return g_hal_selector.hal_selector_setApWpsCancel(ap_index);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_set_acs_keep_out_chans(wifi_radio_operationParam_t *wifi_radio_oper_param, int radioIndex) {
    if (g_hal_selector.hal_selector_set_acs_keep_out_chans != NULL) {
        return g_hal_selector.hal_selector_set_acs_keep_out_chans(wifi_radio_oper_param, radioIndex);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_sendDataFrame(int vap_id, unsigned char *dmac, unsigned char *data_buff, int data_len, BOOL insert_llc, int protocal, int priority) {
    if (g_hal_selector.hal_selector_sendDataFrame != NULL) {
        return g_hal_selector.hal_selector_sendDataFrame(vap_id, dmac, data_buff, data_len, insert_llc, protocal, priority);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_addApAclDevice(INT apIndex, mac_address_t DeviceMacAddress) {
    if (g_hal_selector.hal_selector_addApAclDevice != NULL) {
        return g_hal_selector.hal_selector_addApAclDevice(apIndex, DeviceMacAddress);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_delApAclDevice(INT apIndex, mac_address_t DeviceMacAddress) {
    if (g_hal_selector.hal_selector_delApAclDevice != NULL) {
        return g_hal_selector.hal_selector_delApAclDevice(apIndex, DeviceMacAddress);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_delApAclDevices(INT apIndex) {
    if (g_hal_selector.hal_selector_delApAclDevices != NULL) {
        return g_hal_selector.hal_selector_delApAclDevices(apIndex);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_steering_eventRegister(wifi_steering_eventCB_t event_cb) {
    if (g_hal_selector.hal_selector_steering_eventRegister != NULL) {
        return g_hal_selector.hal_selector_steering_eventRegister(event_cb);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setRadioTransmitPower(wifi_radio_index_t radioIndex, uint txpower) {
    if (g_hal_selector.hal_selector_setRadioTransmitPower != NULL) {
        return g_hal_selector.hal_selector_setRadioTransmitPower(radioIndex, txpower);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_getRadioTransmitPower(INT radioIndex, ULONG *tx_power) {
    if (g_hal_selector.hal_selector_getRadioTransmitPower != NULL) {
        return g_hal_selector.hal_selector_getRadioTransmitPower(radioIndex, tx_power);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_startNeighborScan(INT apIndex, wifi_neighborScanMode_t scan_mode, INT dwell_time, UINT chan_num, UINT *chan_list) {
    if (g_hal_selector.hal_selector_startNeighborScan != NULL) {
        return g_hal_selector.hal_selector_startNeighborScan(apIndex, scan_mode, dwell_time, chan_num, chan_list);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_getNeighboringWiFiStatus(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array, UINT *output_array_size) {
    if (g_hal_selector.hal_selector_getNeighboringWiFiStatus != NULL) {
        return g_hal_selector.hal_selector_getNeighboringWiFiStatus(radioIndex, neighbor_ap_array, output_array_size);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_getNeighboringWiFiStatus_test(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array, UINT *output_array_size) {
    if (g_hal_selector.hal_selector_getNeighboringWiFiStatus_test != NULL) {
        return g_hal_selector.hal_selector_getNeighboringWiFiStatus_test(radioIndex, neighbor_ap_array, output_array_size);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setBTMRequest(UINT apIndex, mac_address_t peerMac, wifi_BTMRequest_t *request) {
    if (g_hal_selector.hal_selector_setBTMRequest != NULL) {
        return g_hal_selector.hal_selector_setBTMRequest(apIndex, peerMac, request);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setRMBeaconRequest(UINT apIndex, mac_address_t peer_mac, wifi_BeaconRequest_t *in_req, UCHAR *out_DialogToken) {
    if (g_hal_selector.hal_selector_setRMBeaconRequest != NULL) {
        return g_hal_selector.hal_selector_setRMBeaconRequest(apIndex, peer_mac, in_req, out_DialogToken);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_cancelRMBeaconRequest(UINT apIndex, UCHAR dialogToken) {
    if (g_hal_selector.hal_selector_cancelRMBeaconRequest != NULL) {
        return g_hal_selector.hal_selector_cancelRMBeaconRequest(apIndex, dialogToken);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_configNeighborReports(UINT apIndex, bool enable, bool auto_resp) {
    if (g_hal_selector.hal_selector_configNeighborReports != NULL) {
        return g_hal_selector.hal_selector_configNeighborReports(apIndex, enable, auto_resp);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_setNeighborReports(UINT apIndex, UINT numNeighborReports, wifi_NeighborReport_t *neighborReports) {
    if (g_hal_selector.hal_selector_setNeighborReports != NULL) {
        return g_hal_selector.hal_selector_setNeighborReports(apIndex, numNeighborReports, neighborReports);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

void wifi_hal_newApAssociatedDevice_callback_register(wifi_newApAssociatedDevice_callback func) {
    if (g_hal_selector.hal_selector_newApAssociatedDevice_callback_register != NULL) {
        g_hal_selector.hal_selector_newApAssociatedDevice_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_apDisassociatedDevice_callback_register(wifi_device_disassociated_callback func) {
    if (g_hal_selector.hal_selector_apDisassociatedDevice_callback_register != NULL) {
        g_hal_selector.hal_selector_apDisassociatedDevice_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_stamode_callback_register(wifi_stamode_callback func) {
    if (g_hal_selector.hal_selector_stamode_callback_register != NULL) {
        g_hal_selector.hal_selector_stamode_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_apStatusCode_callback_register(wifi_apStatusCode_callback func) {
    if (g_hal_selector.hal_selector_apStatusCode_callback_register != NULL) {
        g_hal_selector.hal_selector_apStatusCode_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_radiusEapFailure_callback_register(wifi_radiusEapFailure_callback func) {
    if (g_hal_selector.hal_selector_radiusEapFailure_callback_register != NULL) {
        g_hal_selector.hal_selector_radiusEapFailure_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_radiusFallback_failover_callback_register(wifi_radiusFallback_failover_callback func) {
    if (g_hal_selector.hal_selector_radiusFallback_failover_callback_register != NULL) {
        g_hal_selector.hal_selector_radiusFallback_failover_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_apDeAuthEvent_callback_register(wifi_device_deauthenticated_callback func) {
    if (g_hal_selector.hal_selector_apDeAuthEvent_callback_register != NULL) {
        g_hal_selector.hal_selector_apDeAuthEvent_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

void wifi_hal_ap_max_client_rejection_callback_register(wifi_apMaxClientRejection_callback func) {
    if (g_hal_selector.hal_selector_ap_max_client_rejection_callback_register != NULL) {
        g_hal_selector.hal_selector_ap_max_client_rejection_callback_register(func);
        return;
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return;
}

INT wifi_hal_BTMQueryRequest_callback_register(UINT apIndex, wifi_BTMQueryRequest_callback btmQueryCallback, wifi_BTMResponse_callback btmResponseCallback) {
    if (g_hal_selector.hal_selector_BTMQueryRequest_callback_register != NULL) {
        return g_hal_selector.hal_selector_BTMQueryRequest_callback_register(apIndex, btmQueryCallback, btmResponseCallback);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_RMBeaconRequestCallbackRegister(UINT apIndex, wifi_RMBeaconReport_callback beaconReportCallback) {
    if (g_hal_selector.hal_selector_RMBeaconRequestCallbackRegister != NULL) {
        return g_hal_selector.hal_selector_RMBeaconRequestCallbackRegister(apIndex, beaconReportCallback);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

INT wifi_hal_RMBeaconRequestCallbackUnregister(UINT apIndex, wifi_RMBeaconReport_callback beaconReportCallback) {
    if (g_hal_selector.hal_selector_RMBeaconRequestCallbackUnregister != NULL) {
        return g_hal_selector.hal_selector_RMBeaconRequestCallbackUnregister(apIndex, beaconReportCallback);
    }
    wifi_hal_error_print("%s:%d: NULL Function Pointer\n", __func__, __LINE__);
    return RETURN_ERR;
}

// Test API wrappers that call nl_hal_* implementations
INT wifi_test_createVAP(wifi_radio_index_t index, wifi_vap_info_map_t *map) {
    wifi_hal_dbg_print("%s:%d: Calling nl_hal_createVAP\n", __func__, __LINE__);
    return nl_hal_createVAP(index, map);
}

INT wifi_test_getHalCapability(wifi_hal_capability_t *hal) {
    wifi_hal_dbg_print("%s:%d: Calling nl_hal_getHalCapability\n", __func__, __LINE__);
    return nl_hal_getHalCapability(hal);
}

INT wifi_init_test() {
    wifi_hal_dbg_print("%s:%d: Calling nl_hal_init\n", __func__, __LINE__);
    return nl_hal_init();
}