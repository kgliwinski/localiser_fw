#if defined(IS_TARGET)
#include <Arduino.h>
#include "LpsFtmResponder.h"

LpsFtmResponder ftm("LPS_TARGET_1");

void setup() {
    Serial.begin(115200);
    ftm.begin(); // No password needed
}

void loop() {
    // The WiFi task handles FTM responses automatically in the background.
    delay(1000); 
}

#endif