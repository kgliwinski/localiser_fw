#pragma once

#include <Arduino.h>
#include <WiFi.h>

class LpsFtm {
public:
    LpsFtm();

    // --- Configuration Modes ---
    // Target Device: Starts a SoftAP and enables the FTM responder
    void initResponder(const char* ssid, const char* password = nullptr);
    
    // Anchor Device: Starts STA mode and registers the FTM report listener
    void initInitiator();

    // --- Actions (Anchor Only) ---
    // Triggers the 802.11mc FTM burst to a specific MAC address
    bool requestRanging(const uint8_t* targetMac, uint8_t channel = 1);

    // --- Getters (Anchor Only) ---
    // Returns distance in meters. Returns -1.0 if the last range failed.
    static float getLatestDistance();
    static uint32_t getLatestRTT();
    static int getLatestStatus();
    static bool isNewDataAvailable();
    static void clearDataFlag();

private:
    // Shared data storage for the WiFi event callback
    static float latestDistance;
    static uint32_t latestRtt;
    static int latestStatus;
    static bool newDataAvailable;

    // The hardware event callback must be static
    static void onFtmReport(arduino_event_id_t event, arduino_event_info_t info);
};

