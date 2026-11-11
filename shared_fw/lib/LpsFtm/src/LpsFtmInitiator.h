#pragma once

#include "LpsFtm.h"
#include <esp_event.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <optional>
#include <string>

class LpsFtmInitiator : public LpsFtm {
public:
    explicit LpsFtmInitiator(
        const uint8_t* targetMac,
        const char* targetSsid,
        const char* targetPassword = nullptr,
        uint8_t frameCount = 16,
        uint16_t burstPeriod = 2,
        uint8_t targetChannel = 1
    );

    bool begin() override;
    bool requestRanging();
    std::string macAddress() const;

    static std::optional<float> latestDistance();
    static uint32_t latestRTT();
    static int latestStatus();
    static bool isNewDataAvailable();
    static void clearDataFlag();

private:
    static void onWifiEvent(
        void* context,
        esp_event_base_t eventBase,
        int32_t eventId,
        void* eventData
    );
    static void onFtmReport(
        void* context,
        esp_event_base_t eventBase,
        int32_t eventId,
        void* eventData
    );

    std::string targetPassword;
    std::string targetSsid;
    bool hasPassword;
    uint8_t frameCount;
    uint16_t burstPeriod;
    uint8_t targetMac[6];
    uint8_t targetChannel;
    esp_event_handler_instance_t ftmEventHandler = nullptr;
    SemaphoreHandle_t connectedSemaphore = nullptr;

    static std::optional<float> latestDistanceValue;
    static uint32_t latestRtt;
    static int latestStatusValue;
    static bool newDataAvailable;
    static SemaphoreHandle_t ftmReportSemaphore;
    static uint8_t ftmReportEntryCount;
};
