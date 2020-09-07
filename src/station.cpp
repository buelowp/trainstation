#include <station.h>

static int g_circle[] = { 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 };
static int g_overheadEvening[] = { 0, 1, 4, 7, 8, 11 };

Station::Station(int pin, int count)
{
    m_station = new Adafruit_NeoPixel(count, pin, SK6812RGBW);
    m_leds = count;
    Log.info("Created a new station on pin %d", pin);
}

Station::~Station()
{
    Log.info("Station destructor, turning off station");
    m_station->begin();
    m_station->show();
    delete m_station;
}

void Station::turnOnStandard()
{
    m_station->begin();
    Log.info("Turning station lights on, white");
    for (int i = 0; i < 6; i++) {
        m_station->setPixelColor(g_overheadEvening[i], 0, 0, 0, 255);
    }
    m_station->show();
}

void Station::turnOnStandardColor(int r, int g, int b, int w)
{
    m_station->begin();
    Log.info("Turning the station on to color %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < 6; i++) {
        m_station->setPixelColor(g_overheadEvening[i], r, g, b, w);
    }
    m_station->show();
}

void Station::turnOn()
{
    Log.info("Turning all station lighs on, white");
    m_station->begin();
    for (int i = 0; i < m_leds; i++) {
        m_station->setPixelColor(i, 0, 0, 0, 255);
    }
    m_station->show();
}

void Station::turnOnAllColor(int r, int g, int b, int w)
{
    m_station->begin();

    Log.info("Turning all station lights on to color %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < m_leds; i++) {
        m_station->setPixelColor(i, r, g, b, w);
    }
    m_station->show();
}

void Station::turnOnCircle()
{
    m_station->begin();
    Log.info("Turning on station circle to white");
    for (int i = 0; i < 12; i++) {
        m_station->setPixelColor(g_circle[i], 0, 0, 0, 255);
    }
    m_station->show();
}

void Station::turnOnCircleColor(int r, int g, int b, int w)
{
    m_station->begin();

    Log.info("Turning circle station lights on to color %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < 12; i++) {
        m_station->setPixelColor(g_circle[i], r, g, b, w);
    }
    m_station->show();
}

void Station::turnOff()
{
    Log.info("Turning station lights off");
    m_station->begin();
    m_station->show();
}

void Station::blinkToLife()
{
    Log.info("Blinking to life");
    turnOnStandardColor(0, 0, 0, 150);
    delay(400);
    m_station->clear();
    m_station->show();
    delay(500);
    turnOnStandardColor(0, 0, 0, 200);
    delay(200);
    m_station->clear();
    m_station->show();
    delay(300);
    turnOnStandardColor(100, 0, 0, 150);
    delay(150);    
    m_station->clear();
    m_station->show();
    delay(200);
    turnOnStandardColor(0, 0, 0, 200);
    delay(100);
    m_station->clear();
    m_station->show();
    delay(900);
    turnOnStandardColor(0, 0, 0, 200);
    delay(400);
    m_station->clear();
    m_station->show();
    delay(100);
    turnOnStandardColor(0, 0, 100, 100);
    delay(200);
    m_station->clear();
    m_station->show();
    delay(100);
    turnOnStandardColor(0, 0, 0, 255);
    m_station->show();
}
