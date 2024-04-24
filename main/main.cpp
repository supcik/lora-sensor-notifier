/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

static const char* TAG = "ROOT";

#include <Arduino.h>
#include <assert.h>
#include <esp_log.h>
#include <esp_spi_flash.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/hal.h>
#include <lmic.h>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>

#include "CayenneLPP.hpp"
#include "display.hpp"
#include "lorawan.hpp"
#include "notifier.hpp"
#include "sensor.hpp"
#include "util.hpp"

const long kRebootTime = 60 * 60 * 24 * 365 * 1;  // 1 year

const long kSplashTime          = 10;               // 10 seconds
const long kFirstMessageTimeout = kSplashTime + 5;  // 5 second after splash screen
const long kDefaultPeriod       = 5 * 60;           // 5 minutes
const long kAlarmPeriod         = 150;              // 2.5 minutes
const long kIdlePeriod          = 60 * 60;          // 1 hour

const int kTriggerPin = 25;
const int kTestPin    = 0;

void LoRaWanDisplayHandler(void* event_handler_arg,
                           esp_event_base_t event_base,
                           int32_t event_id,
                           void* event_data) {
    Display* lcd = Display::Instance();
    switch (event_id) {
        case EV_JOINING:
            lcd->SetMessage("JOINING");
            break;
        case EV_JOINED:
            lcd->SetMessage("JOINED");
            break;
        case EV_TXCOMPLETE:
            lcd->SetMessage("TXCOMPLETE");
            break;
        case EV_TXSTART:
            lcd->SetMessage("TXSTART");
            break;
    }
}

static Sensor waterSensor;
static Sensor testSensor;
static Notifier* notifier = Notifier::Instance();
static Display* lcd       = Display::Instance();

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;  // wait for serial port to connect
    }
    ESP_LOGI(TAG, "Starting");
    ESP_LOGI(TAG, "Displaying Splash Screen");
    lcd->Init();
    lcd->SplashScreen();
    vTaskDelay(kSplashTime * 1000);

    lcd->SetStatus("HELLO");
    lcd->SetMessage("STARTING");
    LoRaWanInit();

    esp_event_handler_register_with(LoRaWanEventLoopHandler(),
                                    LORAWAN_EVENT_BASE,
                                    ESP_EVENT_ANY_ID,
                                    LoRaWanDisplayHandler,
                                    nullptr);

    waterSensor.Init(kTriggerPin);
    testSensor.Init(kTestPin);

    notifier->Init();
}

void loop() {
    static CayenneLPP payload(32);
    static long lastSend        = 0;
    static int nextPayload[2]   = {1, 1};
    static long delayToNextSend = kFirstMessageTimeout;

    os_runloop_once();
    timeval now;

    gettimeofday(&now, NULL);
    if (now.tv_sec > kRebootTime) {
        ESP_LOGW(TAG, "Restarting now");
        fflush(stdout);
        esp_restart();
    }

    bool alarmIdle = waterSensor.IdleSeconds(&now) > kIdlePeriod;
    bool testIdle  = testSensor.IdleSeconds(&now) > kIdlePeriod;

    waterSensor.Read(&now);
    testSensor.Read(&now);

    if (waterSensor.State() == Sensor::kOn) {
        nextPayload[0] = 0;
        if (alarmIdle && delayToNextSend > 0) {
            ESP_LOGI(TAG, "Alarm : Sending immediately");
            delayToNextSend = 0;
        } else if (delayToNextSend > kAlarmPeriod) {
            lcd->SetStatus("ALARM...");
            ESP_LOGI(TAG, "Alarm : Reducing delay to next send");
            delayToNextSend = kAlarmPeriod;
        }
    }
    if (testSensor.State() == Sensor::kOn) {
        nextPayload[1] = 0;
        if (testIdle && delayToNextSend > 0) {
            ESP_LOGI(TAG, "Test : Sending immediately");
            delayToNextSend = 0;
        } else if (delayToNextSend > kAlarmPeriod) {
            lcd->SetStatus("TEST...");
            ESP_LOGI(TAG, "Test : Reducing delay to next send");
            delayToNextSend = kAlarmPeriod;
        }
    }

    if (now.tv_sec - lastSend >= delayToNextSend) {
        ESP_LOGI(TAG, "Sending packet");
        if (nextPayload[0] == 0) {
            lcd->SetStatus("ALARM");
        } else if (nextPayload[1] == 0) {
            lcd->SetStatus("TEST");
        } else {
            lcd->SetStatus("OK");
        }

        payload.reset();
        payload.addDigitalInput(0, nextPayload[0]);
        payload.addDigitalInput(1, nextPayload[1]);
        notifier->SendPacket(&payload);
        lastSend        = now.tv_sec;
        nextPayload[0]  = 1;
        nextPayload[1]  = 1;
        delayToNextSend = kDefaultPeriod;
    }
    vTaskDelay(1);
}
