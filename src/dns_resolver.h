#pragma once
#include <Arduino.h>
#include <WiFi.h>

namespace dns_resolver {
    bool resolve(const char* host, IPAddress& out);
}

