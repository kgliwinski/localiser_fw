#include "LpsFtmInitiator.h"

#include <cstdio>
#include <cstring>
#include <esp_idf_version.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

std::optional<float> LpsFtmInitiator::latestDistanceValue = std::nullopt;
uint32_t LpsFtmInitiator::latestRtt = 0;
int LpsFtmInitiator::latestStatusValue = -1;
bool LpsFtmInitiator::newDataAvailable = false;
SemaphoreHandle_t LpsFtmInitiator::ftmReportSemaphore = nullptr;
uint8_t LpsFtmInitiator::ftmReportEntryCount = 0;

LpsFtmInitiator::LpsFtmInitiator(
    const uint8_t* targetMac,
    const char* targetSsid,
    const char* targetPassword,
    uint8_t frameCount,
    uint16_t burstPeriod,
    uint8_t targetChannel
)
    : targetPassword(targetPassword != nullptr ? targetPassword : ""),
      targetSsid(targetSsid),
      hasPassword(targetPassword != nullptr),
      frameCount(frameCount),
      burstPeriod(burstPeriod),
      targetChannel(targetChannel) {
    memcpy(this->targetMac, targetMac, sizeof(this->targetMac));
    ftmReportSemaphore = xSemaphoreCreateBinary();
    connectedSemaphore = xSemaphoreCreateBinary();
}

bool LpsFtmInitiator::begin() {
    if (ftmReportSemaphore == nullptr || connectedSemaphore == nullptr) {
        printf("Failed to create FTM synchronization objects.\n");
        return false;
    }

    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        result = nvs_flash_init();
    }
    if (result != ESP_OK) {
        printf("Failed to initialize NVS: %s\n", esp_err_to_name(result));
        return false;
    }
    if ((result = esp_netif_init()) != ESP_OK && result != ESP_ERR_INVALID_STATE) {
        printf("Failed to initialize network interface: %s\n", esp_err_to_name(result));
        return false;
    }
    result = esp_event_loop_create_default();
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE) {
        printf("Failed to create event loop: %s\n", esp_err_to_name(result));
        return false;
    }
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
    if ((result = esp_wifi_init(&initConfig)) != ESP_OK ||
        (result = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
        printf("Failed to initialize station Wi-Fi: %s\n", esp_err_to_name(result));
        return false;
    }

    wifi_config_t stationConfig{};
    memcpy(stationConfig.sta.ssid, targetSsid.c_str(), targetSsid.size() + 1);
    if (hasPassword) {
        memcpy(stationConfig.sta.password, targetPassword.c_str(), targetPassword.size() + 1);
        stationConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    if ((result = esp_wifi_set_config(WIFI_IF_STA, &stationConfig)) != ESP_OK) {
        printf("Failed to configure station: %s\n", esp_err_to_name(result));
        return false;
    }

    if ((result = esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &LpsFtmInitiator::onWifiEvent,
        this, &ftmEventHandler
    )) != ESP_OK) {
        printf("Failed to register Wi-Fi handler: %s\n", esp_err_to_name(result));
        return false;
    }
    esp_event_handler_instance_t reportHandler = nullptr;
    if ((result = esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &LpsFtmInitiator::onFtmReport,
        nullptr, &reportHandler
    )) != ESP_OK) {
        printf("Failed to register FTM handler: %s\n", esp_err_to_name(result));
        return false;
    }
    if ((result = esp_wifi_start()) != ESP_OK || (result = esp_wifi_connect()) != ESP_OK) {
        printf("Failed to start/connect station Wi-Fi: %s\n", esp_err_to_name(result));
        return false;
    }
    if (xSemaphoreTake(connectedSemaphore, pdMS_TO_TICKS(10000)) != pdPASS) {
        printf("Timed out connecting to FTM target: %s\n", targetSsid.c_str());
        return false;
    }

    wifi_ap_record_t apRecord{};
    if ((result = esp_wifi_sta_get_ap_info(&apRecord)) != ESP_OK) {
        printf("Failed to read connected AP information: %s\n", esp_err_to_name(result));
        return false;
    }
    memcpy(targetMac, apRecord.bssid, sizeof(targetMac));
    targetChannel = apRecord.primary;
    printf("FTM Initiator connected to %s on channel %u.\n", targetSsid.c_str(), targetChannel);
    return true;
}

