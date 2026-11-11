#include "LpsFtmResponder.h"

#include <cstdio>
#include <cstring>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

LpsFtmResponder::LpsFtmResponder(const char* ssid, const char* password)
    : networkName(ssid), networkPassword(password != nullptr ? password : ""),
      hasPassword(password != nullptr) {}

bool LpsFtmResponder::begin() {
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
        return false;
    }
    result = esp_event_loop_create_default();
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE) {
        return false;
    }
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t config{};
    memcpy(config.ap.ssid, networkName.c_str(), networkName.size() + 1);
    config.ap.ssid_len = networkName.size();
    config.ap.channel = 1;
    config.ap.max_connection = 4;
    config.ap.authmode = hasPassword ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    config.ap.ftm_responder = true;
    if (hasPassword) {
        memcpy(config.ap.password, networkPassword.c_str(), networkPassword.size() + 1);
    }
    if ((result = esp_wifi_init(&initConfig)) != ESP_OK ||
        (result = esp_wifi_set_mode(WIFI_MODE_AP)) != ESP_OK ||
        (result = esp_wifi_set_config(WIFI_IF_AP, &config)) != ESP_OK ||
        (result = esp_wifi_start()) != ESP_OK) {
        printf("Failed to start FTM responder: %s\n", esp_err_to_name(result));
        return false;
    }
    printf("FTM Responder Initialized. SSID: %s\n", ssid());
    printf("Target MAC Address: %s\n", macAddress().c_str());
    return true;
}

const char* LpsFtmResponder::ssid() const { return networkName.c_str(); }

std::string LpsFtmResponder::macAddress() const {
    uint8_t mac[6]{};
    if (esp_wifi_get_mac(WIFI_IF_AP, mac) != ESP_OK) {
        return {};
    }
    char formatted[18]{};
    snprintf(formatted, sizeof(formatted), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return formatted;
}
