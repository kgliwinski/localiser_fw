#if defined(IS_ANCHOR)

#include "LpsFtmInitiator.h"
#include <cstdio>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {
const uint8_t targetMac[6] = {0x68, 0x9D, 0xD2, 0xC2, 0x72, 0x7D};
}

extern "C" void app_main() {
    LpsFtmInitiator ftm(targetMac, "LPS_TARGET_1");
    if (!ftm.begin()) {
        printf("FTM initiator startup failed.\n");
        return;
    }
    printf("Anchor STA MAC Address: %s\n", ftm.macAddress().c_str());

    while (true) {
        printf("Requesting Range...\n");
        ftm.requestRanging();
        if (const auto distance = ftm.latestDistance()) {
            printf("Distance: %.2f m, RTT: %" PRIu32 " ns\n", *distance, ftm.latestRTT());
            ftm.clearDataFlag();
        } else {
            printf("FTM Failed. Status: %d\n", ftm.latestStatus());
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif
