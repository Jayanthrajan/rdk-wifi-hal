#include "wifi_hal_selector.h"
#include "cJSON.h"
#include "wifi_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HAL_SELECTOR_FILE "/nvram/wifiHalSelector.json"

const wifi_hal_selector_info_t vendor_hal_selector =  {
    .hal_selector_createVAP = wifi_test_createVAP,
    .hal_selector_getHalCap = wifi_test_getHalCapability,
    .hal_selector_init = wifi_init,
    .hal_selector_pre_init = NULL,
    .hal_selector_post_init = NULL,
    .hal_selector_send_mgmt_frame_response = NULL,
    .hal_selector_deauth = NULL,
    .hal_selector_getInterfaceMap = wifi_getInterfaceMap,
    .hal_selector_connect = NULL,
    .hal_selector_setRadioOperatingParameters = wifi_setRadioOperatingParameters,
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
    .hal_selector_getNeighboringWiFiStatus_test = NULL,
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
    .hal_selector_getNeighboringWiFiStatus_test = NULL,
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
            if (cJSON_IsBool(use_nl)) {
                use_nl = use_nl->valueint ? "true" : "false";
            }
            if (cJSON_IsString(name)) {
                if (!strncmp(name->valuestring), "createVAP", strlen(name->valuestring)) {
                    if (!use_nl) {
                        g_hal_selector.hal_selector_createVAP =
                            vendor_hal_selector.hal_selector_createVAP;
                    } else {
                        g_hal_selector.hal_selector_createVAP =
                            nl_hal_selector.hal_selector_createVAP;
                    }
                }

                if (!strncmp(name->valuestring, "getHalCapability", strlen(name->valuestring))) {
                    if (!use_nl) {
                        g_hal_selector.hal_selector_getHalCap =
                            vendor_hal_selector.hal_selector_getHalCap;
                    } else {
                        g_hal_selector.hal_selector_getHalCap =
                            nl_hal_selector.hal_selector_getHalCap;
                    }
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

    return RETURN_ERR;
}

INT wifi_hal_getHalCapability(wifi_hal_capability_t *hal)
{
    if (g_hal_selector.hal_selector_getHalCap != NULL) {
        return g_hal_selector.hal_selector_getHalCap(hal);
    }

    wifi_hal_error_print("%s:%d: NULL FUnction Pointer \n", __func__, __LINE__);
    return RETURN_ERR;
}
