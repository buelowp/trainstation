#include "adafruithouses.h"

AdafruitHouses::AdafruitHouses(int pin, int count) : Houses(pin, count)
{
    m_houses = new Adafruit_NeoPixel(count, pin, Adafruit_NeoPixel::SK6812RGBW);
    m_leds = count;
    m_houseCount = count / 7;
    m_houses->begin();
    m_houses->show();
    Log.info("Created a new station on pin %d with %d leds", pin, m_leds);
}

AdafruitHouses::~AdafruitHouses()
{
    Log.info("Station: destructor, turning off station");
    m_houses->clear();
    m_houses->show();
    delete m_houses;
}

void AdafruitHouses::turnOn()
{
    Log.info("AdafruitHouses: Turning all AdafruitHouses on");
    for (int i = 0; i < m_leds; i++) {
        m_houses->setColor(i, 0, 0, 0, 50);
    }
    m_houses->show();
}

bool AdafruitHouses::turnOn(int house)
{
    int bright = random(50, 80);

    if (house < m_houseCount) {
        for (int i = 0; i < 7; i++) {
            m_houses->setColor(i + (house * 7), 0, 0, 0, bright);
        }
        m_houses->show();
        Log.info("%s:%d: Turning house %d on (0x%08lx)", __PRETTY_FUNCTION__, __LINE__, house, m_houses->getPixelColor(house));
        return true;
    }
    Log.error("AdafruitHouses: House %d is not valid", house);
    return false;
}

bool AdafruitHouses::turnOn(int house, uint8_t bright)
{
    if (house < m_houseCount) {
        Log.info("AdafruitHouses: Turning house %d on", house);
        for (int i = 0; i < 7; i++) {
            m_houses->setColor(i + (house * 7), 0, 0, 0, bright);
        }
        m_houses->show();
        return true;
    }
    Log.error("AdafruitHouses: House %d is not valid", house);
    return false;
}

void AdafruitHouses::turnOff()
{
    Log.info("AdafruitHouses: Turning all AdafruitHouses off");

    m_houses->clear();
    m_houses->show();
}

bool AdafruitHouses::turnOff(int house)
{
    if (house < m_houseCount) {
        for (int i = 0; i < 7; i++) {
            m_houses->setColor(i + (house * 7), 0, 0, 0, 0);
        }
        m_houses->show();
        Log.info("%s:%d: Turning house %d on (0x%08lx)", __PRETTY_FUNCTION__, __LINE__, house, m_houses->getPixelColor(house));
        return true;
    }
    Log.error("AdafruitHouses: House %d is not valid", house);
    return false;
}

bool AdafruitHouses::isOn(int house) 
{
    if (house < m_houseCount) {
        bool result = (m_houses->getPixelColor(house * 7) != 0);
        Log.info("%s:%d: House %d is %d (%ld)",  __PRETTY_FUNCTION__, __LINE__, house, result, m_houses->getPixelColor(house * 7));
        return result;
    }
    Log.error("AdafruitHouses: House %d is not valid", house);
    return false;
}

void AdafruitHouses::setColors(uint8_t r, uint8_t g, uint8_t b)
{
    setColors(r, g, b, 0);
}

void AdafruitHouses::setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b)
{
    setColor(pin, r, g, b, 0);
}

void AdafruitHouses::setColors(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    Log.info("AdafruitHouses: Setting block colors to %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < m_leds; i++) {
        m_houses->setColor(i, r, g, b, w);
    }
    m_houses->show();
}

void AdafruitHouses::setColor(uint8_t pin, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    Log.info("AdafruitHouses: Setting pin %d color to %d:%d:%d:%d", pin, r, g, b, w);
    m_houses->setColor(pin, r, g, b, w);
    m_houses->show();
}

bool AdafruitHouses::allOn()
{
    bool state = true;

    for (int i = 0; i < m_houseCount; i++) {
        state &= isOn(i);
    }

    return state;
}