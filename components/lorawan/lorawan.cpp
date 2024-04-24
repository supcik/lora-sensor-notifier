#include "lorawan.hpp"

#include <Arduino.h>
#include <arduino_lmic_hal_boards.h>
#include <esp_event.h>
#include <freertos/task.h>
#include <hal/hal.h>
#include <lmic.h>
#include <nvs_flash.h>

#include <util.hpp>

static const char* TAG = "LORAWAN";
ESP_EVENT_DEFINE_BASE(LORAWAN_EVENT_BASE);

static const int32_t kQueueSize = 16;

static esp_event_loop_handle_t lorawan_event_loop_handler;

// ----- Functions for LMIC-Arduino -----

void os_getArtEui(u1_t* buf) {
    ESP_LOGD(TAG, "Asking for ArtEui");
    const int buffer_size = 8;
    u1_t local_buffer[buffer_size];
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &handle));
    size_t size = buffer_size;
    ESP_ERROR_CHECK(nvs_get_blob(handle, "appeui", (void*)local_buffer, &size));
    assert(size == buffer_size);
    nvs_close(handle);
    for (int i = 0; i < buffer_size; i++) {
        buf[i] = local_buffer[buffer_size - 1 - i];  // Reverse the order
    }
}

void os_getDevEui(u1_t* buf) {
    ESP_LOGD(TAG, "Asking for DevEui");
    const int buffer_size = 8;
    u1_t local_buffer[buffer_size];
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &handle));
    size_t size = buffer_size;
    ESP_ERROR_CHECK(nvs_get_blob(handle, "deveui", (void*)local_buffer, &size));
    assert(size == buffer_size);
    nvs_close(handle);
    for (int i = 0; i < buffer_size; i++) {
        buf[i] = local_buffer[buffer_size - 1 - i];  // Reverse the order
    }
}

void os_getDevKey(u1_t* buf) {
    ESP_LOGD(TAG, "Asking for DevKey");
    const int buffer_size = 16;
    u1_t local_buffer[buffer_size];
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &handle));
    size_t size = buffer_size;
    ESP_ERROR_CHECK(nvs_get_blob(handle, "appkey", (void*)local_buffer, &size));
    assert(size == buffer_size);
    nvs_close(handle);
    for (int i = 0; i < buffer_size; i++) {
        buf[i] = local_buffer[i];
    }
}

void onEvent(ev_t ev) {
    ESP_LOGD(TAG, "Posting event %d", ev);
    esp_event_post_to(lorawan_event_loop_handler, LORAWAN_EVENT_BASE, ev, NULL, 0, portMAX_DELAY);
}

// ----- Private functions -----

static void LoRaWanSystemHandler(void* event_handler_arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void* event_data) {
    switch (event_id) {
        case EV_JOINED:
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
    }
}

static void LoRaWanLogHandler(void* event_handler_arg,
                              esp_event_base_t event_base,
                              int32_t event_id,
                              void* event_data) {
    switch (event_id) {
        case EV_SCAN_TIMEOUT:
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
            ESP_LOGI(TAG, "EV_JOINING");
            break;
        case EV_JOINED:
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
            ESP_LOGI(TAG, "EV_TXCOMPLETE (includes waiting for RX windows)");
            if (LMIC.txrxFlags & TXRX_ACK) {
                ESP_LOGI(TAG, "Received ack");
            }
            if (LMIC.dataLen) {
                ESP_LOGI(TAG, "Received %d bytes of payload", LMIC.dataLen);
            }
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
            ESP_LOGW(TAG, "Unknown event: %u", event_id);
            break;
    }
}

// ----- Public functions -----

esp_event_loop_handle_t LoRaWanEventLoopHandler() { return lorawan_event_loop_handler; }

void LoRaWanInit() {
    ESP_LOGI(TAG, "Initializing OS");
    ESP_ERROR_CHECK(nvs_flash_init());
    os_init_ex(Arduino_LMIC::GetPinmap_ThisBoard());
    ESP_LOGI(TAG, "Reset LMIC");
    LMIC_reset();

    esp_event_loop_args_t loop_args = {.queue_size      = kQueueSize,
                                       .task_name       = "lw_ev_loop",
                                       .task_priority   = 1,
                                       .task_stack_size = 2048,
                                       .task_core_id    = APP_CPU_NUM};

    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &lorawan_event_loop_handler));

    ESP_ERROR_CHECK(esp_event_handler_register_with(lorawan_event_loop_handler,
                                                    LORAWAN_EVENT_BASE,
                                                    ESP_EVENT_ANY_ID,
                                                    LoRaWanSystemHandler,
                                                    nullptr));

    ESP_ERROR_CHECK(esp_event_handler_register_with(lorawan_event_loop_handler,
                                                    LORAWAN_EVENT_BASE,
                                                    ESP_EVENT_ANY_ID,
                                                    LoRaWanLogHandler,
                                                    nullptr));
}

void LoRaWanDeInit() {}