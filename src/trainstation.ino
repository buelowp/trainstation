#include <neopixel.h>
#include <sunset.h>
#include <MQTT.h>
#include <map>

#define TIME_BASE_YEAR        2020
#define CST_OFFSET          -6
#define DST_OFFSET          (CST_OFFSET + 1)
#define ONE_SECOND          (1000)
#define TWO_SECONDS         (ONE_SECOND * 2)
#define ONE_MINUTE          (ONE_SECOND * 60)
#define FIVE_MINUTES        (ONE_MINUTE * 5)
#define ONE_HOUR            (ONE_MINUTE * 60)

#define LATITUDE            41.12345
#define LONGITUDE           -87.98765
#define CST_OFFSET          -6
#define DST_OFFSET          (CST_OFFSET + 1)

#define BANK_1_LEDS         5
#define BANK_2_LEDS         5
#define BANK_3_LEDS         5
#define BANK_4_LEDS         14

#define BANK_1_PIN          D3
#define BANK_2_PIN          D4
#define BANK_3_PIN          D5
#define BANK_4_PIN          D6

#define BANK_1              BANK_1_PIN
#define BANK_2              BANK_2_PIN
#define BANK_3              BANK_3_PIN
#define BANK_4              BANK_4_PIN

const uint8_t _usDSTStart[22] = { 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9};
const uint8_t _usDSTEnd[22]   = { 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2};
String g_name = "trainstation-";
String g_mqttName = g_name + System.deviceID().substring(0, 8);
byte mqttServer[] = { 172, 24, 1, 13 };
MQTT client(mqttServer, 1883, mqttCallback, 512);
char mqttBuffer[512];
int g_timeZone;
bool g_stationOn;
bool g_lightsOn;
bool g_timeSetDone;

int g_circle[] = { 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 };
int g_overheadEvening[] = { 0, 1, 4, 7, 8, 11 };

Adafruit_NeoPixel bank1(BANK_1_LEDS, BANK_1_PIN, WS2812);
Adafruit_NeoPixel bank2(BANK_2_LEDS, BANK_2_PIN, WS2812);
Adafruit_NeoPixel bank3(BANK_3_LEDS, BANK_3_PIN, WS2812);
Adafruit_NeoPixel station(BANK_4_LEDS, BANK_4_PIN, SK6812RGBW);
SunSet sun;
SerialLogHandler logHandler;

int currentTimeZone()
{
    g_timeZone = DST_OFFSET;
    
    if (Time.month() > 3 && Time.month() < 11) {
        return DST_OFFSET;
    }
    if (Time.month() == 3) {
        if ((Time.day() == _usDSTStart[Time.year() - TIME_BASE_YEAR]) && Time.hour() >= 2)
            return DST_OFFSET;
        if (Time.day() > _usDSTStart[Time.year() - TIME_BASE_YEAR])
            return DST_OFFSET;
    }
    if (Time.month() == 11) {
        if ((Time.day() == _usDSTEnd[Time.year() - TIME_BASE_YEAR]) && Time.hour() <=2)
            return DST_OFFSET;
        if (Time.day() < _usDSTEnd[Time.year() - TIME_BASE_YEAR])
            return DST_OFFSET;
    }
    g_timeZone = CST_OFFSET;
    return CST_OFFSET;
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Got message on topic ");
    Serial.println(topic);
}

/*
 * Lights for normal evening looking
 */
void drawOverheadLights(int r, int g, int b, int w)
{
    station.begin();

    for (int i = 0; i < 6; i++) {
        station.setPixelColor(g_overheadEvening[i], r, g, b, w);
    }
    station.show();
}

