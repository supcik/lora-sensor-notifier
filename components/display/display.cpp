#include "display.hpp"

#include "Arduino.h"
#include "IBMPlexSansBold9pt8b.h"
#include "IBMPlexSansMedium9pt8b.h"
#include "Wire.h"

#define SSD1306_NO_SPLASH 1

static constexpr int kSDA          = 4;
static constexpr int kSCL          = 15;
static constexpr int kRST          = 16;
static constexpr int kScreenWidth  = 128;  // OLED display width, in pixels
static constexpr int kScreenHeight = 64;   // OLED display height, in pixels

Display::Display() {}

void Display::Init() {
    pinMode(kRST, OUTPUT);
    digitalWrite(kRST, LOW);
    delay(20);
    digitalWrite(kRST, HIGH);
    Wire.begin(kSDA, kSCL);
    lcd_         = new Adafruit_SSD1306(kScreenWidth, kScreenHeight, &Wire, kRST);
    bool started = lcd_->begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false);
    assert(started);
    (void)started;
}

Display* Display::Instance() {
    static Display* instance = new Display();
    return instance;
}

Adafruit_SSD1306* Display::Lcd() { return lcd_; }

void Display::SplashScreen() {
    int16_t x, y;
    uint16_t w, h;
    char title[] = "LoRa Notifier";
    lcd_->setTextColor(SSD1306_WHITE);
    lcd_->clearDisplay();
    lcd_->setFont(&IBMPlexSansBold9pt8b);
    lcd_->getTextBounds(title, 0, 0, &x, &y, &w, &h);
    lcd_->setCursor((kScreenWidth - w) / 2, 25);
    lcd_->println(title);

    lcd_->setFont(nullptr);
    lcd_->setTextSize(1);

    char line0[] = "Copyright (c) 2023";
    lcd_->getTextBounds(line0, 0, 0, &x, &y, &w, &h);
    lcd_->setCursor((kScreenWidth - w) / 2, 35);
    lcd_->println(line0);

    char line1[] = "Jacques Supcik";
    lcd_->getTextBounds(line1, 0, 0, &x, &y, &w, &h);
    lcd_->setCursor((kScreenWidth - w) / 2, 47);
    lcd_->println(line1);

    lcd_->display();  // actually display all of the above})
}

void Display::SetStatus(const char* status) {
    strncpy(status_, status, sizeof(status_));
    status_[sizeof(status_) - 1] = '\0';
    Update();
}

void Display::SetMessage(const char* message) {
    strncpy(message_, message, sizeof(message_));
    message_[sizeof(message_) - 1] = '\0';
    Update();
}

void Display::Update() {
    int16_t x, y;
    uint16_t w, h;
    lcd_->setTextColor(SSD1306_WHITE);
    lcd_->clearDisplay();
    lcd_->setFont(&IBMPlexSansMedium9pt8b);
    lcd_->getTextBounds(status_, 0, 0, &x, &y, &w, &h);
    lcd_->setCursor((kScreenWidth - w) / 2, 30);
    lcd_->println(status_);

    lcd_->setFont(nullptr);
    lcd_->setTextSize(1);
    lcd_->getTextBounds(message_, 0, 0, &x, &y, &w, &h);
    lcd_->setCursor((kScreenWidth - w) / 2, 50);
    lcd_->println(message_);

    lcd_->display();  // actually display all of the above})
}