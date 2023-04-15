#pragma once

#include <stdint.h>

extern "C" {
    void IoOut32(uint16_t addr, uint32_t date);
    uint32_t IoIn32(uint16_t addr);
}
