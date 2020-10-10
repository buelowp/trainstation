#ifndef __HOUSES_H__
#define __HOUSES_H__

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define MAX_HOUSES          10      // this must be even, each house has 2 leds
#define BANK_1_PIN          D6
#define BANK_2_PIN          D3
#define BANK_3_PIN          D4
#define BANK_4_PIN          D5

class Houses {
public:
    Houses(int, int);
    ~Houses();

    void turnOn();
    bool turnOn(int);
    void turnOff();
    bool turnOff(int);
    bool turnOffNextHouse();
    int numPixels() { return m_leds; }
    int numHouses() { return m_houseCount; }
    int bank() { return m_bank; }
    bool isOn(int);
    void setColors(uint8_t r, uint8_t g, uint8_t b);

private:
    CRGB m_warmWhite;
    CRGB m_houses[MAX_HOUSES * 2];
    int m_leds;
    int m_houseCount;
    int m_bank;
};

#endif