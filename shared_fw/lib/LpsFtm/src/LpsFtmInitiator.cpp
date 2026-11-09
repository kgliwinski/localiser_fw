#include "LpsFtmInitiator.h"

std::optional<float> LpsFtmInitiator::latestDistanceValue = std::nullopt;
uint32_t LpsFtmInitiator::latestRtt = 0;
int LpsFtmInitiator::latestStatusValue = -1;
bool LpsFtmInitiator::newDataAvailable = false;

LpsFtmInitiator::LpsFtmInitiator(uint8_t frameCount, uint16_t burstPeriod)
    : frameCount(frameCount), burstPeriod(burstPeriod) {}

bool LpsFtmInitiator::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.onEvent(onFtmReport, ARDUINO_EVENT_WIFI_FTM_REPORT);
    Serial.println("FTM Initiator Initialized. Ready to range.");
    return true;
}

bool LpsFtmInitiator::requestRanging(const uint8_t* targetMac, uint8_t channel) {
    return WiFi.initiateFTM(frameCount, burstPeriod, channel, targetMac);
}

void LpsFtmInitiator::onFtmReport(
    arduino_event_id_t event,
    arduino_event_info_t info
) {
    if (event != ARDUINO_EVENT_WIFI_FTM_REPORT) {
        return;
    }

    wifi_event_ftm_report_t* report = &info.wifi_ftm_report;
    latestStatusValue = report->status;

    if (report->status == FTM_STATUS_SUCCESS) {
        latestDistanceValue = static_cast<float>(report->dist_est) / 100.0F;
        latestRtt = report->rtt_est;
        newDataAvailable = true;
    } else {
        latestDistanceValue = std::nullopt;
        newDataAvailable = false;
    }

    free(report->ftm_report_data);
}

std::optional<float> LpsFtmInitiator::latestDistance() {
    return latestDistanceValue;
}

uint32_t LpsFtmInitiator::latestRTT() {
    return latestRtt;
}

int LpsFtmInitiator::latestStatus() {
    return latestStatusValue;
}

bool LpsFtmInitiator::isNewDataAvailable() {
    return newDataAvailable;
}

void LpsFtmInitiator::clearDataFlag() {
    newDataAvailable = false;
}
