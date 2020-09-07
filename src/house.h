#ifndef __HOUSE_H__
#define __HOUSE_H__

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define MAX_HOUSES      12
#define BANK_1_PIN          D6
#define BANK_2_PIN          D3
#define BANK_3_PIN          D4
#define BANK_4_PIN          D5

class Houses {
public:
    Houses(int, int);
    ~Houses();

    void turnOn();
    void turnOn(int);
    void turnOff();
    void turnOff(int);
    int numPixels() { return m_leds; }

private:
    CRGB m_houses[MAX_HOUSES];
    int m_leds;
};

#endif