bool LpsFtmInitiator::requestRanging() {
    wifi_ap_record_t apRecord{};
    if (esp_wifi_sta_get_ap_info(&apRecord) != ESP_OK) {
        latestStatusValue = -1;
        latestDistanceValue = std::nullopt;
        return false;
    }

    latestDistanceValue = std::nullopt;
    newDataAvailable = false;
    ftmReportEntryCount = 0;
    xSemaphoreTake(ftmReportSemaphore, 0);

    wifi_ftm_initiator_cfg_t config{};
    memcpy(config.resp_mac, targetMac, sizeof(config.resp_mac));
    config.channel = targetChannel;
    config.frm_count = frameCount;
    config.burst_period = burstPeriod;
    config.use_get_report_api = true;

    const esp_err_t result = esp_wifi_ftm_initiate_session(&config);
    if (result != ESP_OK) {
        latestStatusValue = -1;
        printf("Failed to initiate native FTM session: %s\n", esp_err_to_name(result));
        return false;
    }
    if (xSemaphoreTake(ftmReportSemaphore, pdMS_TO_TICKS(10000)) != pdPASS) {
        latestStatusValue = -1;
        printf("Timed out waiting for native FTM report.\n");
        return false;
    }

    if (latestStatusValue == FTM_STATUS_SUCCESS && ftmReportEntryCount > 0) {
        auto* report = static_cast<wifi_ftm_report_entry_t*>(
            calloc(ftmReportEntryCount, sizeof(wifi_ftm_report_entry_t))
        );
        if (report == nullptr) {
            latestStatusValue = -1;
            printf("Failed to allocate native FTM report.\n");
            return false;
        }
        const esp_err_t reportResult = esp_wifi_ftm_get_report(report, ftmReportEntryCount);
        free(report);
        if (reportResult != ESP_OK) {
            latestStatusValue = -1;
            printf("Failed to retrieve native FTM report: %s\n", esp_err_to_name(reportResult));
            return false;
        }
    }
    return latestStatusValue == FTM_STATUS_SUCCESS;
}

std::string LpsFtmInitiator::macAddress() const {
    uint8_t mac[6]{};
    if (esp_wifi_get_mac(WIFI_IF_STA, mac) != ESP_OK) {
        return {};
    }
    char formatted[18]{};
    snprintf(formatted, sizeof(formatted), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return formatted;
}

void LpsFtmInitiator::onWifiEvent(void* context, esp_event_base_t eventBase,
                                  int32_t eventId, void*) {
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_CONNECTED) {
        xSemaphoreGive(static_cast<LpsFtmInitiator*>(context)->connectedSemaphore);
    }
}

void LpsFtmInitiator::onFtmReport(void*, esp_event_base_t eventBase,
                                  int32_t eventId, void* eventData) {
    if (eventBase != WIFI_EVENT || eventId != WIFI_EVENT_FTM_REPORT || eventData == nullptr) {
        return;
    }
    auto* report = static_cast<wifi_event_ftm_report_t*>(eventData);
    latestStatusValue = report->status;
    latestRtt = report->rtt_est;
    ftmReportEntryCount = report->ftm_report_num_entries;
    if (report->status == FTM_STATUS_SUCCESS) {
        latestDistanceValue = static_cast<float>(report->dist_est) / 100.0F;
        newDataAvailable = true;
    } else {
        latestDistanceValue = std::nullopt;
        newDataAvailable = false;
        printf("FTM failed with status: %d\n", report->status);
    }
    xSemaphoreGive(ftmReportSemaphore);
}

std::optional<float> LpsFtmInitiator::latestDistance() { return latestDistanceValue; }
uint32_t LpsFtmInitiator::latestRTT() { return latestRtt; }
int LpsFtmInitiator::latestStatus() { return latestStatusValue; }
bool LpsFtmInitiator::isNewDataAvailable() { return newDataAvailable; }
void LpsFtmInitiator::clearDataFlag() { newDataAvailable = false; }
