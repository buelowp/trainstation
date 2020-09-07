#include "house.h"

Houses::Houses(int pin, int count)
{
    m_leds = count * 2;
    if (m_leds > 12)
        m_leds = 12;

    switch (pin) {
        case BANK_1_PIN:
            FastLED.addLeds<WS2812, D6, EOrder::RGB>(m_houses, m_leds);
            Log.info("Created a bank of houses on pin D6");
            break;
        case BANK_2_PIN:
            FastLED.addLeds<WS2812, D3, EOrder::RGB>(m_houses, m_leds);
            Log.info("Created a bank of houses on pin D6");
            break;
        case BANK_3_PIN:
            FastLED.addLeds<WS2812, D4, EOrder::RGB>(m_houses, m_leds);
            Log.info("Created a bank of houses on pin D6");
            break;
        default:
            Log.error("Invalid pin %d specified", pin);
    }
    
    Log.info("Created %d new houses", count);
}

Houses::~Houses()
{
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = CRGB::Black;
    }
    FastLED.show();
}

void Houses::turnOn()
{
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = CRGB::White;
    }
    FastLED.show();
}

void Houses::turnOn(int led)
{
    if (led < m_leds) {
        m_houses[led] = CRGB::White;
    }
    FastLED.show();
}

void Houses::turnOff()
{
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = CRGB::Black;
    }
    FastLED.show();
}

void Houses::turnOff(int led)
{
    if (led < m_leds) {
        m_houses[led] = CRGB::Black;
    }
    FastLED.show();
}