void blinkToLife()
{
    drawOverheadLights(0, 0, 0, 150);
    delay(400);
    station.clear();
    station.show();
    delay(500);
    drawOverheadLights(0, 0, 0, 200);
    delay(200);
    station.clear();
    station.show();
    delay(300);
    drawOverheadLights(100, 0, 0, 150);
    delay(150);    
    station.clear();
    station.show();
    delay(200);
    drawOverheadLights(0, 0, 0, 200);
    delay(100);
    station.clear();
    station.show();
    delay(900);
    drawOverheadLights(0, 0, 0, 200);
    delay(400);
    station.clear();
    station.show();
    delay(100);
    drawOverheadLights(0, 0, 100, 100);
    delay(200);
    station.clear();
    station.show();
    delay(100);
    drawOverheadLights(0, 0, 0, 255);
    station.show();
}

void turnOnStationLights()
{
    blinkToLife();
}

// setup() runs once, when the device is first turned on.
void setup() 
{
    g_timeSetDone = true;
    g_lightsOn = false;
    g_stationOn = false;

    sun.setPosition(LATITUDE, LONGITUDE, currentTimeZone());
    sun.setCurrentDate(Time.year(), Time.month(), Time.day());

    client.connect(g_mqttName.c_str());
    if (client.isConnected())
        Log.info("MQTT client connected as %s", g_mqttName.c_str());
    else
    {
        /* If we can't connect, go to sleep and try again! */
        Log.error("Unable to connect to MQTT");
    }
    bank1.clear();
    bank1.show();
    bank2.clear();
    bank2.show();
    bank3.clear();
    bank3.show();
    station.clear();
    station.show();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() 
{
    static system_tick_t nextTimeout = 0;
    int mpm = Time.minute() + (Time.hour() * 60);

    /* Update time and date. Note this is done after 2am to handle changes
     * in timezone. For the first 2 hours, the sunset/sunrise will be slightly off */
    if (Time.hour() == 3 && !g_timeSetDone) {
        Particle.syncTime();
        waitUntil(Particle.syncTimeDone);
        sun.setCurrentDate(Time.year(), Time.month(), Time.day());
        sun.setTZOffset(currentTimeZone());
        g_timeSetDone = true;
    }
    else if (Time.hour() != 3) {
        g_timeSetDone = false;
    }

    double sunset = sun.calcSunset();
    double sunrise = sun.calcSunrise();

    if (mpm >= (sunrise - 15) && g_lightsOn) {
        if (millis() > nextTimeout) {
            int bank = random(0, 2);
            int entry = 0;
            bool foundAndTurnedOff = false;
            int pixelCount = 15;
            while (!foundAndTurnedOff && pixelCount > 0) {
                switch (bank) {
                    case 0:
                        entry = random(0, bank1.getNumLeds());
                        if (bank1.getPixelColor(entry) != 0) {
                            bank1.setPixelColor(entry, 0, 0, 0);
                            foundAndTurnedOff = true;
                        }
                        else {
                            pixelCount--;
                        }
                        break;
                    case 1:
                        entry = random(0, bank1.getNumLeds());
                        if (bank1.getPixelColor(entry) != 0) {
                            bank1.setPixelColor(entry, 0, 0, 0);
                        }
                        else {
                            pixelCount--;
                        }
                        break;
                    case 2:
                        entry = random(0, bank1.getNumLeds());
                        if (bank1.getPixelColor(entry) != 0) {
                            bank1.setPixelColor(entry, 0, 0, 0);
                        }
                        else {
                            pixelCount--;
                        }
                        break;
                    default:
                        Log.info("Shouldn't have gotten here");
                        break;
                }
                nextTimeout = random(ONE_MINUTE, FIVE_MINUTES);
            }
            if (pixelCount == 0)
                g_lightsOn = false;
        }
    }

    if (mpm >= sunrise && g_stationOn) {
        station.clear();
        station.show();
        g_stationOn = false;
    }

    if (mpm >= (sunset - 15) && !g_lightsOn) {

    }

    if ((mpm >= sunset) && !g_stationOn) {
        blinkToLife();
        g_stationOn = true;
    }
}