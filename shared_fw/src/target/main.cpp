#if defined(IS_TARGET)

#include "LpsFtmResponder.h"
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" void app_main() {
    LpsFtmResponder ftm("LPS_TARGET_1");
    if (!ftm.begin()) {
        printf("FTM responder startup failed.\n");
        return;
    }
    printf("Target SoftAP MAC Address: %s\n", ftm.macAddress().c_str());

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif
