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
#define SSID_STRING_LEN 64

#define CSI_MAX_DEVICES_BRCM 10
#define OUTPUT_STRING_LENGTH_16 16
#define OUTPUT_STRING_LENGTH_18 18
#define OUTPUT_STRING_LENGTH_32 32
#define OUTPUT_STRING_LENGTH_64 64
#define OUTPUT_STRING_LENGTH_128 128
#define OUTPUT_STRING_LENGTH_256 256
#define OUTPUT_STRING_LENGTH_512 512
#define OUTPUT_STRING_LENGTH_1024 1024
#define OUTPUT_STRING_LENGTH_2048 2048
#define IP_ADDR_LEN 45

#define HAL_RADIO_STARTING_INDEX 0
#define HAL_AP_STARTING_INDEX 0 /* ?? */
#define HAL_AP_NUM_APS_PER_RADIO 8

#define CSI_DELAY_PERIOD 100 /* CSI collection period: 100ms */

#define BUF_LEN_26 26
#define NEIGHBOR_SECURITY_MODE_MAX 8
#define MAX_OPERATING_BANDWIDTH 5

#define WIFI_MAC_ADDRESS_LENGTH 6
#define HAL_AP_IDX_TO_HAL_RADIO(apIdx) \
    ((apIdx < (2 * HAL_AP_NUM_APS_PER_RADIO)) ? (apIdx % 2) : (apIdx / HAL_AP_NUM_APS_PER_RADIO))
#define HAL_AP_IDX_TO_SSID_IDX(apIdx) \
    ((apIdx < (2 * HAL_AP_NUM_APS_PER_RADIO)) ? (apIdx / 2) : (apIdx % HAL_AP_NUM_APS_PER_RADIO))
#define WL_DRIVER_TO_AP_IDX(idx, subidx) \
    ((idx < 2) ? (idx + subidx * 2 + 1) : ((idx * HAL_AP_NUM_APS_PER_RADIO) + subidx + 1))
#define HAL_RADIO_IDX_TO_HAL_AP(radioIdx) \
    ((radioIdx < 2) ? radioIdx : (radioIdx * HAL_AP_NUM_APS_PER_RADIO))
#define HAL_VAP_AP_INDEX_NEXT(apIdx) ((apIdx < 16) ? (apIdx + 2) : (apIdx + 1))

/* bandwidth string to wifi_channelBandwidth_t */
static struct wifi_enum_to_str_map wifi_bandwidth_infoTable[] = {
    /* bwShift  bwStr */
    { WIFI_CHANNELBANDWIDTH_20MHZ,    "20"   },
    { WIFI_CHANNELBANDWIDTH_40MHZ,    "40"   },
    { WIFI_CHANNELBANDWIDTH_80MHZ,    "80"   },
    { WIFI_CHANNELBANDWIDTH_160MHZ,   "160"  },
    { WIFI_CHANNELBANDWIDTH_80_80MHZ, "8080" },
    { 0,                              NULL   }
};

wifi_enum_to_str_map_t mfp_table[] = {
    { wifi_mfp_cfg_disabled, "Disabled" },
    { wifi_mfp_cfg_optional, "Optional" },
    { wifi_mfp_cfg_required, "Required" },
    { 0xff,                  NULL       }
};

#define OPER_STANDS_MASK                                                                           \
    (WIFI_80211_VARIANT_AX | WIFI_80211_VARIANT_AC | WIFI_80211_VARIANT_N | WIFI_80211_VARIANT_G | \
        WIFI_80211_VARIANT_B | WIFI_80211_VARIANT_A)

/* 802.11 standard to wifi_ieee80211Variant_t */
static struct wifi_enum_to_str_map std2ieee80211Variant_infoTable[] = {
    /* ivar operStd */
    { WIFI_80211_VARIANT_AX, "ax" },
    { WIFI_80211_VARIANT_AD, "ad" },
    { WIFI_80211_VARIANT_AC, "ac" },
    { WIFI_80211_VARIANT_H,  "h"  },
    { WIFI_80211_VARIANT_N,  "n"  },
    { WIFI_80211_VARIANT_G,  "g"  },
    { WIFI_80211_VARIANT_B,  "b"  },
    { WIFI_80211_VARIANT_A,  "a"  },
    { 0,                     NULL }
};

wifi_enum_to_str_map_t security_mode_table[] = {
    { wifi_security_mode_none,                "None"                     },
    { wifi_security_mode_wep_64,              "WEP-64"                   },
    { wifi_security_mode_wep_128,             "WEP-128"                  },
    { wifi_security_mode_wpa_personal,        "WPA-Personal"             },
    { wifi_security_mode_wpa2_personal,       "WPA2-Personal"            },
    { wifi_security_mode_wpa_wpa2_personal,   "WPA-WPA2-Personal"        },
    { wifi_security_mode_wpa_enterprise,      "WPA-Enterprise"           },
    { wifi_security_mode_wpa2_enterprise,     "WPA2-Enterprise"          },
    { wifi_security_mode_wpa_wpa2_enterprise, "WPA-WPA2-Enterprise"      },
    { wifi_security_mode_wpa3_personal,       "WPA3-Personal"            },
    { wifi_security_mode_wpa3_transition,     "WPA3-Personal-Transition" },
    { wifi_security_mode_wpa3_enterprise,     "WPA3-Enterprise"          },
    { 0xff,                                   NULL                       }
};

wifi_enum_to_str_map_t encryption_table[] = {
    { wifi_encryption_tkip,     "TKIPEncryption"       },
    { wifi_encryption_aes,      "AESEncryption"        },
    { wifi_encryption_aes_tkip, "TKIPandAESEncryption" },
    { 0xff,                     NULL                   }
};

/* refer to wifi_ap_OnBoardingMethods_t for the enum definitions */
static wifi_enum_to_str_map_t wps_config_method_table[] = {
    { WIFI_ONBOARDINGMETHODS_USBFLASHDRIVE,      "USBFlashDrive"      },
    { WIFI_ONBOARDINGMETHODS_ETHERNET,           "Ethernet"           },
    { WIFI_ONBOARDINGMETHODS_LABEL,              "Label"              },
    { WIFI_ONBOARDINGMETHODS_DISPLAY,            "Display"            },
    { WIFI_ONBOARDINGMETHODS_EXTERNALNFCTOKEN,   "ExternalNFCToken"   },
    { WIFI_ONBOARDINGMETHODS_INTEGRATEDNFCTOKEN, "IntegratedNFCToken" },
    { WIFI_ONBOARDINGMETHODS_NFCINTERFACE,       "NFCInterface"       },
    { WIFI_ONBOARDINGMETHODS_PUSHBUTTON,         "PushButton"         },
    { WIFI_ONBOARDINGMETHODS_PIN,                "Keypad"             },
    { WIFI_ONBOARDINGMETHODS_PHYSICALPUSHBUTTON, "PhysicalPushButton" },
    { WIFI_ONBOARDINGMETHODS_PHYSICALDISPLAY,    "PhysicalDisplay"    },
    { WIFI_ONBOARDINGMETHODS_VIRTUALPUSHBUTTON,  "VirtualPushButton"  },
    { WIFI_ONBOARDINGMETHODS_VIRTUALDISPLAY,     "VirtualDisplay"     },
    { WIFI_ONBOARDINGMETHODS_EASYCONNECT,        "EASYCONNECT"        }, // not expected in WPS APIs
    { 0xff,                                      NULL                 }
};

#define PMODE_NONE 0x00
#define PMODE_A 0x01
#define PMODE_B 0x02
#define PMODE_G 0x04
#define PMODE_N 0x08
#define PMODE_AC 0x10
#define PMODE_AX 0x20

#define CHMODE_NONE PMODE_NONE
#define CHMODE_11A PMODE_A
#define CHMODE_11B PMODE_B
#define CHMODE_11G PMODE_G
#define CHMODE_11N PMODE_N
#define CHMODE_11AC PMODE_AC
#define CHMODE_11AX PMODE_AX

struct operStdPMode_info {
    char *operStd;
    char *pmodeStr;
    unsigned int pmodeVal;
};

/* operatingStandard to pureMode value for wifi_getRadioStandard */
static struct operStdPMode_info operStdPMode_infoTable[] = {
    /* operStd  pmodeStr    pmodeVal    Old RadioStandard Reference */
    { "b,g,n",     "n",  PMODE_NONE            }, /* gOnly = nOnly = acOnly = FALSE; */
#if !(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_))
    { "g,n",       "n",  PMODE_G               }, /* gOnly = TRUE; nOnly = acOnly = FALSE; */
#else
    { "g,n", "n", PMODE_NONE }, /* gOnly = nOnly = acOnly = FALSE; */
#endif
    { "g",         "g",  PMODE_G               }, /* gOnly = TRUE; nOnly = acOnly = FALSE; */
    { "b,g",       "g",  PMODE_NONE            }, /* gOnly = nOnly = acOnly = FALSE; */
    { "n",         "n",  PMODE_N               }, /* nOnly = TRUE; gOnly = acOnly = FALSE; */
    { "a,n,ac",    "ac", PMODE_NONE            }, /* gOnly = nOnly = acOnly = FALSE; */
    { "n,ac",      "ac", PMODE_N               }, /* nOnly = TRUE; gOnly = acOnly = FALSE; */
    { "ac",        "ac", PMODE_AC              }, /* acOnly = TRUE; gOnly = nOnly = FALSE; */
    { "b,g,n,ax",  "ax", PMODE_NONE            }, /* gOnly = nOnly = acOnly = axOnly = FALSE; */
#if !(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_))
    { "g,n,ax",    "ax", (PMODE_AX | PMODE_G)  }, /* 2G */
#else
    { "g,n,ax", "ax", PMODE_NONE }, /* gOnly = nOnly = acOnly = axOnly = FALSE; */
#endif
    { "n,ax",      "ax", (PMODE_AX | PMODE_N)  }, /* 2G */
    { "ac,ax",     "ax", (PMODE_AX | PMODE_AC) }, /* 5G */
    { "n,ac,ax",   "ax", (PMODE_AX | PMODE_N)  }, /* 5G */
    { "a,n,ac,ax", "ax", PMODE_NONE            }, /* gOnly = nOnly = acOnly = axOnly = FALSE; */
    { "ax",        "ax", PMODE_AX              }, /* axOnly = TRUE; gOnly = nOnly = acOnly = FALSE; */
    { NULL,        NULL, PMODE_NONE            }
};

