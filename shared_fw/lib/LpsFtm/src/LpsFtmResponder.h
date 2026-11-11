#pragma once

#include "LpsFtm.h"
#include <string>

class LpsFtmResponder : public LpsFtm {
public:
    explicit LpsFtmResponder(const char* ssid, const char* password = nullptr);

    bool begin() override;
    const char* ssid() const;
    std::string macAddress() const;

private:
    std::string networkName;
    std::string networkPassword;
    bool hasPassword;
};
