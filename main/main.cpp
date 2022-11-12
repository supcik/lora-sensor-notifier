/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
static const char* TAG = "ROOT";

#include <stdio.h>

#include "Arduino.h"
#include "arduino_lmic_hal_boards.h"
#include "assert.h"
#include "display.hpp"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/hal.h"
#include "lmic.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

extern "C" void app_main(void);

static u1_t appEUI[8];
static u1_t devEUI[8];
static u1_t appKey[16];

void os_getArtEui(u1_t* buf) {
    ESP_LOGD(TAG, "Asking for ArtEui");
    memcpy(buf, appEUI, 8);
}
void os_getDevEui(u1_t* buf) {
    ESP_LOGD(TAG, "Asking for DevEui");
    memcpy(buf, devEUI, 8);
}
void os_getDevKey(u1_t* buf) {
    ESP_LOGD(TAG, "Asking for DevKey");
    memcpy(buf, appKey, 16);
}

static uint8_t mydata[] = {0x43, 0x44, 0x00};
static osjob_t sendjob;

// Pin mapping
const lmic_pinmap* pPinMap = Arduino_LMIC::GetPinmap_ThisBoard();

// const lmic_pinmap lmic_pins = {0};
// lmic_pins.nss               = 18;
// lmic_pins.rxtx              = LMIC_UNUSED_PIN;
// lmic_pins.rst               = 14;
// lmic_pins.dio               = {26, 33, 32};

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

void do_send(osjob_t* j);

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

void setSecrets() {
    char buffer[33];
    esp_err_t err = nvs_flash_init();
    ESP_ERROR_CHECK(err);

    ESP_LOGD(TAG, "Opening Non-Volatile Storage (NVS) handle");
    nvs_handle_t nvsHandle;
    err = nvs_open("storage", NVS_READONLY, &nvsHandle);
    ESP_ERROR_CHECK(err);

    size_t size = sizeof(appEUI);
    err         = nvs_get_blob(nvsHandle, "appeui", appEUI, &size);
    ESP_ERROR_CHECK(err);
    assert(size == sizeof(appEUI));
    HexDump(appEUI, sizeof(appEUI), buffer, sizeof(buffer));
    ESP_LOGD(TAG, "appeui: %s", buffer);

    size = sizeof(devEUI);
    err  = nvs_get_blob(nvsHandle, "deveui", devEUI, &size);
    ESP_ERROR_CHECK(err);
    assert(size == sizeof(devEUI));
    HexDump(devEUI, sizeof(devEUI), buffer, sizeof(buffer));
    ESP_LOGD(TAG, "deveui: %s", buffer);

    size = sizeof(appKey);
    err  = nvs_get_blob(nvsHandle, "appkey", appKey, &size);
    ESP_ERROR_CHECK(err);
    assert(size == sizeof(appKey));
    HexDump(appKey, sizeof(appKey), buffer, sizeof(buffer));
    ESP_LOGD(TAG, "appkey: %s", buffer);
}

void onEvent(ev_t ev) {
    Display* lcd = Display::Instance();
    switch (ev) {
        case EV_SCAN_TIMEOUT:
            lcd->Message("SCAN_TIMEOUT", "");
            ESP_LOGI(TAG, "EV_SCAN_TIMEOUT");
            break;
        case EV_BEACON_FOUND:
            ESP_LOGI(TAG, "EV_BEACON_FOUND");
            break;
        case EV_BEACON_MISSED:
            ESP_LOGI(TAG, "EV_BEACON_MISSED");
            break;
        case EV_BEACON_TRACKED:
            ESP_LOGI(TAG, "EV_BEACON_TRACKED");
            break;
        case EV_JOINING:
            lcd->Message("JOINING", "");
            ESP_LOGI(TAG, "EV_JOINING");
            break;
        case EV_JOINED:
            lcd->Message("JOINED", "");
            ESP_LOGI(TAG, "EV_JOINED");
            {
                u4_t netid        = 0;
                devaddr_t devaddr = 0;
                u1_t nwkKey[16];
                u1_t artKey[16];

                char buffer[33];
                LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
                ESP_LOGI(TAG, "netid: %d", netid);
                ESP_LOGI(TAG, "devaddr: %08X", devaddr);
                HexDump(artKey, sizeof(artKey), buffer, sizeof(buffer));
                ESP_LOGI(TAG, "AppSKey: %s", buffer);
                HexDump(nwkKey, sizeof(nwkKey), buffer, sizeof(buffer));
                ESP_LOGI(TAG, "NwkSKey: %s", buffer);
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     ESP_LOGI(TAG, "EV_RFU1");
        ||     break;
        */
        case EV_JOIN_FAILED:
            ESP_LOGW(TAG, "EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            ESP_LOGW(TAG, "EV_REJOIN_FAILED");
            break;
        case EV_TXCOMPLETE:
            lcd->Message("TXCOMPLETE", "");
            ESP_LOGI(TAG, "EV_TXCOMPLETE (includes waiting for RX windows)");
            if (LMIC.txrxFlags & TXRX_ACK) {
                ESP_LOGI(TAG, "Received ack");
            }
            if (LMIC.dataLen) {
                ESP_LOGI(TAG, "Received %d bytes of payload", LMIC.dataLen);
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            ESP_LOGW(TAG, "EV_LOST_TSYNC");
            break;
        case EV_RESET:
            ESP_LOGW(TAG, "EV_RESET");
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            ESP_LOGI(TAG, "EV_RXCOMPLETE");
            break;
        case EV_LINK_DEAD:
            ESP_LOGW(TAG, "EV_LINK_DEAD");
            break;
        case EV_LINK_ALIVE:
            ESP_LOGI(TAG, "EV_LINK_ALIVE");
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    ESP_LOGI(TAG, "EV_SCAN_FOUND");
        ||    break;
        */
        case EV_TXSTART:
            lcd->Message("TXSTART", "");
            ESP_LOGI(TAG, "EV_TXSTART");
            break;
        case EV_TXCANCELED:
            ESP_LOGW(TAG, "EV_TXCANCELED");
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            ESP_LOGI(TAG, "EV_JOIN_TXCOMPLETE: no JoinAccept");
            break;

        default:
            ESP_LOGW(TAG, "Unknown event: %u", ev);
            break;
    }
}

void do_send(osjob_t* j) {
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        ESP_LOGI(TAG, "OP_TXRXPEND, not sending");
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
        ESP_LOGI(TAG, "Packet queued");
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void app_main(void) {
    initArduino();
    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }
    esp_log_level_set("*", ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "Starting");
    // delay(1000);
    Display* lcd = Display::Instance();
    setSecrets();
    lcd->Message("READY", "  Copyright (c) 2022\n    Jacques Supcik");
    delay(2000);

    ESP_LOGI(TAG, "Initializing OS");
    // os_init();
    os_init_ex(pPinMap);
    ESP_LOGI(TAG, "Reset LMIC");
    LMIC_reset();
    ESP_LOGI(TAG, "SENDING");
    do_send(&sendjob);

    while (1) {
        os_runloop_once();
        delay(1);
    }
}