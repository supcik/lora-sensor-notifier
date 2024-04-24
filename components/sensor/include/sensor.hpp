#include <Arduino.h>

class Sensor {
   public:
    enum StateT { kOff, kOn };

    Sensor();

    void Init(int Pin, int active = LOW, int debounce = 10);
    StateT State();
    int Value();
    void Read(timeval* now);
    long IdleSeconds(timeval* now);

   private:
    StateT state_;
    long lastActivity_;
    int pin_;
    int active_;
    int debounce_;
    int countdown_;
};