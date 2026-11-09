#pragma once

#include "LpsFtm.h"
#include <Arduino.h>
#include <WiFi.h>
#include <optional>

class LpsFtmInitiator : public LpsFtm {
public:
    explicit LpsFtmInitiator(
        uint8_t frameCount = 16,
        uint16_t burstPeriod = 2
    );

    bool begin() override;
    bool requestRanging(const uint8_t* targetMac, uint8_t channel = 1);

    static std::optional<float> latestDistance();
    static uint32_t latestRTT();
    static int latestStatus();
    static bool isNewDataAvailable();
    static void clearDataFlag();

private:
    static void onFtmReport(arduino_event_id_t event, arduino_event_info_t info);

    uint8_t frameCount;
    uint16_t burstPeriod;

    static std::optional<float> latestDistanceValue;
    static uint32_t latestRtt;
    static int latestStatusValue;
    static bool newDataAvailable;
};
