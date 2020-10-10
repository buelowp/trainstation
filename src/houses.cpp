#include "houses.h"

Houses::Houses(int pin, int count)
{
    m_leds = count;
    m_houseCount = count / 2;

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
            Log.info("Houses: Created bank %d with %d houses on pin D6", m_bank, m_houseCount);
            break;
        case BANK_2_PIN:
            FastLED.addLeds<WS2812, D3, EOrder::RGB>(m_houses, m_leds);
            m_bank = 1;
            Log.info("Houses: Created bank %d with %d houses on pin D3", m_bank, m_houseCount);
            break;
        case BANK_3_PIN:
            FastLED.addLeds<WS2812, D4, EOrder::RGB>(m_houses, m_leds);
            m_bank = 2;
            Log.info("Houses: Created bank %d with %d houses on pin D4", m_bank, m_houseCount);
            break;
        default:
            Log.error("Houses: Invalid pin %d specified", pin);
    }

    m_warmWhite = CRGB(255, 150, 40);
}

Houses::~Houses()
{
    Log.info("Houses: Destructor, turning all houses off");
    FastLED.clear();
    FastLED.show();
}

void Houses::turnOn()
{
    Log.info("Houses: Turning all houses on for bank %d", m_bank);
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = m_warmWhite;
    }
    FastLED.show();
}

bool Houses::turnOn(int house)
{
    int led = house * 2;
    if (house < m_houseCount) {
        Log.info("Houses: Turning house %d on in bank %d", house, m_bank);
        m_houses[led] = m_warmWhite;
        m_houses[led + 1] = m_warmWhite;
        FastLED.show();
        return true;
    }
    Log.error("Houses: House %d is not valid", house);
    return false;
}

void Houses::turnOff()
{
    Log.info("Houses: Turning all houses off for bank %d", m_bank);

    FastLED.clear();
    FastLED.show();
}

bool Houses::turnOff(int house)
{
    int led = house * 2;
    if (house < m_houseCount) {
        Log.info("Houses: Turning house %d off in bank %d", house, m_bank);
        m_houses[led] = CRGB::Black;
        m_houses[led + 1] = CRGB::Black;
        FastLED.show();
        return true;
    }
    Log.error("Houses: House %d is not valid", house);
    return false;
}

bool Houses::isOn(int house) 
{
    int led = house * 2;
    if (house < m_houseCount) {
        bool result = m_houses[led] && m_houses[led + 1];
        Log.info("Houses: House %d is %d", house, result);
        return result;
    }
    Log.error("Houses: House %d is not valid", house);
    return false;
}

void Houses::setColors(uint8_t r, uint8_t g, uint8_t b)
{
    Log.info("Houses: Setting block colors to %d:%d:%d", r, g, b);
    for (int i = 0; i < m_leds; i++) {
        m_houses[i] = CRGB(r, g, b);
    }
    FastLED.show();
}