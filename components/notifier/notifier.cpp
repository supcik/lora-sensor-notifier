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

#include "notifier.hpp"

#include <Arduino.h>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/ringbuf.h>
#include <lmic.h>
#include <time.h>

#include <lorawan.hpp>
#include <util.hpp>

static const char* TAG = "Notifier";

Notifier::Notifier() {}

Notifier* Notifier::Instance() {
    static Notifier* instance = new Notifier();
    return instance;
}

void Notifier::Init() { bufHandle_ = xRingbufferCreate(32, RINGBUF_TYPE_NOSPLIT); }

void Notifier::SendPacket(CayenneLPP* lpp) {
    ESP_LOGD(TAG, "Sending packet to buffer");

    BaseType_t res = xRingbufferSend(bufHandle_, lpp->getBuffer(), lpp->getSize(), portMAX_DELAY);
    assert(res == pdTRUE);
    (void)res;

    os_setCallback(&sendjob_, [](osjob_t* j) {
        // This callback reads the message from the ring buffer and sends it to
        // the LoRaWAN.
        ESP_LOGD(TAG, "Receiving packet from buffer");
        size_t item_size = 0;
        u1_t* payload =
            (u1_t*)xRingbufferReceive(Notifier::Instance()->bufHandle_, &item_size, portMAX_DELAY);
        if (payload == NULL) {
            abort();
        }
        LMIC_setTxData2(1, payload, item_size, 0);
        ESP_LOGD(TAG, "Packet queued");
        vRingbufferReturnItem(Notifier::Instance()->bufHandle_,
                              (void*)payload);  // Don't forget to return the item!
    });
}
