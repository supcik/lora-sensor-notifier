/**
 ******************************************************************************
 * @brief       : LoRa Notifier
 * @author      : Jacques Supcik <jacques@supcik.net>
 * @date        : 20 July 2023
 ******************************************************************************
 * @copyright   : Copyright (c) 2023 Jacques Supcik
 * @attention   : SPDX-License-Identifier: MIT OR Apache-2.0
 ******************************************************************************
 * @details
 * LoRa Notifier
 ******************************************************************************
 */

#pragma once
#include <Arduino.h>
#include <esp_event.h>
#include <freertos/ringbuf.h>
#include <lmic.h>
#include <time.h>

#include <CayenneLPP.hpp>
#include <cstdint>

class Notifier {
   public:
    static Notifier* Instance();
    void Init();
    void SendPacket(CayenneLPP* lpp);
    RingbufHandle_t bufHandle_;

   private:
    Notifier();
    Notifier(Notifier const&);
    void operator=(Notifier const&);

    osjob_t sendjob_;
};
