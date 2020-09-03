/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/pete/Projects/github/trainstation/src/trainstation.ino"
#include <neopixel.h>
#include <sunset.h>
#include <MQTT.h>
#include <map>

int currentTimeZone();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void setup();
void loop();
#line 6 "/home/pete/Projects/github/trainstation/src/trainstation.ino"
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
bool g_state;
bool g_signalsOn;
bool g_signalSpeaker;
bool g_enableSignalSpeaker;
bool g_sunsetTimerNotSet;
unsigned long g_lastOffEvent;

int g_circle[] = { 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 };
int g_overheadEvening[] = { 0, 1, 4, 7, 8, 11 };

Adafruit_NeoPixel bank1(BANK_1_LEDS, BANK_1_PIN, WS2812);
Adafruit_NeoPixel bank2(BANK_2_LEDS, BANK_2_PIN, WS2812);
Adafruit_NeoPixel bank3(BANK_3_LEDS, BANK_3_PIN, WS2812);
Adafruit_NeoPixel station(BANK_4_LEDS, BANK_4_PIN, SK6812RGBW);
SunSet sun;
SerialLogHandler logHandler;

std::map<int, Adafruit_NeoPixel*> g_banks;

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

// setup() runs once, when the device is first turned on.
void setup() 
{
    g_banks[BANK_1] = &bank1;
    g_banks[BANK_2] = &bank2;
    g_banks[BANK_3] = &bank3;
    g_banks[BANK_4] = &station;

    client.connect(g_mqttName.c_str());
    if (client.isConnected())
        Log.info("MQTT client connected as %s", g_mqttName.c_str());
    else
    {
        /* If we can't connect, go to sleep and try again! */
        Log.error("Unable to connect to MQTT");
    }

    for (const auto &bank : g_banks) {
        bank.second->begin();
        bank.second->show();
    }
}

// loop() runs over and over again, as quickly as it can execute.
void loop() 
{
}