void wifi_hal_print(wifi_hal_log_level_t level, const char *format, ...)
{
    char buff[256] = { 0 };
    va_list list;
    FILE *fpg = NULL;

    get_formatted_time(buff);

#ifdef LINUX_VM_PORT
    printf("%s ", buff);
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
#else
    if ((access("/nvram/wifiHalDbg", R_OK)) == 0) {

        fpg = fopen("/tmp/wifiHal", "a+");
        if (fpg == NULL) {
            return;
        }
    } else {
        switch (level) {
        case WIFI_HAL_LOG_LVL_INFO:
        case WIFI_HAL_LOG_LVL_ERROR:
            fpg = fopen("/rdklogs/logs/wifiHal.txt", "a+");
            if (fpg == NULL) {
                return;
            }
            break;
        case WIFI_HAL_LOG_LVL_DEBUG:
        default:
            return;
        }
    }
    static const char *level_marker[WIFI_HAL_LOG_LVL_MAX] = {
        [WIFI_HAL_LOG_LVL_DEBUG] = "<D>",
        [WIFI_HAL_LOG_LVL_INFO] = "<I>",
        [WIFI_HAL_LOG_LVL_ERROR] = "<E>",
    };
    if (level < WIFI_HAL_LOG_LVL_MAX)
        snprintf(&buff[strlen(buff)], 256 - strlen(buff), " %s ", level_marker[level]);

    fprintf(fpg, "%s ", buff);
    va_start(list, format);
    vfprintf(fpg, format, list);
    va_end(list);
    fflush(fpg);
    fclose(fpg);
#endif
    return;
}

