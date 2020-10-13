#include "fastledhouses.h"

FastLEDHouses::FastLEDHouses(int pin, int count) : Houses(pin, count)
{
    /*
     * This is tricky. The addLeds fails passing an int or const in variable
     * for some reason. I haven't figured it out yet. So, you tell the class
     * which pin, and then tell FastLED to use that pin directly. It won't
     * accept the int pin value
     */
    switch (pin) {
        case BANK_1_PIN:
            FastLED.addLeds<WS2812, D6, EOrder::RGB>(m_houses, m_leds);
            m_bank = 0;
            Log.info("FastLEDHouses: Created bank %d with %d FastLEDHouses on pin D6", m_bank, m_houseCount);
            break;
        case BANK_2_PIN:
            FastLED.addLeds<WS2812, D3, EOrder::RGB>(m_houses, m_leds);
            m_bank = 1;
            Log.info("FastLEDHouses: Created bank %d with %d FastLEDHouses on pin D3", m_bank, m_houseCount);
            break;
        case BANK_3_PIN:
            FastLED.addLeds<WS2812, D4, EOrder::RGB>(m_houses, m_leds);
            m_bank = 2;
            Log.info("FastLEDHouses: Created bank %d with %d FastLEDHouses on pin D4", m_bank, m_houseCount);
            break;
        default:
            Log.error("FastLEDHouses: Invalid pin %d specified", pin);
    }

    m_houseCount = m_leds / 2;
    m_warmWhite = CRGB(255, 150, 40);
}

FastLEDHouses::~FastLEDHouses()
{
    Log.info("FastLEDHouses: Destructor, turning all FastLEDHouses off");
    FastLED.clear();
    FastLED.show();
}

void FastLEDHouses::turnOn()
{
    Log.info("FastLEDHouses: Turning all FastLEDHouses on for bank %d", m_bank);
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = m_warmWhite;
    }
    FastLED.show();
}

bool FastLEDHouses::turnOn(int house)
{
    int led = house * 2;
    if (house < m_houseCount) {
        Log.info("FastLEDHouses: Turning house %d on in bank %d", house, m_bank);
        m_houses[led] = m_warmWhite;
        m_houses[led + 1] = m_warmWhite;
        FastLED.show();
        return true;
    }
    Log.error("FastLEDHouses: House %d is not valid", house);
    return false;
}

void FastLEDHouses::turnOff()
{
    Log.info("FastLEDHouses: Turning all FastLEDHouses off for bank %d", m_bank);

    FastLED.clear();
    FastLED.show();
}

bool FastLEDHouses::turnOff(int house)
{
    int led = house * 2;
    if (house < m_houseCount) {
        Log.info("FastLEDHouses: Turning house %d off in bank %d", house, m_bank);
        m_houses[led] = CRGB::Black;
        m_houses[led + 1] = CRGB::Black;
        FastLED.show();
        return true;
    }
    Log.error("FastLEDHouses: House %d is not valid", house);
    return false;
}

bool FastLEDHouses::isOn(int house) 
{
    int led = house * 2;
    if (house < m_houseCount) {
        bool result = m_houses[led] && m_houses[led + 1];
        Log.info("FastLEDHouses: House %d is %d", house, result);
        return result;
    }
    Log.error("FastLEDHouses: House %d is not valid", house);
    return false;
}

void FastLEDHouses::setColors(uint8_t r, uint8_t g, uint8_t b)
{
    Log.info("FastLEDHouses: Setting block colors to %d:%d:%d", r, g, b);
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = CRGB(r, g, b);
    }
    FastLED.show();
}

void FastLEDHouses::setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b)
{
    Log.info("FastLEDHouses: Setting pin %d color to %d:%d:%d", pin, r, g, b);
    m_houses[pin] = CRGB(r, g, b);
    FastLED.show();
}

void FastLEDHouses::setColors(uint8_t r, uint8_t g, uint8_t b, uint8_t)
{
    Log.info("FastLEDHouses: Setting block colors to %d:%d:%d, dropping white", r, g, b);
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = CRGB(r, g, b);
    }
    FastLED.show();
}

void FastLEDHouses::setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b, uint8_t)
{
    Log.info("FastLEDHouses: Setting pin %d color to %d:%d:%d, dropping white", pin, r, g, b);
    m_houses[pin] = CRGB(r, g, b);
    FastLED.show();
}