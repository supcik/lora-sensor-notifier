#pragma once

#include <esp_event.h>

ESP_EVENT_DECLARE_BASE(LORAWAN_EVENT_BASE);

// Use this function to get the event loop handler for the LoRaWan component.
// This is needed to register event handlers for the LoRaWan component.
// For example, a handler could print status messages on the display
esp_event_loop_handle_t LoRaWanEventLoopHandler();

void LoRaWanInit();
void LoRaWanDeInit();