wifi_enum_to_str_map_t countrycode_table[] = {
    { wifi_countrycode_AC,  "AC" }, /**< ASCENSION ISLAND */
    { wifi_countrycode_AD,  "AD" }, /**< ANDORRA */
    { wifi_countrycode_AE,  "AE" }, /**< UNITED ARAB EMIRATES */
    { wifi_countrycode_AF,  "AF" }, /**< AFGHANISTAN */
    { wifi_countrycode_AG,  "AG" }, /**< ANTIGUA AND BARBUDA */
    { wifi_countrycode_AI,  "AI" }, /**< ANGUILLA */
    { wifi_countrycode_AL,  "AL" }, /**< ALBANIA */
    { wifi_countrycode_AM,  "AM" }, /**< ARMENIA */
    { wifi_countrycode_AN,  "AN" }, /**< NETHERLANDS ANTILLES */
    { wifi_countrycode_AO,  "AO" }, /**< ANGOLA */
    { wifi_countrycode_AQ,  "AQ" }, /**< ANTARCTICA */
    { wifi_countrycode_AR,  "AR" }, /**< ARGENTINA */
    { wifi_countrycode_AS,  "AS" }, /**< AMERICAN SAMOA */
    { wifi_countrycode_AT,  "AT" }, /**< AUSTRIA */
    { wifi_countrycode_AU,  "AU" }, /**< AUSTRALIA */
    { wifi_countrycode_AW,  "AW" }, /**< ARUBA */
    { wifi_countrycode_AZ,  "AZ" }, /**< AZERBAIJAN */
    { wifi_countrycode_BA,  "BA" }, /**< BOSNIA AND HERZEGOVINA */
    { wifi_countrycode_BB,  "BB" }, /**< BARBADOS */
    { wifi_countrycode_BD,  "BD" }, /**< BANGLADESH */
    { wifi_countrycode_BE,  "BE" }, /**< BELGIUM */
    { wifi_countrycode_BF,  "BF" }, /**< BURKINA FASO */
    { wifi_countrycode_BG,  "BG" }, /**< BULGARIA */
    { wifi_countrycode_BH,  "BH" }, /**< BAHRAIN */
    { wifi_countrycode_BI,  "BI" }, /**< BURUNDI */
    { wifi_countrycode_BJ,  "BJ" }, /**< BENIN */
    { wifi_countrycode_BM,  "BM" }, /**< BERMUDA */
    { wifi_countrycode_BN,  "BN" }, /**< BRUNEI DARUSSALAM */
    { wifi_countrycode_BO,  "BO" }, /**< BOLIVIA */
    { wifi_countrycode_BR,  "BR" }, /**< BRAZIL */
    { wifi_countrycode_BS,  "BS" }, /**< BAHAMAS */
    { wifi_countrycode_BT,  "BT" }, /**< BHUTAN */
    { wifi_countrycode_BV,  "BV" }, /**< BOUVET ISLAND */
    { wifi_countrycode_BW,  "BW" }, /**< BOTSWANA */
    { wifi_countrycode_BY,  "BY" }, /**< BELARUS */
    { wifi_countrycode_BZ,  "BZ" }, /**< BELIZE */
    { wifi_countrycode_CA,  "CA" }, /**< CANADA */
    { wifi_countrycode_CC,  "CC" }, /**< COCOS (KEELING) ISLANDS */
    { wifi_countrycode_CD,  "CD" }, /**< CONGO, THE DEMOCRATIC REPUBLIC OF THE */
    { wifi_countrycode_CF,  "CF" }, /**< CENTRAL AFRICAN REPUBLIC */
    { wifi_countrycode_CG,  "CG" }, /**< CONGO */
    { wifi_countrycode_CH,  "CH" }, /**< SWITZERLAND */
    { wifi_countrycode_CI,  "CI" }, /**< COTE D'IVOIRE */
    { wifi_countrycode_CK,  "CK" }, /**< COOK ISLANDS */
    { wifi_countrycode_CL,  "CL" }, /**< CHILE */
    { wifi_countrycode_CM,  "CM" }, /**< CAMEROON */
    { wifi_countrycode_CN,  "CN" }, /**< CHINA */
    { wifi_countrycode_CO,  "CO" }, /**< COLOMBIA */
    { wifi_countrycode_CP,  "CP" }, /**< CLIPPERTON ISLAND */
    { wifi_countrycode_CR,  "CR" }, /**< COSTA RICA */
    { wifi_countrycode_CU,  "CU" }, /**< CUBA */
    { wifi_countrycode_CV,  "CV" }, /**< CAPE VERDE */
    { wifi_countrycode_CY,  "CY" }, /**< CYPRUS */
    { wifi_countrycode_CX,  "CX" }, /**< CHRISTMAS ISLAND */
    { wifi_countrycode_CZ,  "CZ" }, /**< CZECH REPUBLIC */
    { wifi_countrycode_DE,  "DE" }, /**< GERMANY */
    { wifi_countrycode_DJ,  "DJ" }, /**< DJIBOUTI */
    { wifi_countrycode_DK,  "DK" }, /**< DENMARK */
    { wifi_countrycode_DM,  "DM" }, /**< DOMINICA */
    { wifi_countrycode_DO,  "DO" }, /**< DOMINICAN REPUBLIC */
    { wifi_countrycode_DZ,  "DZ" }, /**< ALGERIA */
    { wifi_countrycode_EC,  "EC" }, /**< ECUADOR */
    { wifi_countrycode_EE,  "EE" }, /**< ESTONIA */
    { wifi_countrycode_EG,  "EG" }, /**< EGYPT */
    { wifi_countrycode_EH,  "EH" }, /**< WESTERN SAHARA */
    { wifi_countrycode_ER,  "ER" }, /**< ERITREA */
    { wifi_countrycode_ES,  "ES" }, /**< SPAIN */
    { wifi_countrycode_ET,  "ET" }, /**< ETHIOPIA */
    { wifi_countrycode_FI,  "FI" }, /**< FINLAND */
    { wifi_countrycode_FJ,  "FJ" }, /**< FIJI */
    { wifi_countrycode_FK,  "FK" }, /**< FALKLAND ISLANDS (MALVINAS) */
    { wifi_countrycode_FM,  "FM" }, /**< MICRONESIA, FEDERATED STATES OF */
    { wifi_countrycode_FO,  "FO" }, /**< FAROE ISLANDS */
    { wifi_countrycode_FR,  "FR" }, /**< FRANCE */
    { wifi_countrycode_GA,  "GA" }, /**< GABON */
    { wifi_countrycode_GB,  "GB" }, /**< UNITED KINGDOM */
    { wifi_countrycode_GD,  "GD" }, /**< GRENADA */
    { wifi_countrycode_GE,  "GE" }, /**< GEORGIA */
    { wifi_countrycode_GF,  "GF" }, /**< FRENCH GUIANA */
    { wifi_countrycode_GG,  "GG" }, /**< GUERNSEY */
    { wifi_countrycode_GH,  "GH" }, /**< GHANA */
    { wifi_countrycode_GI,  "GI" }, /**< GIBRALTAR */
    { wifi_countrycode_GL,  "GL" }, /**< GREENLAND */
    { wifi_countrycode_GM,  "GM" }, /**< GAMBIA */
    { wifi_countrycode_GN,  "GN" }, /**< GUINEA */
    { wifi_countrycode_GP,  "GP" }, /**< GUADELOUPE */
    { wifi_countrycode_GQ,  "GQ" }, /**< EQUATORIAL GUINEA */
    { wifi_countrycode_GR,  "GR" }, /**< GREECE */
    { wifi_countrycode_GS,  "GS" }, /**< SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS */
    { wifi_countrycode_GT,  "GT" }, /**< GUATEMALA */
    { wifi_countrycode_GU,  "GU" }, /**< GUAM */
    { wifi_countrycode_GW,  "GW" }, /**< GUINEA-BISSAU */
    { wifi_countrycode_GY,  "GY" }, /**< GUYANA */
    { wifi_countrycode_HR,  "HR" }, /**< CROATIA */
    { wifi_countrycode_HT,  "HT" }, /**< HAITI */
    { wifi_countrycode_HM,  "HM" }, /**< HEARD ISLAND AND MCDONALD ISLANDS */
    { wifi_countrycode_HN,  "HN" }, /**< HONDURAS */
    { wifi_countrycode_HK,  "HK" }, /**< HONG KONG */
    { wifi_countrycode_HU,  "HU" }, /**< HUNGARY */
    { wifi_countrycode_IS,  "IS" }, /**< ICELAND */
    { wifi_countrycode_IN,  "IN" }, /**< INDIA */
    { wifi_countrycode_ID,  "ID" }, /**< INDONESIA */
    { wifi_countrycode_IR,  "IR" }, /**< IRAN, ISLAMIC REPUBLIC OF */
    { wifi_countrycode_IQ,  "IQ" }, /**< IRAQ */
    { wifi_countrycode_IE,  "IE" }, /**< IRELAND */
    { wifi_countrycode_IL,  "IL" }, /**< ISRAEL */
    { wifi_countrycode_IM,  "IM" }, /**< MAN, ISLE OF */
    { wifi_countrycode_IT,  "IT" }, /**< ITALY */
    { wifi_countrycode_IO,  "IO" }, /**< BRITISH INDIAN OCEAN TERRITORY */
    { wifi_countrycode_JM,  "JM" }, /**< JAMAICA */
    { wifi_countrycode_JP,  "JP" }, /**< JAPAN */
    { wifi_countrycode_JE,  "JE" }, /**< JERSEY */
    { wifi_countrycode_JO,  "JO" }, /**< JORDAN */
    { wifi_countrycode_KE,  "KE" }, /**< KENYA */
    { wifi_countrycode_KG,  "KG" }, /**< KYRGYZSTAN */
    { wifi_countrycode_KH,  "KH" }, /**< CAMBODIA */
    { wifi_countrycode_KI,  "KI" }, /**< KIRIBATI */
    { wifi_countrycode_KM,  "KM" }, /**< COMOROS */
    { wifi_countrycode_KN,  "KN" }, /**< SAINT KITTS AND NEVIS */
    { wifi_countrycode_KP,  "KP" }, /**< KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF */
    { wifi_countrycode_KR,  "KR" }, /**< KOREA, REPUBLIC OF */
    { wifi_countrycode_KW,  "KW" }, /**< KUWAIT */
    { wifi_countrycode_KY,  "KY" }, /**< CAYMAN ISLANDS */
    { wifi_countrycode_KZ,  "KZ" }, /**< KAZAKHSTAN */
    { wifi_countrycode_LA,  "LA" }, /**< LAO PEOPLE'S DEMOCRATIC REPUBLIC */
    { wifi_countrycode_LB,  "LB" }, /**< LEBANON */
    { wifi_countrycode_LC,  "LC" }, /**< SAINT LUCIA */
    { wifi_countrycode_LI,  "LI" }, /**< LIECHTENSTEIN */
    { wifi_countrycode_LK,  "LK" }, /**< SRI LANKA */
    { wifi_countrycode_LR,  "LR" }, /**< LIBERIA */
    { wifi_countrycode_LS,  "LS" }, /**< LESOTHO */
    { wifi_countrycode_LT,  "LT" }, /**< LITHUANIA */
    { wifi_countrycode_LU,  "LU" }, /**< LUXEMBOURG */
    { wifi_countrycode_LV,  "LV" }, /**< LATVIA */
    { wifi_countrycode_LY,  "LY" }, /**< LIBYAN ARAB JAMAHIRIYA */
    { wifi_countrycode_MA,  "MA" }, /**< MOROCCO */
    { wifi_countrycode_MC,  "MC" }, /**< MONACO */
    { wifi_countrycode_MD,  "MD" }, /**< MOLDOVA, REPUBLIC OF */
    { wifi_countrycode_ME,  "ME" }, /**< MONTENEGRO */
    { wifi_countrycode_MG,  "MG" }, /**< MADAGASCAR */
    { wifi_countrycode_MH,  "MH" }, /**< MARSHALL ISLANDS */
    { wifi_countrycode_MK,  "MK" }, /**< MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF */
    { wifi_countrycode_ML,  "ML" }, /**< MALI */
    { wifi_countrycode_MM,  "MM" }, /**< MYANMAR */
    { wifi_countrycode_MN,  "MN" }, /**< MONGOLIA */
    { wifi_countrycode_MO,  "MO" }, /**< MACAO */
    { wifi_countrycode_MQ,  "MQ" }, /**< MARTINIQUE */
    { wifi_countrycode_MR,  "MR" }, /**< MAURITANIA */
    { wifi_countrycode_MS,  "MS" }, /**< MONTSERRAT */
    { wifi_countrycode_MT,  "MT" }, /**< MALTA */
    { wifi_countrycode_MU,  "MU" }, /**< MAURITIUS */
    { wifi_countrycode_MV,  "MV" }, /**< MALDIVES */
    { wifi_countrycode_MW,  "MW" }, /**< MALAWI */
    { wifi_countrycode_MX,  "MX" }, /**< MEXICO */
    { wifi_countrycode_MY,  "MY" }, /**< MALAYSIA */
    { wifi_countrycode_MZ,  "MZ" }, /**< MOZAMBIQUE */
    { wifi_countrycode_NA,  "NA" }, /**< NAMIBIA */
    { wifi_countrycode_NC,  "NC" }, /**< NEW CALEDONIA */
    { wifi_countrycode_NE,  "NE" }, /**< NIGER */
    { wifi_countrycode_NF,  "NF" }, /**< NORFOLK ISLAND */
    { wifi_countrycode_NG,  "NG" }, /**< NIGERIA */
    { wifi_countrycode_NI,  "NI" }, /**< NICARAGUA */
    { wifi_countrycode_NL,  "NL" }, /**< NETHERLANDS */
    { wifi_countrycode_NO,  "NO" }, /**< NORWAY */
    { wifi_countrycode_NP,  "NP" }, /**< NEPAL */
    { wifi_countrycode_NR,  "NR" }, /**< NAURU */
    { wifi_countrycode_NU,  "NU" }, /**< NIUE */
    { wifi_countrycode_NZ,  "NZ" }, /**< NEW ZEALAND */
    { wifi_countrycode_MP,  "MP" }, /**< NORTHERN MARIANA ISLANDS */
    { wifi_countrycode_OM,  "OM" }, /**< OMAN */
    { wifi_countrycode_PA,  "PA" }, /**< PANAMA */
    { wifi_countrycode_PE,  "PE" }, /**< PERU */
    { wifi_countrycode_PF,  "PF" }, /**< FRENCH POLYNESIA */
    { wifi_countrycode_PG,  "PG" }, /**< PAPUA NEW GUINEA */
    { wifi_countrycode_PH,  "PH" }, /**< PHILIPPINES */
    { wifi_countrycode_PK,  "PK" }, /**< PAKISTAN */
    { wifi_countrycode_PL,  "PL" }, /**< POLAND */
    { wifi_countrycode_PM,  "PM" }, /**< SAINT PIERRE AND MIQUELON */
    { wifi_countrycode_PN,  "PN" }, /**< PITCAIRN */
    { wifi_countrycode_PR,  "PR" }, /**< PUERTO RICO */
    { wifi_countrycode_PS,  "PS" }, /**< PALESTINIAN TERRITORY, OCCUPIED */
    { wifi_countrycode_PT,  "PT" }, /**< PORTUGAL */
    { wifi_countrycode_PW,  "PW" }, /**< PALAU */
    { wifi_countrycode_PY,  "PY" }, /**< PARAGUAY */
    { wifi_countrycode_QA,  "QA" }, /**< QATAR */
    { wifi_countrycode_RE,  "RE" }, /**< REUNION */
    { wifi_countrycode_RO,  "RO" }, /**< ROMANIA */
    { wifi_countrycode_RS,  "RS" }, /**< SERBIA */
    { wifi_countrycode_RU,  "RU" }, /**< RUSSIAN FEDERATION */
    { wifi_countrycode_RW,  "RW" }, /**< RWANDA */
    { wifi_countrycode_SA,  "SA" }, /**< SAUDI ARABIA */
    { wifi_countrycode_SB,  "SB" }, /**< SOLOMON ISLANDS */
    { wifi_countrycode_SD,  "SD" }, /**< SUDAN */
    { wifi_countrycode_SE,  "SE" }, /**< SWEDEN */
    { wifi_countrycode_SC,  "SC" }, /**< SEYCHELLES */
    { wifi_countrycode_SG,  "SG" }, /**< SINGAPORE */
    { wifi_countrycode_SH,  "SH" }, /**< SAINT HELENA */
    { wifi_countrycode_SI,  "SI" }, /**< SLOVENIA */
    { wifi_countrycode_SJ,  "SJ" }, /**< SVALBARD AND JAN MAYEN */
    { wifi_countrycode_SK,  "SK" }, /**< SLOVAKIA */
    { wifi_countrycode_SL,  "SL" }, /**< SIERRA LEONE */
    { wifi_countrycode_SM,  "SM" }, /**< SAN MARINO */
    { wifi_countrycode_SN,  "SN" }, /**< SENEGAL */
    { wifi_countrycode_SO,  "SO" }, /**< SOMALIA */
    { wifi_countrycode_SR,  "SR" }, /**< SURINAME */
    { wifi_countrycode_ST,  "ST" }, /**< SAO TOME AND PRINCIPE */
    { wifi_countrycode_SV,  "SV" }, /**< EL SALVADOR */
    { wifi_countrycode_SY,  "SY" }, /**< SYRIAN ARAB REPUBLIC */
    { wifi_countrycode_SZ,  "SZ" }, /**< SWAZILAND */
    { wifi_countrycode_TA,  "TA" }, /**< TRISTAN DA CUNHA */
    { wifi_countrycode_TC,  "TC" }, /**< TURKS AND CAICOS ISLANDS */
    { wifi_countrycode_TD,  "TD" }, /**< CHAD */
    { wifi_countrycode_TF,  "TF" }, /**< FRENCH SOUTHERN TERRITORIES */
    { wifi_countrycode_TG,  "TG" }, /**< TOGO */
    { wifi_countrycode_TH,  "TH" }, /**< THAILAND */
    { wifi_countrycode_TJ,  "TJ" }, /**< TAJIKISTAN */
    { wifi_countrycode_TK,  "TK" }, /**< TOKELAU */
    { wifi_countrycode_TL,  "TL" }, /**< TIMOR-LESTE (EAST TIMOR) */
    { wifi_countrycode_TM,  "TM" }, /**< TURKMENISTAN */
    { wifi_countrycode_TN,  "TN" }, /**< TUNISIA */
    { wifi_countrycode_TO,  "TO" }, /**< TONGA */
    { wifi_countrycode_TR,  "TR" }, /**< TURKEY */
    { wifi_countrycode_TT,  "TT" }, /**< TRINIDAD AND TOBAGO */
    { wifi_countrycode_TV,  "TV" }, /**< TUVALU */
    { wifi_countrycode_TW,  "TW" }, /**< TAIWAN, PROVINCE OF CHINA */
    { wifi_countrycode_TZ,  "TZ" }, /**< TANZANIA, UNITED REPUBLIC OF */
    { wifi_countrycode_UA,  "UA" }, /**< UKRAINE */
    { wifi_countrycode_UG,  "UG" }, /**< UGANDA */
    { wifi_countrycode_UM,  "UM" }, /**< UNITED STATES MINOR OUTLYING ISLANDS */
    { wifi_countrycode_US,  "US" }, /**< UNITED STATES */
    { wifi_countrycode_UY,  "UY" }, /**< URUGUAY */
    { wifi_countrycode_UZ,  "UZ" }, /**< UZBEKISTAN */
    { wifi_countrycode_VA,  "VA" }, /**< HOLY SEE (VATICAN CITY STATE) */
    { wifi_countrycode_VC,  "VC" }, /**< SAINT VINCENT AND THE GRENADINES */
    { wifi_countrycode_VE,  "VE" }, /**< VENEZUELA */
    { wifi_countrycode_VG,  "VG" }, /**< VIRGIN ISLANDS, BRITISH */
    { wifi_countrycode_VI,  "VI" }, /**< VIRGIN ISLANDS, U.S. */
    { wifi_countrycode_VN,  "VN" }, /**< VIET NAM */
    { wifi_countrycode_VU,  "VU" }, /**< VANUATU */
    { wifi_countrycode_WF,  "WF" }, /**< WALLIS AND FUTUNA */
    { wifi_countrycode_WS,  "WS" }, /**< SAMOA */
    { wifi_countrycode_YE,  "YE" }, /**< YEMEN */
    { wifi_countrycode_YT,  "YT" }, /**< MAYOTTE */
    { wifi_countrycode_YU,  "YU" }, /**< YUGOSLAVIA */
    { wifi_countrycode_ZA,  "ZA" }, /**< SOUTH AFRICA */
    { wifi_countrycode_ZM,  "ZM" }, /**< ZAMBIA */
    { wifi_countrycode_ZW,  "ZW" }, /**< ZIMBABWE */
    { wifi_countrycode_max, NULL }, /**< Max number of country code */
};

