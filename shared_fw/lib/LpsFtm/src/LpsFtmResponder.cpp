#include "LpsFtmResponder.h"
#include <WiFi.h>

LpsFtmResponder::LpsFtmResponder(const char* ssid, const char* password)
    : networkName(ssid),
      networkPassword(password != nullptr ? password : ""),
      hasPassword(password != nullptr) {}

bool LpsFtmResponder::begin() {
    WiFi.mode(WIFI_AP);

    const bool started = WiFi.softAP(
        networkName.c_str(),
        hasPassword ? networkPassword.c_str() : nullptr,
        1,
        0,
        4,
        true
    );

    if (!started) {
        Serial.printf("Failed to initialize FTM Responder. SSID: %s\n", ssid());
        return false;
    }

    Serial.printf("FTM Responder Initialized. SSID: %s\n", ssid());
    Serial.printf("Target MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
    return true;
}

const char* LpsFtmResponder::ssid() const {
    return networkName.c_str();
}
