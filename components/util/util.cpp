/**
 ******************************************************************************
 * @brief       : Toolbox for the main program
 * @author      : Jacques Supcik <jacques@supcik.net>
 * @date        : 13 November 2022
 ******************************************************************************
 * @copyright   : Copyright (c) 2022 Jacques Supcik
 * @attention   : SPDX-License-Identifier: MIT OR Apache-2.0
 ******************************************************************************
 * @details
 * Toolbox for the main program
 ******************************************************************************
 */

#include "util.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>

void HexDump(const uint8_t* data, size_t size, char* buffer, size_t buffer_size) {
    if (buffer_size < size * 2 + 1) {
        return;
    }
    for (size_t i = 0; i < size; i++) {
        sprintf(buffer, "%02x", data[i]);
        buffer += 2;
    }
    *buffer = 0;
}