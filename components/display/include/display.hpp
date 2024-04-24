#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display {
   public:
    static Display* Instance();
    Adafruit_SSD1306* Lcd();
    void Init();
    void SplashScreen();
    void SetStatus(const char* status);
    void SetMessage(const char* status);
    void Update();

   private:
    Display();
    Display(Display const&);
    void operator=(Display const&);
    Adafruit_SSD1306* lcd_;
    char status_[32]  = {0};
    char message_[64] = {0};
};
