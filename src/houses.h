#pragma once
#include <Particle.h>

class Houses {
public:
    Houses(int pin, int count)
    {
        m_pin = pin;
        m_leds = count;
    }
    ~Houses();

    static constexpr int MAX_HOUSES = 25;
    static constexpr int BANK_1_PIN = D6;
    static constexpr int BANK_2_PIN = D3;
    static constexpr int BANK_3_PIN = D4;
    static constexpr int BANK_4_PIN = D5;

    int numPixels() { return m_leds; }
    int numHouses() { return m_houseCount; }
    int bank() { return m_bank; }

    virtual void turnOn() = 0;
    virtual bool turnOn(int) = 0;
    virtual void turnOff() = 0;
    virtual bool turnOff(int) = 0;
    virtual bool isOn(int) = 0;
    virtual void setColors(uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void setColors(uint8_t r, uint8_t g, uint8_t b, uint8_t w) = 0;
    virtual void setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b, uint8_t w) = 0;
    virtual bool isRGBWCapable() = 0;

protected:
    int m_leds;
    int m_houseCount;
    int m_bank;
    int m_pin;
};
