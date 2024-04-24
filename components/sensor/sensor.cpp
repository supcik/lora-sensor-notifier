#include "sensor.hpp"

#include <Arduino.h>

Sensor::Sensor() { lastActivity_ = -3600 * 24; }

void Sensor::Init(int pin, int active, int debounce) {
    pin_       = pin;
    active_    = active;
    debounce_  = debounce;
    countdown_ = debounce;
    pinMode(pin_, INPUT_PULLUP);

    int value = digitalRead(pin_);
    state_    = value == active_ ? kOn : kOff;
}

Sensor::StateT Sensor::State() { return state_; }

int Sensor::Value() { return state_ == kOn ? 1 : 0; }

long Sensor::IdleSeconds(timeval* now) {
    if (state_ == kOff) {
        return now->tv_sec - lastActivity_;
    } else {
        return 0;
    }
}

void Sensor::Read(timeval* now) {
    int value = digitalRead(pin_);
    switch (state_) {
        case kOff:
            if (value == active_) {
                if (countdown_ > 0) {
                    countdown_--;
                } else {
                    state_        = kOn;
                    lastActivity_ = now->tv_sec;
                    countdown_    = debounce_;
                }
            } else {
                countdown_ = debounce_;
            }
            break;
        case kOn:
            if (value != active_) {
                if (countdown_ > 0) {
                    countdown_--;
                    lastActivity_ = now->tv_sec;
                } else {
                    state_     = kOff;
                    countdown_ = debounce_;
                }
            } else {
                countdown_    = debounce_;
                lastActivity_ = now->tv_sec;
            }
            break;
        default:
            break;
    }
}