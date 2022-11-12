#ifndef COMPONENTS_DISPLAY_INCLUDE_DISPLAY_HPP_
#define COMPONENTS_DISPLAY_INCLUDE_DISPLAY_HPP_

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display {
   public:
    static Display* Instance();
    Adafruit_SSD1306* Lcd();
    void Message(const char* title, const char* message);
    bool Started();

   private:
    Display();
    Display(Display const&);
    void operator=(Display const&);
    Adafruit_SSD1306* lcd_;
    bool started_;
};

#endif /* COMPONENTS_DISPLAY_INCLUDE_DISPLAY_HPP_ */
