#include <station.h>

static int g_circle[] = { 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 };
static int g_overheadEvening[] = { 0, 1, 4, 7, 8, 11 };

extern char *g_mqttBuffer;
extern int g_appId;

Station::Station(int pin, int count) : m_leds(count), m_pin(pin)
{
    m_station = new Adafruit_NeoPixel(m_leds, D5, SK6812RGBW);
    m_station->begin();
    m_station->show();
    Log.info("Created a new station on pin %d with %d leds", pin, m_leds);
}

Station::~Station()
{
    Log.info("Station: destructor, turning off station");
    m_station->clear();
    m_station->show();
    delete m_station;
}

void Station::turnOnStandard()
{
    m_station->clear();
    Log.info("Station: Turning station lights on, white");
    for (int i = 0; i < 6; i++) {
        m_station->setPixelColor(g_overheadEvening[i], 0, 0, 0, 255);
    }
    m_station->show();
}

void Station::turnOnStandardColor(int r, int g, int b, int w)
{
    m_station->clear();
    Log.info("Station: Turning the station on to color %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < 6; i++) {
        m_station->setPixelColor(g_overheadEvening[i], r, g, b, w);
    }
    m_station->show();
}

void Station::turnOn()
{
    Log.info("Station: Turning all station lighs on, white");
    m_station->clear();
    for (int i = 0; i < m_leds; i++) {
        m_station->setPixelColor(i, 0, 0, 0, 255);
    }
    m_station->show();
}

void Station::turnOnAllColor(int r, int g, int b, int w)
{
    m_station->clear();

    Log.info("Station: Turning all station lights on to color %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < m_leds; i++) {
        m_station->setPixelColor(i, r, g, b, w);
    }
    m_station->show();
}

void Station::turnOnCircle()
{
    m_station->clear();
    Log.info("Station: Turning on station circle to white");
    for (int i = 0; i < 12; i++) {
        m_station->setPixelColor(g_circle[i], 0, 0, 0, 255);
    }
    m_station->show();
}

void Station::turnOnCircleColor(int r, int g, int b, int w)
{
    m_station->clear();

    Log.info("Station: Turning circle station lights on to color %d:%d:%d:%d", r, g, b, w);
    for (int i = 0; i < 12; i++) {
        m_station->setPixelColor(g_circle[i], r, g, b, w);
    }
    m_station->show();
}

void Station::turnOff()
{
    m_station->clear();
    m_station->show();
    Log.info("%s: turning off station lights", __PRETTY_FUNCTION__);
}

void Station::blinkToLife()
{
    Log.info("%s: blinktolife", __PRETTY_FUNCTION__);

    turnOnStandardColor(0, 0, 0, 150);
    delay(400);
    m_station->clear();
    m_station->show();
    delay(200);
    turnOnStandardColor(0, 0, 0, 200);
    delay(200);
    m_station->clear();
    m_station->show();
    delay(150);
    turnOnStandardColor(100, 0, 0, 150);
    delay(150);    
    m_station->clear();
    m_station->show();
    delay(200);
    turnOnStandardColor(0, 0, 150, 100);
    delay(100);
    m_station->clear();
    m_station->show();
    delay(250);
    turnOnStandardColor(0, 100, 0, 150);
    delay(400);
    m_station->clear();
    m_station->show();
    delay(100);
    turnOnStandardColor(0, 0, 100, 100);
    delay(150);
    m_station->clear();
    m_station->show();
    delay(100);
    turnOnStandardColor(0, 0, 0, 255);
    m_station->show();
}
