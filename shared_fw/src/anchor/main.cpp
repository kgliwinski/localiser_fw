#if defined(IS_ANCHOR)

#include <Arduino.h>
#include "LpsFtmInitiator.h"

LpsFtmInitiator ftm;
// Replace with the MAC printed by your Target
uint8_t targetMac[6] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F}; 

void setup() {
    Serial.begin(115200);
    ftm.begin();
}

void loop() {
    Serial.println("Requesting Range...");
    ftm.requestRanging(targetMac, 1);
    
    delay(100); // Give the hardware time to complete the burst
    
    if (const auto distance = ftm.latestDistance()) {
        Serial.printf("Distance: %.2f m, RTT: %u ns\n", 
                      *distance,
                      ftm.latestRTT());
        ftm.clearDataFlag();
    } else {
        Serial.printf("FTM Failed. Status: %d\n", ftm.latestStatus());
    }
    
    delay(1000);
}

#endif