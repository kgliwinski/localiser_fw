#pragma once

#include "LpsFtm.h"
#include <Arduino.h>

class LpsFtmResponder : public LpsFtm {
public:
    explicit LpsFtmResponder(const char* ssid, const char* password = nullptr);

    bool begin() override;
    const char* ssid() const;

private:
    String networkName;
    String networkPassword;
    bool hasPassword;
};
