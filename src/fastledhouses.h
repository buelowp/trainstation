#pragma once

#include <Particle.h>
#include <FastLED.h>
#include "houses.h"

FASTLED_USING_NAMESPACE

class FastLEDHouses : public Houses {
public:
    FastLEDHouses(int, int);
    ~FastLEDHouses();

    void turnOn() override;
    bool turnOn(int) override;
    bool turnOn(int, uint8_t) override;
    void turnOff() override;
    bool turnOff(int) override;
    bool isOn(int) override;
    void setColors(uint8_t r, uint8_t g, uint8_t b) override;
    void setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b) override;
    void setColors(uint8_t r, uint8_t g, uint8_t b, uint8_t w) override;
    void setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b, uint8_t w) override;
    bool isRGBWCapable() override { return false; }

private:
    CRGB m_warmWhite;
    CRGB m_houses[MAX_HOUSES * 2];
};
