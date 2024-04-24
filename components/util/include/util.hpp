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

#pragma once

#include "cstdint"
#include "cstring"

void HexDump(const uint8_t* data, size_t size, char* buffer, size_t buffer_size);
