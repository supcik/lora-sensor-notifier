#include "display.hpp"

#include "Arduino.h"
#include "Wire.h"

#define SSD1306_NO_SPLASH 1

static constexpr int kSDA          = 4;
static constexpr int kSCL          = 15;
static constexpr int kRST          = 16;
static constexpr int kScreenWidth  = 128;  // OLED display width, in pixels
static constexpr int kScreenHeight = 64;   // OLED display height, in pixels

Display::Display() {
    pinMode(kRST, OUTPUT);
    digitalWrite(kRST, LOW);
    delay(20);
    digitalWrite(kRST, HIGH);
    Wire.begin(kSDA, kSCL);
    lcd_     = new Adafruit_SSD1306(kScreenWidth, kScreenHeight, &Wire, kRST);
    started_ = lcd_->begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false);
}

Display* Display::Instance() {
    static Display* instance = new Display();
    return instance;
}

bool Display::Started() { return started_; }

Adafruit_SSD1306* Display::Lcd() { return lcd_; }

void Display::Message(const char* title, const char* message) {
    int16_t x, y;
    uint16_t w, h;
    lcd_->setTextColor(SSD1306_WHITE);
    lcd_->clearDisplay();
    lcd_->setTextSize(2);
    lcd_->getTextBounds(title, 0, 0, &x, &y, &w, &h);
    lcd_->setCursor((kScreenWidth - w) / 2, 5);
    lcd_->println(title);

    lcd_->setTextSize(1);
    lcd_->setCursor(0, 30);
    lcd_->println(message);

    lcd_->display();  // actually display all of the above})
}