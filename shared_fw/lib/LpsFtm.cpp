#include "LpsFtm.h"

// Initialize the static variables
float LpsFtm::latestDistance = -1.0;
uint32_t LpsFtm::latestRtt = 0;
int LpsFtm::latestStatus = -1;
bool LpsFtm::newDataAvailable = false;

LpsFtm::LpsFtm() {
    // Constructor
}

void LpsFtm::initResponder(const char* ssid, const char* password) {
    // FTM Responders operate in Access Point mode
    WiFi.mode(WIFI_AP);
    
    // Start the AP. If password is null, it creates an open network
    if (password == nullptr) {
        WiFi.softAP(ssid);
    } else {
        WiFi.softAP(ssid, password);
    }

    // Enable the FTM Responder hardware flag
    WiFi.enableFtmResponder(true);
    
    Serial.printf("FTM Responder Initialized. SSID: %s\n", ssid);
    Serial.printf("Target MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
}

void LpsFtm::initInitiator() {
    // FTM Initiators operate in Station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Register the static callback function to catch FTM hardware reports
    WiFi.onEvent(LpsFtm::onFtmReport, ARDUINO_EVENT_WIFI_FTM_REPORT);
    
    Serial.println("FTM Initiator Initialized. Ready to range.");
}

bool LpsFtm::requestRanging(const uint8_t* targetMac, uint8_t channel) {
    // Trigger the FTM session. The ESP32 supports setting custom burst sizes,
    // but the standard request uses the Arduino core defaults.
    return WiFi.requestFtm(targetMac, channel);
}

void LpsFtm::onFtmReport(arduino_event_id_t event, arduino_event_info_t info) {
    if (event == ARDUINO_EVENT_WIFI_FTM_REPORT) {
        // Extract the FTM payload from the event info union
        wifi_event_ftm_report_t* report = &info.wifi_ftm_report;
        
        latestStatus = report->status;
        
        // FTM_STATUS_SUCCESS is 0
        if (report->status == FTM_STATUS_SUCCESS) {
            // ESP32 reports FTM distance in centimeters. Convert to meters.
            latestDistance = (float)report->dist_est / 100.0;
            latestRtt = report->rtt;
            newDataAvailable = true;
        } else {
            latestDistance = -1.0; // Indicate failure
            newDataAvailable = false;
        }
    }
}

float LpsFtm::getLatestDistance() {
    return latestDistance;
}

uint32_t LpsFtm::getLatestRTT() {
    return latestRtt;
}

int LpsFtm::getLatestStatus() {
    return latestStatus;
}

bool LpsFtm::isNewDataAvailable() {
    return newDataAvailable;
}

void LpsFtm::clearDataFlag() {
    newDataAvailable = false;
}