static void str_to_upper(char *str)
{
    char *p = str;

    while (*p) {
        *p = toupper(*p);
        p++;
    }
}

static INT wifi_updateApSecurity(INT apIndex, wifi_vap_security_t *security)
{
    int i;
    char *mfpStr;

    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);

    if (security->mode == wifi_security_mode_wep_64 ||
        security->mode == wifi_security_mode_wep_128) {
        return RETURN_ERR;
    }

    /* set mfp */
    /* WIFI_HAL_VERSION_3_PHASE2 for RDKM */
    for (i = 0; mfp_table[i].str_val != NULL; ++i) {
        if (mfp_table[i].enum_val == security->mfp)
            break;
    }
    if (mfp_table[i].str_val == NULL) {
        return RETURN_ERR;
    }

    mfpStr = (char *)mfp_table[i].str_val;
    if (wifi_setApSecurityMFPConfig(apIndex, mfpStr)) {
        wifi_hal_error_print("%s:%d wifi_setApSecurityMFPConfig Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* set security mode */
    for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
        if (security_mode_table[i].enum_val == security->mode)
            break;
    }
    if (security_mode_table[i].str_val == NULL) {
        return RETURN_ERR;
    }

    if (wifi_setApSecurityModeEnabled(apIndex, (char *)security_mode_table[i].str_val) < 0) {
        wifi_hal_error_print("%s:%d wifi_setApSecurityModeEnabled Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }
    if (security->mode == wifi_security_mode_none)
        return RETURN_OK;

    /* set wpa encryption */
    for (i = 0; encryption_table[i].str_val != NULL; ++i) {
        if (encryption_table[i].enum_val == security->encr)
            break;
    }

    if (encryption_table[i].str_val == NULL) {
        wifi_hal_error_print("%s:%d Wrong Encryption Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    if (wifi_setApWpaEncryptionMode(apIndex, (char *)encryption_table[i].str_val) < 0) {
        wifi_hal_error_print("%s:%d wifi_setApWpaEncryptionMode Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* set wpa psk or passphrase */
    if (security->mode == wifi_security_mode_wpa_personal ||
        security->mode == wifi_security_mode_wpa2_personal ||
        security->mode == wifi_security_mode_wpa3_personal ||
        security->mode == wifi_security_mode_wpa_wpa2_personal ||
        security->mode == wifi_security_mode_wpa3_transition) {
        if (security->u.key.type == wifi_security_key_type_psk) {
            if (wifi_setApSecurityPreSharedKey(apIndex, security->u.key.key) < 0) {
                wifi_hal_error_print("%s:%d wifi_setApSecurityPreSharedKey Failed\n", __func__,
                    __LINE__);
                return RETURN_ERR;
            }

        } else {
            if (wifi_setApSecurityKeyPassphrase(apIndex, security->u.key.key) < 0) {
                wifi_hal_error_print("%s:%d wifi_setApSecurityKeyPassphrase Failed\n", __func__,
                    __LINE__);
                return RETURN_ERR;
            }
        }
        // Wp3 Transition Need to check.
    }

    /* set RADIUS auth server params
     * greylist requires RADIUS params for open mode,
     * but greylist is not configured through this API, and greylist
     * may be enabled after RADIUS params are set, we have to set RADIUS
     * params for open mode even greylist may not be enabled.
     */
    if (security->mode == wifi_security_mode_wpa_enterprise ||
        security->mode == wifi_security_mode_wpa2_enterprise ||
        security->mode == wifi_security_mode_wpa3_enterprise ||
        security->mode == wifi_security_mode_wpa_wpa2_enterprise ||
        security->mode == wifi_security_mode_none) {
        char ip_txt[INET6_ADDRSTRLEN] = { '\0' };
        unsigned int port;
#if defined(WIFI_HAL_VERSION_3_PHASE2)
        int domain;
#endif

#if defined(WIFI_HAL_VERSION_3_PHASE2)
        /* set primary radius auth server ip addr, port, secret */
        if (security->u.radius.ip.family == wifi_ip_family_ipv4) {
            domain = AF_INET;
        } else if (security->u.radius.ip.family == wifi_ip_family_ipv6) {
            domain = AF_INET6;
        } else {
            wifi_hal_error_print("%s:%d unknown IP addr family\n", __func__, __LINE__);
            return RETURN_ERR;
        }
        /* addr of IPv4adddr and IPv6addr is the same as they are in the same union */
        if (inet_ntop(domain, (void *)security->u.radius.ip.u.IPv6addr, ip_txt, sizeof(ip_txt)) ==
            NULL) {
            wifi_hal_error_print("%s:%d fail to convert primary RADIUS server ip addr \n", __func__,
                __LINE__);
            return RETURN_ERR;
        }
#else /* WIFI_HAL_VERSION_3_PHASE2 */
        snprintf(ip_txt, sizeof(ip_txt), (char *)security->u.radius.ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

        if (wifi_setApSecurityRadiusServer(apIndex, ip_txt, security->u.radius.port,
                security->u.radius.key) < 0) {
            wifi_hal_error_print("%s:%d wifi_setApSecurityRadiusServer Failed \n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

#if defined(WIFI_HAL_VERSION_3_PHASE2)
        /* set secondary radius auth server ip addr, port, secret */
        if (security->u.radius.s_ip.family == wifi_ip_family_ipv4) {
            if (security->u.radius.s_ip.u.IPv4addr == 0) {
                /* So far, we cannot know explicitly how secondary RADIUS auth server
                 * is not set , but mostly likely, memory for secondary RADIUS auth
                 * server is set to all '\0'
                 *
                 * TODO: To discuss how to decide secondary RADIUS auth server
                 * is not set in a better way */
                return RETURN_OK;
            }
            domain = AF_INET;
        } else if (security->u.radius.s_ip.family == wifi_ip_family_ipv6) {
            domain = AF_INET6;
        } else {
            /* consider this as secondary RADIUS auth is not configured,
             * until we have a better way to decide secondary RADIUS is not set */
            return RETURN_OK;
        }
        /* addr of IPv4adddr and IPv6addr is the same as they are in the same union */
        if (inet_ntop(domain, (void *)security->u.radius.s_ip.u.IPv6addr, ip_txt, sizeof(ip_txt)) ==
            NULL) {
            /* Don't consider this as error */
            return RETURN_OK;
        }
#else /* WIFI_HAL_VERSION_3_PHASE2 */
        snprintf(ip_txt, sizeof(ip_txt), (char *)security->u.radius.s_ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
        if (wifi_setApSecuritySecondaryRadiusServer(apIndex, ip_txt, security->u.radius.s_port,
                security->u.radius.s_key) < 0) {
            wifi_hal_error_print("%s:%d wifi_setApSecurityRadiusServer Failed \n", __func__,
                __LINE__);
        }

        return RETURN_OK;
    }

    return RETURN_OK;
}

INT wifi_setApWpsConfiguration(INT apIndex, wifi_wps_t *wpsConfig)
{
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);

    /* Set WPS enable */
    if (wifi_setApWpsEnable(apIndex, wpsConfig->enable) < 0) {
        wifi_hal_error_print("%s:%d wifi_setApWpsEnable Failed \n", __func__, __LINE__);
        return RETURN_ERR;
    }

    if (!wpsConfig->enable)
        return RETURN_OK;

    /* Set WPS device PIN */
    if (wpsConfig->pin[0] != '\0') {
        if (wifi_setApWpsDevicePIN(apIndex, wpsConfig->pin[0]) < 0) {
            wifi_hal_error_print("%s:%d wifi_setApSecurityRadiusServer Failed \n", __func__,
                __LINE__);
            return RETURN_ERR;
        }
    }

    /* Set enabled WPS config methods */
    if (wpsConfig->methods) {
        char enabled_methods[512] = { '\0' }; // buffer is big enough to hold all methods
        int i = 0, size = 0, len = 0;

        len = sizeof(enabled_methods);
        for (i = 0; wps_config_method_table[i].str_val != NULL; ++i) {
            if (wpsConfig->methods & wps_config_method_table[i].enum_val) {
                if (enabled_methods[0] == '\0')
                    size = snprintf(enabled_methods, len, "%s", wps_config_method_table[i].str_val);
                else
                    size += snprintf(enabled_methods + size, len - size, ",%s",
                        wps_config_method_table[i].str_val);
            }
        }
        if (wifi_setApWpsConfigMethodsEnabled(apIndex, enabled_methods) < 0) {
            wifi_hal_error_print("%s:%d wifi_setApWpsConfigMethodsEnabled Failed \n", __func__,
                __LINE__);
            return RETURN_ERR;
        }
    }
    return RETURN_OK;
}

INT wifi_createVAP(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);
    int i, apIndex, ret, enable, result = RETURN_OK;
    char encMode[OUTPUT_STRING_LENGTH_64] = { 0 };
    int no_apply = 0;
    int wps_no_apply = 0;
    int security_no_apply = 0;
    char *name;
    unsigned char UAPSDEnable = 0;
    unsigned char WMEnable = 0;
    int local_nbr_val = 0;
    wifi_wps_t event;
    wifi_vap_security_t tmp_security;

    for (i = 0; i < map->num_vaps; i++) {
        apIndex = map->vap_array[i].vap_index;
        /*apIndex check is a temporary workaround to bring down WifiAgent boot-up time, will be
         * removed*/
        ret = wifi_setApEnable(apIndex, FALSE);

        if (!map->vap_array[i].u.bss_info.enabled) {
            wifi_hal_info_print("%s:%d ap_index:%d not enabled\n", __func__, __LINE__, apIndex);
            continue;
        }
        if (ret < 0) {
            wifi_hal_error_print("%s:%d wifi_setApEnable Failed\n", __func__, __LINE__);
            return RETURN_ERR;
        }

        name = malloc(SSID_STRING_LEN);
        if (name) {
            memset(name, 0, SSID_STRING_LEN);
        }
        strncpy(name, map->vap_array[i].u.bss_info.ssid,
            sizeof(map->vap_array[i].u.bss_info.ssid) + 1);
        if (wifi_setSSIDName(apIndex, name) < 0) {
            wifi_hal_error_print("%s:%d wifi_setSSIDName Failed\n", __func__, __LINE__);
            return RETURN_ERR;
        }

        if (name) {
            free(name);
        }

        if (wifi_setApSsidAdvertisementEnable(apIndex, map->vap_array[i].u.bss_info.showSsid) < 0) {
            wifi_hal_error_print("%s:%d wifi_setSSIDName Failed\n", __func__, __LINE__);
            return RETURN_ERR;
        }

        if (wifi_setApIsolationEnable(apIndex, map->vap_array[i].u.bss_info.isolation) < 0) {
            wifi_hal_error_print("%s:%d  wifi_setApIsolationEnable Failed\n", __func__, __LINE__);
            return RETURN_ERR;
        }

        if (wifi_setApManagementFramePowerControl(apIndex,
                map->vap_array[i].u.bss_info.mgmtPowerControl) < 0) {
            wifi_hal_error_print("%s:%d   wifi_setApManagementFramePowerControl Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        if (map->vap_array[i].u.bss_info.bssMaxSta > 0) {
            if (wifi_setApMaxAssociatedDevices(apIndex, map->vap_array[i].u.bss_info.bssMaxSta) <
                0) {
                wifi_hal_error_print("%s:%d  wifi_setApMaxAssociatedDevices Failed\n", __func__,
                    __LINE__);
                return RETURN_ERR;
            }
        }

        if (wifi_setBSSTransitionActivation(apIndex,
                map->vap_array[i].u.bss_info.bssTransitionActivated) < 0) {
            wifi_hal_error_print("%s:%d  wifi_setBSSTransitionActivation Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        bool rrm_val = false;
        if (wifi_getNeighborReportActivation(apIndex, (unsigned char *)&rrm_val) < 0) {
            wifi_hal_error_print("%s:%d  wifi_getNeighborReportActivation Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        if (wifi_setNeighborReportActivation(apIndex, rrm_val) < 0) {
            wifi_hal_error_print("%s:%d wifi_setNeighborReportActivation  Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        if (wifi_updateApSecurity(apIndex, &(map->vap_array[i].u.bss_info.security)) < 0) {
            wifi_hal_error_print("%s:%d  wifi_updateApSecurity Failed\n", __func__, __LINE__);
            return RETURN_ERR;
        }

        if (wifi_pushApInterworkingElement(apIndex,
                &(map->vap_array[i].u.bss_info.interworking.interworking)) < 0) {
            wifi_hal_error_print("%s:%d wifi_pushApInterworkingElement  Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        int enable = 0;
        if (map->vap_array[i].u.bss_info.mac_filter_enable) {
            if (map->vap_array[i].u.bss_info.mac_filter_mode == wifi_mac_filter_mode_black_list) {
                enable = 1;
            } else {
                enable = 2;
            }
        } else {
            enable = 0;
        }

        if (wifi_setApMacAddressControlMode(apIndex, enable) < 0) {
            wifi_hal_error_print("%s:%d wifi_pushApInterworkingElement  Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        if (wifi_setApWmmEnable(apIndex, map->vap_array[i].u.bss_info.wmm_enabled) < 0) {
            wifi_hal_error_print("%s:%d wifi_setApWmmEnable Failed \n", __func__, __LINE__);
            return RETURN_ERR;
        }

        if (wifi_setApWpsConfiguration(apIndex, &(map->vap_array[i].u.bss_info.wps)) < 0) {
            wifi_hal_error_print("%s:%d wifi_setApWpsConfiguration Failed \n", __func__, __LINE__);
            return RETURN_ERR;
        }
    }
    return RETURN_OK;
}

INT wifi_getApSecurity(INT apIndex, wifi_vap_security_t *security)
{
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);
    int ret, i;
    char security_mode[128] = { '\0' }, encryption[128] = { '\0' }, mfpStr[32] = { '\0' };

    memset(security, 0, sizeof(*security));

    /* get security mode */
    if (wifi_getApSecurityModeEnabled(apIndex, security_mode) < 0) {
        wifi_hal_error_print("%s:%d wifi_getApSecurityModeEnabled Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
        if (!strcmp(security_mode_table[i].str_val, security_mode)) {
            security->mode = security_mode_table[i].enum_val;
            break;
        }
    }
    if (security_mode_table[i].str_val == NULL) {
        wifi_hal_error_print("%s:%d Unsupported Security mode\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* get mfp */
    if (wifi_getApSecurityMFPConfig(apIndex, mfpStr) < 0) {
        wifi_hal_error_print("%s:%d wifi_getApSecurityMFPConfig Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    for (i = 0; mfp_table[i].str_val != NULL; ++i) {
        if (!strcmp(mfp_table[i].str_val, mfpStr)) {
            security->mfp = mfp_table[i].enum_val;
            break;
        }
    }
    if (mfp_table[i].str_val == NULL) {
        wifi_hal_error_print("%s:%d Unsupported MFP Value\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    if (security->mode == wifi_security_mode_none)
        return RETURN_OK;

    /* get wep key */
    if (security->mode == wifi_security_mode_wep_64 ||
        security->mode == wifi_security_mode_wep_128) {
        return RETURN_ERR; /* wep not supported */
    }
    /* get wpa encryption */
    if (wifi_getApWpaEncryptionMode(apIndex, encryption) < 0) {
        wifi_hal_error_print("%s:%d wifi_getApWpaEncryptionMode Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    for (i = 0; encryption_table[i].str_val != NULL; ++i) {
        if (!strcmp(encryption_table[i].str_val, encryption)) {
            security->encr = encryption_table[i].enum_val;
            break;
        }
    }

    if (encryption_table[i].str_val == NULL) {
        wifi_hal_error_print("%s:%d Unsupported Encryption\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* get wpa psk or passphrase */
    if (security->mode == wifi_security_mode_wpa_personal ||
        security->mode == wifi_security_mode_wpa2_personal ||
        security->mode == wifi_security_mode_wpa3_personal ||
        security->mode == wifi_security_mode_wpa_wpa2_personal ||
        security->mode == wifi_security_mode_wpa3_transition) {
        if (wifi_getApSecurityKeyPassphrase(apIndex, security->u.key.key) == 0) {
            if (security->mode == wifi_security_mode_wpa3_personal)
                security->u.key.type = wifi_security_key_type_sae;
            else if (security->mode == wifi_security_mode_wpa3_transition)
                security->u.key.type = wifi_security_key_type_psk_sae;
            else
                security->u.key.type = wifi_security_key_type_pass;
        } else { /* not passphrase, try to get psk */
            if (wifi_getApSecurityPreSharedKey(apIndex, security->u.key.key) == 0) {
                security->u.key.type = wifi_security_key_type_psk;
            } else {
                wifi_hal_error_print("%s:%d Failed ot get PSK or Passphrase\n", __func__, __LINE__);
                return RETURN_ERR;
            }
        }

        /* get wpa3_transition_disable */
    }
    /* get RADIUS auth server params */
    if (security->mode == wifi_security_mode_wpa_enterprise ||
        security->mode == wifi_security_mode_wpa2_enterprise ||
        security->mode == wifi_security_mode_wpa3_enterprise ||
        security->mode == wifi_security_mode_wpa_wpa2_enterprise) {
        char ip_txt[INET6_ADDRSTRLEN] = { '\0' };
        unsigned int port;

        /* get primary radius auth server ip addr, port, secret */
        if (wifi_getApSecurityRadiusServer(apIndex, ip_txt, &port, security->u.radius.key) < 0) {
            wifi_hal_error_print("%s:%d Failed ot get PSK or Passphrase\n", __func__, __LINE__);
            return RETURN_ERR;
        }
#if defined(WIFI_HAL_VERSION_3_PHASE2)
        if (inet_pton(AF_INET, ip_txt, &security->u.radius.ip.u.IPv4addr) > 0) {
            security->u.radius.ip.family = wifi_ip_family_ipv4;
        } else if (inet_pton(AF_INET6, ip_txt, security->u.radius.ip.u.IPv6addr) > 0) {
            security->u.radius.ip.family = wifi_ip_family_ipv6;
        } else {
            return RETURN_ERR;
        }
#else /* WIFI_HAL_VERSION_3_PHASE2 */
        snprintf(security->u.radius.ip, sizeof(security->u.radius.ip), ip_txt);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
        security->u.radius.port = (unsigned short)port;

        /* get secondary radius auth server ip addr, port, secret,
         * it is not an error if secondary radius server is not set */
        if (wifi_getApSecuritySecondaryRadiusServer(apIndex, ip_txt, &port,
                security->u.radius.s_key) == 0) {
#if defined(WIFI_HAL_VERSION_3_PHASE2)
            if (inet_pton(AF_INET, ip_txt, &security->u.radius.s_ip.u.IPv4addr) > 0) {
                security->u.radius.s_ip.family = wifi_ip_family_ipv4;
            } else if (inet_pton(AF_INET6, ip_txt, security->u.radius.s_ip.u.IPv6addr) > 0) {
                security->u.radius.s_ip.family = wifi_ip_family_ipv6;
            } else {
                return RETURN_ERR;
            }
#else /* WIFI_HAL_VERSION_3_PHASE2 */
            snprintf(security->u.radius.s_ip, sizeof(security->u.radius.s_ip), ip_txt);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

            security->u.radius.s_port = (unsigned short)port;
        }
    }

    return RETURN_OK;
}

INT wifi_getApWpsConfiguration(INT apIndex, wifi_wps_t *wpsConfig)
{
    char enabled_methods[512], *method, *rest;
    int i;

    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);

    memset(wpsConfig, 0, sizeof(*wpsConfig));

    if (wifi_getApWpsEnable(apIndex, &(wpsConfig->enable)) < 0) {
        wifi_hal_error_print("%s:%d wifi_getApWpsEnable Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    if (!wpsConfig->enable) {
        return RETURN_ERR;
    }

    /* Get WPS device PIN */
    unsigned long pin_num = strtoul(wpsConfig->pin, NULL, 10);
    if (wifi_getApWpsDevicePIN(apIndex, &pin_num) < 0) {
        wifi_hal_error_print("%s:%d wifi_getApWpsDevicePIN Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* Get enabled WPS config methods */
    if (wifi_getApWpsConfigMethodsEnabled(apIndex, enabled_methods) < 0) {
        wifi_hal_error_print("%s:%d  wifi_getApWpsConfigMethodsEnabled Failed\n", __func__,
            __LINE__);
        return RETURN_OK;
    }

    method = strtok_r(enabled_methods, ",", &rest);
    while (method != NULL) {
        i = 0;
        while (TRUE) {
            if (wps_config_method_table[i].str_val == NULL) {
                break;
            }

            if (!strcmp(method, wps_config_method_table[i].str_val)) {
                wpsConfig->methods |= wps_config_method_table[i].enum_val;
                break;
            }
            ++i;
        }
        method = strtok_r(NULL, ",", &rest);
    }

    return RETURN_OK;
}

#define MACF_TO_MAC(macstr, mac)                                                           \
    sscanf(macstr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[0], &mac[1], &mac[2], \
        &mac[3], &mac[4], &mac[5])

INT wifi_getRadioVapInfoMap(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);
    int i, apIndex, ret, len, enable, result = RETURN_OK;
    char bssid[18];

    map->num_vaps = MAX_NUM_VAP_PER_RADIO;
    apIndex = HAL_RADIO_IDX_TO_HAL_AP(index);
    for (i = 0; i < MAX_NUM_VAP_PER_RADIO; i++) {
        map->vap_array[i].vap_index = apIndex;
        map->vap_array[i].radio_index = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

        if (wifi_getSSIDName(apIndex, map->vap_array[i].u.bss_info.ssid) < 0) {
            wifi_hal_error_print("%s:%d wifi_getSSIDName Failed\n", __func__, __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getApEnable(apIndex, &(map->vap_array[i].u.bss_info.enabled)) < 0) {
            wifi_hal_error_print("%s:%d wifi_getApEnable Failed\n", __func__, __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getApSsidAdvertisementEnable(apIndex, &(map->vap_array[i].u.bss_info.showSsid)) <
            0) {
            wifi_hal_error_print("%s:%d wifi_getApSsidAdvertisementEnable Failed\n", __func__,
                __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getApManagementFramePowerControl(apIndex,
                &(map->vap_array[i].u.bss_info.mgmtPowerControl)) < 0) {
            wifi_hal_error_print("%s:% d wifi_getApManagementFramePowerControl Failed\n", __func__,
                __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getApMaxAssociatedDevices(apIndex, &(map->vap_array[i].u.bss_info.bssMaxSta)) <
            0) {
            wifi_hal_error_print("%s:%d wifi_getApMaxAssociatedDevices Failed\n", __func__,
                __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getBSSTransitionActivation(apIndex,
                &(map->vap_array[i].u.bss_info.bssTransitionActivated)) < 0) {
            wifi_hal_error_print("%s:%d wifi_getBSSTransitionActivation Failed\n", __func__,
                __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getNeighborReportActivation(apIndex,
                &(map->vap_array[i].u.bss_info.nbrReportActivated)) < 0) {
            wifi_hal_error_print("%s:%d  wifi_getNeighborReportActivation Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }

        ret = wifi_getApSecurity(apIndex, &(map->vap_array[i].u.bss_info.security));
        if (ret != RETURN_OK) {
            result = RETURN_ERR;
        }

        if (wifi_getApInterworkingElement(apIndex,
                &(map->vap_array[i].u.bss_info.interworking.interworking)) < 0) {
            wifi_hal_error_print("%s:%d  wifi_getApInterworkingElement Failed\n", __func__,
                __LINE__);
            result = RETURN_ERR;
        }

        if (wifi_getSSIDMACAddress(apIndex, bssid) < 0) {
            wifi_hal_error_print("%s:%d wifi_getSSIDMACAddress  Failed\n", __func__, __LINE__);
            result = RETURN_ERR;
        }
        MACF_TO_MAC(bssid, map->vap_array[i].u.bss_info.bssid);
        apIndex = HAL_VAP_AP_INDEX_NEXT(apIndex);
    }
    return result;
}

static int wl_str2uintArray(char *istr, char *delim, unsigned int *count, unsigned int *uarray,
    unsigned int maxCount)
{
    char *tmp_str, *tok, *rest;
    unsigned int i = 0;

    if ((istr == NULL) || (delim == NULL) || (count == NULL) || (uarray == NULL)) {
        return RETURN_ERR;
    }
    tmp_str = strdup(istr);
    if (tmp_str == NULL) {
        return RETURN_ERR;
    }
    tok = strtok_r(tmp_str, delim, &rest);
    while (tok && (i < maxCount)) {
        uarray[i] = atoi(tok);
        i++;
        tok = strtok_r(NULL, delim, &rest);
    }
    free(tmp_str);
    *count = i;
    return RETURN_OK;
}

static int wl_getRadioCapabilities(int radioIndex, wifi_radio_capabilities_t *rcap)
{
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);
    char xpwrSupStr[OUTPUT_STRING_LENGTH_256], encModeStr[OUTPUT_STRING_LENGTH_256], *abbrev,
        cbuf[OUTPUT_STRING_LENGTH_1024];
    char channelbuff[OUTPUT_STRING_LENGTH_1024] = { 0 };
    int i, j, ccnt, ret, numbands, bandis2g, bcnt, acnt;
    uint32_t possibleChannels[MAX_CHANNELS + 1] = { 0 };
    wifi_channels_list_t *chlistp, *bchlistp, *achlistp;
    wifi_channelBandwidth_t *chbwp;
    wifi_enum_to_str_map_t *cctentry;
    int count = 0;
    /* numSupportedFreqBand - if dual band is 2G and 5G capable */
    rcap->numSupportedFreqBand = 1;
    numbands = rcap->numSupportedFreqBand;

    /* channel_list - gets all for dualband as well */
    if (wifi_getRadioPossibleChannels(radioIndex, channelbuff) < 0) {
        wifi_hal_error_print("%s:%d  wifi_getRadioPossibleChannels Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    char *data = strchr(channelbuff, '=');
    if (data != NULL) {
        data++;

        char *token = strtok(data, ",");
        while (token != NULL) {
            possibleChannels[count++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }

    // Hard coding Bnads here.
    if (radioIndex == 0) {
        rcap->band[0] = WIFI_FREQUENCY_2_4_BAND;
    } else if (radioIndex == 1) {
        rcap->band[0] = WIFI_FREQUENCY_5_BAND;
    }

    if (numbands == 1) {
        chlistp = &(rcap->channel_list[0]);
        chlistp->num_channels = count;
        for (j = 0; j < count; j++) {
            chlistp->channels_list[j] = possibleChannels[j];
        }
    }
    for (i = 0; i < numbands; i++) {
        /* channelWidth - all supported bandwidths */
        rcap->channelWidth[i] = 0;
        if (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) {
            rcap->channelWidth[i] |= (WIFI_CHANNELBANDWIDTH_20MHZ | WIFI_CHANNELBANDWIDTH_40MHZ);
        } else if (rcap->band[i] & (WIFI_FREQUENCY_5_BAND | WIFI_FREQUENCY_6_BAND)) {
            rcap->channelWidth[i] |= (WIFI_CHANNELBANDWIDTH_20MHZ | WIFI_CHANNELBANDWIDTH_40MHZ |
                WIFI_CHANNELBANDWIDTH_80MHZ | WIFI_CHANNELBANDWIDTH_160MHZ);
        }

        /* mode - all supported variants */
        rcap->mode[i] = WIFI_80211_VARIANT_H;
        if (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) {
            rcap->mode[i] |= (WIFI_80211_VARIANT_G | WIFI_80211_VARIANT_N | WIFI_80211_VARIANT_AX);
        } else if (rcap->band[i] & WIFI_FREQUENCY_5_BAND) {
            rcap->mode[i] |= (WIFI_80211_VARIANT_A | WIFI_80211_VARIANT_N | WIFI_80211_VARIANT_AC |
                WIFI_80211_VARIANT_AX);
        } else if (rcap->band[i] & (WIFI_FREQUENCY_6_BAND)) {
            rcap->mode[i] |= WIFI_80211_VARIANT_AX;
        }

        /* maxBitRate - from mcs_rate_tbl, pick max values for each band
         *      Standard,   BW, GI, NSS,    Rate
         * 2.4G:    {"ax",      "40",   1,  4,  1147},
         * 5G:      {"ax",      "160",  1,  4,  4804},
         * 6G:      {"ax",      "160",  1,  8,  9608},
         */
        rcap->maxBitRate[i] = (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) ?
            1147 :
            ((rcap->band[i] & WIFI_FREQUENCY_5_BAND) ?
                    4804 :
                    ((rcap->band[i] & (WIFI_FREQUENCY_6_BAND) ? 9608 : 0)));

        /* supportedBitRate - all supported bitrates */
        rcap->supportedBitRate[i] = 0;
        if (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) {
            rcap->supportedBitRate[i] |= (WIFI_BITRATE_1MBPS | WIFI_BITRATE_2MBPS |
                WIFI_BITRATE_5_5MBPS | WIFI_BITRATE_6MBPS | WIFI_BITRATE_9MBPS |
                WIFI_BITRATE_11MBPS | WIFI_BITRATE_12MBPS);
        } else if (rcap->band[i] & (WIFI_FREQUENCY_5_BAND | WIFI_FREQUENCY_6_BAND)) {
            rcap->supportedBitRate[i] |= (WIFI_BITRATE_6MBPS | WIFI_BITRATE_9MBPS |
                WIFI_BITRATE_12MBPS | WIFI_BITRATE_18MBPS | WIFI_BITRATE_24MBPS |
                WIFI_BITRATE_36MBPS | WIFI_BITRATE_48MBPS | WIFI_BITRATE_54MBPS);
        }

        /* transmitPowerSupported_list - refer wifi_getRadioTransmitPowerSupported */
        ret = wifi_getRadioTransmitPowerSupported(radioIndex, xpwrSupStr);
        if (ret < 0) {
            wifi_hal_error_print("%s:%d  wifi_getRadioPossibleChannels Failed\n", __func__,
                __LINE__);
            return RETURN_ERR;
        }
        /* Allow comma or space delimiters in string */
        wl_str2uintArray(xpwrSupStr, ", ", &(rcap->transmitPowerSupported_list[i].numberOfElements),
            rcap->transmitPowerSupported_list[i].transmitPowerSupported,
            MAXNUMBEROFTRANSMIPOWERSUPPORTED);
    } /* for numbands */

    /* autoChannelSupported */
    /* always ON with wifi_getRadioAutoChannelSupported */
    rcap->autoChannelSupported = TRUE;

    /* DCSSupported */
    /* always ON with wifi_getRadioDCSSupported */
    rcap->DCSSupported = TRUE;

    /* zeroDFSSupported - TBD */
    rcap->zeroDFSSupported = FALSE;

    /* csi */
    rcap->csi.maxDevices = CSI_MAX_DEVICES_BRCM;
    rcap->csi.soudingFrameSupported = FALSE; // Setting to false as we dont have the API

    snprintf(rcap->ifaceName, MAXIFACENAMESIZE, "radio%d", radioIndex);

    /* cipher */
    // Harcoding to support all as of now.
    rcap->cipherSupported |= WIFI_CIPHER_CAPA_ENC_CCMP_256 | WIFI_CIPHER_CAPA_ENC_GCMP_256 |
        WIFI_CIPHER_CAPA_ENC_CCMP | WIFI_CIPHER_CAPA_ENC_GCMP | WIFI_CIPHER_CAPA_ENC_TKIP |
        WIFI_CIPHER_CAPA_ENC_BIP | WIFI_CIPHER_CAPA_ENC_BIP_GMAC_128 |
        WIFI_CIPHER_CAPA_ENC_BIP_GMAC_256 | WIFI_CIPHER_CAPA_ENC_BIP_CMAC_256;

    if (wifi_getRadioCountryCode(radioIndex, cbuf) < 0) {
        wifi_hal_error_print("%s:%d wifi_getRadioCountryCode Failed\n", __func__, __LINE__);
    }

    for (j = 0; j < wifi_countrycode_max; j++) {
        cctentry = &(countrycode_table[j]);
        if ((strstr(cctentry->str_val, cbuf))) {
            /* match */
            rcap->countrySupported[1] = cctentry->enum_val;
            break;
        }
    } /* for j */
    rcap->numcountrySupported = 1;

    rcap->maxNumberVAPs = MAX_NUM_VAP_PER_RADIO;

    return RETURN_OK;
}

INT wifi_getHalCapability(wifi_hal_capability_t *cap)
{
    int i, len, ret, bsdSupport = 0;
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);

    memset(cap, 0, sizeof(wifi_hal_capability_t));

    /* version */
    cap->version.major = WIFI_HAL_MAJOR_VERSION;
    cap->version.minor = WIFI_HAL_MINOR_VERSION;

    /* platform_property */
    // hardcoding number of Radios.
    cap->wifi_prop.numRadios = MAX_NUM_RADIOS;

    /* get RadioCapabilities */
    for (i = 0; i < cap->wifi_prop.numRadios; i++) {
        ret = wl_getRadioCapabilities(i, &(cap->wifi_prop.radiocap[i]));
        if (ret < 0) {
            wifi_hal_error_print("%s:%d  wl_getRadioCapabilities Failed\n", __func__, __LINE__);
            return RETURN_ERR;
        }
    }

    if (wifi_getBandSteeringCapability(&(cap->BandSteeringSupported)) < 0) {
        wifi_hal_error_print("%s:%d  wifi_getBandSteeringCapability Failed\n", __func__, __LINE__);
    }
    return RETURN_OK;
}

static INT wl_setRadioMode(INT radioIndex, CHAR *channelMode, UINT pureMode)
{
    int ret, len;
    char *bwCapStr = NULL;

    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);

    if (strstr(channelMode, "320")) {
        bwCapStr = "320MHz";
    } else if (strstr(channelMode, "40")) {
        bwCapStr = "40MHz";
    } else if (strstr(channelMode, "80")) {
        bwCapStr = "80MHz";
    } else if (strstr(channelMode, "160")) {
        bwCapStr = "160MHz";
    } else if (strstr(channelMode, "20")) {
        bwCapStr = "20MHz";
    }

    if (wifi_setRadioOperatingChannelBandwidth(radioIndex, bwCapStr) < 0) {
        wifi_hal_error_print("%s:%d  wifi_setRadioOperatingChannelBandwidth Failed\n", __func__,
            __LINE__);
        return RETURN_ERR;
    }
    // Can't change Operating Standards.
    return RETURN_OK;
}

INT wifi_setRadioOperatingParameters(wifi_radio_index_t index,
    wifi_radio_operationParam_t *operationParam)
{
    int len, tblSize, i, ret = 0;
    unsigned int local_beacon_period;
    char channelMode[OUTPUT_STRING_LENGTH_32] = "11", variant[OUTPUT_STRING_LENGTH_32];
    char operStd[OUTPUT_STRING_LENGTH_32] = { 0 };
    unsigned int pureMode = PMODE_NONE, tempVariant = 0;
    bool enable;

    len = sizeof(enable);
    // RadioEnan;e
    enable = operationParam->enable;
    if (wifi_setRadioEnable(index, enable) < 0) {
        wifi_hal_error_print("%s:%d  wifi_setRadioEnable Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* enable/disable auto channel mode */
    enable = operationParam->autoChannelEnabled;
    if (wifi_setRadioAutoChannelEnable(index, enable) < 0) {
        wifi_hal_error_print("%s:%d  wifi_setRadioAutoChannelEnable Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    /* Translate Variant to oper std mode and append to channel mode */
    tempVariant = operationParam->variant & OPER_STANDS_MASK;
    tblSize = ARRAY_SIZE(std2ieee80211Variant_infoTable);
    for (i = tblSize - 1; i >= 0; i--) {
        if (tempVariant & std2ieee80211Variant_infoTable[i].enum_val) {
            strcat(operStd, std2ieee80211Variant_infoTable[i].str_val);
            strcat(operStd, ",");
        }
    }

    /* operStd to channelMode such as 11AX80 and pureMode value as in PMODE_xxx */
    len = strlen(operStd);
    operStd[len - 1] = '\0';
    for (i = 0; operStdPMode_infoTable[i].operStd != NULL; i++) {
        if (!(strcmp(operStdPMode_infoTable[i].operStd, operStd))) {
            /* match */
            snprintf(variant, sizeof(variant), operStdPMode_infoTable[i].pmodeStr);
            str_to_upper(variant);
            strcat(channelMode, variant);
            pureMode = operStdPMode_infoTable[i].pmodeVal;
            break;
        }
    }

    /* Translate bandwidth and append to channel mode */
    tblSize = ARRAY_SIZE(wifi_bandwidth_infoTable);
    for (i = 0; i < tblSize; i++) {
        if (operationParam->channelWidth == wifi_bandwidth_infoTable[i].enum_val) {
            strcat(channelMode, wifi_bandwidth_infoTable[i].str_val);
            break;
        }
    }
    if (i == tblSize) {
        return RETURN_ERR;
    }

    /* Set the beacon Interval */
    wifi_getRadioBeaconPeriod(index, &local_beacon_period);
    if (local_beacon_period != operationParam->beaconInterval) {
        if (operationParam->beaconInterval) {
            if (wifi_setRadioBeaconPeriod(index, operationParam->beaconInterval) < 0) {
                wifi_hal_error_print("%s:%d wifi_pushRadioChannel2 \n", __func__, __LINE__);
                return RETURN_ERR;
            }
        }
    }

    /* Set oper std mode and bandwidth */
    if (wl_setRadioMode(index, channelMode, pureMode) < 0) {
        return RETURN_ERR;
    }

    if (!operationParam->autoChannelEnabled) {
        if (wifi_setRadioChannel(index, operationParam->channel) < 0) {
            wifi_hal_error_print("%s:%d wifi_setRadioChannel \n", __func__, __LINE__);
            return RETURN_ERR;
        }
        if (operationParam->csa_beacon_count) {
            /* Send CSA */
            if (wifi_pushRadioChannel2(index, operationParam->channel, operationParam->channelWidth,
                    operationParam->csa_beacon_count) < 0) {
                wifi_hal_error_print("%s:%d wifi_pushRadioChannel2 \n", __func__, __LINE__);
                return RETURN_ERR;
            }
        }
    }

    return RETURN_OK;
}

int get_security_mode_int_from_str(char *security_mode_str,char *mfp_str,wifi_security_modes_t *security_mode)
{

    if(strcmp(security_mode_str, "None") == 0) {
        *security_mode = wifi_security_mode_none;
    } else if (strcmp(security_mode_str, "owe") == 0) {
        *security_mode = wifi_security_mode_enhanced_open;
    } else if (strcmp(security_mode_str, "psk") == 0) {
        *security_mode = wifi_security_mode_wpa_personal;
    } else if (strcmp(security_mode_str, "psk2") == 0) {
        *security_mode = wifi_security_mode_wpa2_personal;
    } else if (strcmp(security_mode_str, "psk psk2") == 0) {
        *security_mode = wifi_security_mode_wpa_wpa2_personal;
    } else if ((strstr(security_mode_str, "sae") != NULL) && (strstr(security_mode_str, "psk2") == NULL)) {
        /* should also take care of "sae sae-ext" case regardless of order */
        *security_mode = wifi_security_mode_wpa3_personal;
    } else if (strstr(security_mode_str, "psk2") && strstr(security_mode_str, "sae")) {
        /* should also take care of "psk2 sae sae-ext" case regardless of order */
        *security_mode = wifi_security_mode_wpa3_transition;
    } else if (strcmp(security_mode_str, "wpa") == 0) {
        *security_mode = wifi_security_mode_wpa_enterprise;
    } else if ((strcmp(security_mode_str, "wpa2") == 0) && (strcmp(mfp_str, "2") != 0 )) {
        *security_mode = wifi_security_mode_wpa2_enterprise;
    } else if ((strcmp(security_mode_str, "wpa2") == 0) && (strcmp(mfp_str, "2") == 0 )){
        *security_mode = wifi_security_mode_wpa3_enterprise;
    } else if (strcmp(security_mode_str, "wpa wpa2") == 0) {
        *security_mode = wifi_security_mode_wpa_wpa2_enterprise;
    } else if (strstr(security_mode_str, "psk2") && strstr(security_mode_str, "sae") && !strcmp(mfp_str, "0")) {
        *security_mode = wifi_security_mode_wpa3_compatibility;
    } else {
        wifi_hal_error_print("%s:%d: wifi security mode not found:[%s:%s]\r\n",__func__, __LINE__, security_mode_str,mfp_str);
        return RETURN_ERR;
    }

    wifi_hal_dbg_print("%s:%d: security mode %d string %s and mfp is %s\r\n",__func__, __LINE__, *security_mode,security_mode_str,mfp_str);
    return RETURN_OK;
}

INT wifi_hal_setApWpsPin(INT ap_index, char *wps_pin)
{
    unsigned long val = strtoul(wps_pin, NULL, 10);
    wifi_setApWpsDevicePIN(ap_index, val);
    return RETURN_OK;
}

// Wrapper as Prototype is different in HAL2.19 and HAL3.0.
INT wifi_hal_kickAssociatedDevice(INT apIndex, mac_address_t mac_addr)
{
    wifi_hal_dbg_print("%s:%d Inside \n", __func__, __LINE__);

    char bmac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    if (memcmp(mac_addr, bmac, sizeof(bmac))) {
        // not Handling Kick all as of now.
        return RETURN_OK;
    }

    mac_addr_str_t mac_str;

    to_mac_str(mac_addr, mac_str);
    if (wifi_kickApAssociatedDevice(apIndex, mac_str) < 0) {
        wifi_hal_error_print("%s:%d wifi_kickApAssociatedDevice Failed\n", __func__, __LINE__);
        return RETURN_ERR;
    }

    return RETURN_OK;
}

INT wifi_hal_startNeighborScan(INT apIndex, wifi_neighborScanMode_t scan_mode, INT dwell_time,
    UINT chan_num, UINT *chan_list)
{
    return wifi_startNeighborScan(apIndex, scan_mode, dwell_time, chan_num, chan_list);
}

INT wifi_hal_getNeighboringWiFiStatus(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array,
    UINT *output_array_size)
{
    return wifi_getNeighboringWiFiStatus(radioIndex, neighbor_ap_array, output_array_size);
}

// STUBS
INT wifi_hal_get_default_keypassphrase(char *password, int vap_index)
{
    return RETURN_OK;
}

INT wifi_hal_wifi_send_mgmt_frame_responseget_default_ssid(char *ssid, int vap_index)
{
    return RETURN_OK;
}

INT wifi_hal_get_default_country_code(char *code)
{
    return RETURN_OK;
}

INT wifi_get_default_wps_pin(char *pin)
{
    return RETURN_OK;
}

INT wifi_hal_configNeighborReports(UINT apIndex, bool enable, bool auto_resp)
{
    return RETURN_OK;
}

INT wifi_hal_set_acs_keep_out_chans(wifi_radio_operationParam_t *wifi_radio_oper_param,
    int radioIndex)
{
    return RETURN_OK;
}

INT wifi_hal_send_mgmt_frame_response(int ap_index, int type, int status, int status_code,
    uint8_t *frame, uint8_t *mac, int len, int rssi)
{
    return RETURN_OK;
}

bool is_db_upgrade_required(char *inactive_firmware)
{
    return false;
}

INT wifi_hal_startScan(wifi_radio_index_t index, wifi_neighborScanMode_t scan_mode, INT dwell_time,
    UINT num, UINT *chan_list)
{
    return RETURN_OK;
}

void wifi_hal_ap_max_client_rejection_callback_register(wifi_apMaxClientRejection_callback func)
{
    return;
}

void wifi_hal_radius_eap_failure_callback_register(wifi_radiusEapFailure_callback func)
{
    return;
}

INT wifi_hal_disconnect(INT ap_index)
{
    return RETURN_OK;
}

void wifi_hal_disassoc(int vap_index, int status, uint8_t *mac)
{
    return;
}

INT wifi_hal_connect(INT ap_index, wifi_bss_info_t *bss)
{
    return RETURN_OK;
}

void wifi_hal_register_frame_hook(wifi_hal_frame_hook_fn_t func)
{
    return;
}

INT wifi_vapstatus_callback_register(wifi_vapstatus_callback func)
{
    return RETURN_OK;
}

INT wifi_hal_getRadioTemperature(wifi_radio_index_t radioIndex,
    wifi_radioTemperature_t *radioPhyTemperature)
{
    // Going to affect Levl Funtionality.
    return RETURN_OK;
}

INT wifi_hal_pre_init()
{
    return RETURN_OK;
}

INT wifi_hal_post_init(wifi_vap_info_map_t *vap_map)
{
    return RETURN_OK;
}
INT wifi_anqpSendResponse(UINT apIndex, mac_address_t sta, unsigned char token,
    wifi_anqp_node_t *head)
{
    return RETURN_OK;
}

void wifi_hal_staConnectionStatus_callback_register(wifi_staConnectionStatus_callback func)
{
    return;
}

INT wifi_setGASConfiguration(UINT advertisementID, wifi_GASConfiguration_t *input_struct)
{
    return RETURN_OK;
}

INT wifi_sendActionFrameExt(INT apIndex, mac_address_t MacAddr, UINT frequency, UINT wait,
    UCHAR *frame, UINT len)
{
    return RETURN_OK;
}

int enablePassPointSettings(int ap_index, BOOL passpoint_enable, BOOL downstream_disable, BOOL p2p_disable, BOOL layer2TIF)
{
    return 0;
}

INT wifi_anqp_request_callback_register(wifi_anqp_request_callback_t anqpReqCallback)
{
    return RETURN_OK;
}

INT wifi_hal_setApWpsCancel(INT ap_index)
{
    return RETURN_OK;
}

void wifi_hal_scanResults_callback_register(wifi_scanResults_callback func)
{
    return;
}

INT wifi_hal_getScanResults(wifi_radio_index_t index, wifi_channel_t *channel,
    wifi_bss_info_t **bss, UINT *num_bss)
{
    return RETURN_OK;
}

INT wifi_hal_get_default_radius_key(char *radius_key)
{
    return RETURN_OK;
}

void wifi_hal_set_neighbor_report(uint apIndex, uint add, mac_address_t mac)
{
    wifi_hal_info_print("%s:%d Enter %d\n", __func__, __LINE__, apIndex);
    wifi_NeighborReport_t nbr_report;
    memcpy(nbr_report.bssid,mac,sizeof(mac_address_t));
    wifi_setNeighborReports(apIndex ,add, &nbr_report);

    return;
}

INT wifi_hal_mgmt_frame_callbacks_register(wifi_receivedMgmtFrame_callback func)
{
    return RETURN_OK;
}

INT wifi_hal_sendDataFrame( int vap_id, unsigned char *dmac, unsigned char *data_buff, int data_len, BOOL insert_llc, int protocol, int priority)
{
    return RETURN_OK;
}

INT wifi_chan_event_register(wifi_chan_event_CB_t event_cb)
{
    return RETURN_OK;
}

INT wifi_hal_get_default_ssid(char *ssid, int vap_index)
{
    return RETURN_OK;
}

INT wifi_hal_get_default_wps_pin(char *pin)
{
    return RETURN_OK;
}

// platform Specific APIs, used by OneWifi.
int platform_get_channel_bandwidth(wifi_radio_index_t index, wifi_channelBandwidth_t *channelWidth)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return 0;
}

int nvram_get_current_ssid(char *l_ssid, int vap_index)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}

int nvram_get_vap_enable_status(bool *vap_enable, int vap_index)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}

int nvram_get_default_password(char *l_password, int vap_index)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}

int nvram_get_current_password(char *l_password, int vap_index)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}

int nvram_get_mgmt_frame_power_control(int vap_index, int *output_dbm)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}

int nvram_get_current_security_mode(wifi_security_modes_t *security_mode, int vap_index)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}

int nvram_get_radio_enable_status(bool *radio_enable, int radio_index)
{
    // called in OneWIfi migration scenario to update wifi db from PSM.
    return RETURN_OK;
}
