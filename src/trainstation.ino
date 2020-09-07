#include <sunset.h>
#include <MQTT.h>
#include <map>

#include "house.h"
#include "station.h"

#define APP_ID              15

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

#define BANK_1_LEDS         3
#define BANK_2_LEDS         3
#define BANK_3_LEDS         5
#define BANK_4_LEDS         14

#define BANK_1_PIN          D6
#define BANK_2_PIN          D3
#define BANK_3_PIN          D4
#define BANK_4_PIN          D5

const uint8_t _usDSTStart[22] = { 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9};
const uint8_t _usDSTEnd[22]   = { 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2};
String g_name = "trainstation-";
String g_mqttName = g_name + System.deviceID().substring(0, 8);
byte mqttServer[] = { 172, 24, 1, 13 };
MQTT client(mqttServer, 1883, mqttCallback, 512);
char mqttBuffer[512];
int g_appId;
bool g_stationOn;
bool g_lightsOn;
bool g_timeSetDone;

Houses bank1(BANK_1_PIN, BANK_1_LEDS);
Houses bank2(BANK_2_PIN, BANK_2_LEDS);
Houses bank3(BANK_3_PIN, BANK_3_LEDS);
Station station(BANK_4_PIN, BANK_4_LEDS);

SunSet sun;
SerialLogHandler logHandler;

int currentTimeZone()
{
    int tz = CST_OFFSET;
    
    if (Time.month() > 3 && Time.month() < 11) {
        tz = DST_OFFSET;
    }
    if (Time.month() == 3) {
        if ((Time.day() == _usDSTStart[Time.year() - TIME_BASE_YEAR]) && Time.hour() >= 2)
            tz =  DST_OFFSET;
        if (Time.day() > _usDSTStart[Time.year() - TIME_BASE_YEAR])
            tz =  DST_OFFSET;
    }
    if (Time.month() == 11) {
        if ((Time.day() == _usDSTEnd[Time.year() - TIME_BASE_YEAR]) && Time.hour() <=2)
            tz =  DST_OFFSET;
        if (Time.day() < _usDSTEnd[Time.year() - TIME_BASE_YEAR])
            tz =  DST_OFFSET;
    }

    Log.info("Returning %d as timezone offset", tz);
    return tz;
}

int turnOnSwitch(String num)
{
    Log.info("Turning on switch %ld", num.toInt());
    switch (num.toInt()) {
        case 0:
            digitalWrite(D0, !digitalRead(D0));
            break;
        case 1:
            digitalWrite(D1, HIGH);
            break;
        case 2:
            digitalWrite(D2, HIGH);
            break;
        case 3:
            digitalWrite(A0, HIGH);
            break;
        case 4:
            digitalWrite(A1, HIGH);
            break;
        case 5:
            digitalWrite(A2, HIGH);
            break;
        case 6:
            digitalWrite(A3, HIGH);
            break;
        case 7:
            digitalWrite(A4, HIGH);
            break;
        case 8:
            digitalWrite(A5, HIGH);
            break;
    }
    return num.toInt();
}

int turnOffBank(String num)
{
    Log.info("Turning off bank %ld", num.toInt());

    switch (num.toInt()){
        case 0:
            bank1.turnOff();
            break;
        case 1:
            bank2.turnOff();
            break;
        case 2:
            bank3.turnOff();
            break;
        case 3:
            station.turnOff();
            break;
        default:
            break;
    }

    return num.toInt();
}

int turnOnBank(String num)
{
    Log.info("Turning on bank %ld", num.toInt());

    switch (num.toInt()) {
        case 0:
            Log.info("Setting %d pixels on bank1 to white", bank1.numPixels());
            bank1.turnOn();
            break;
        case 1:
            Log.info("Setting %d pixels on bank2 to white", bank2.numPixels());
            bank2.turnOn();
            break;
        case 2:
            Log.info("Setting %d pixels on bank3 to white", bank3.numPixels());
            bank3.turnOn();
            break;
        case 3:
            turnOnStationLights();
            break;
        default:
            return -1;
    }

    return num.toInt();
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Got message on topic ");
    Serial.println(topic);
}

void turnOnStationLights()
{
    Log.info("Turning on station lights");
    station.blinkToLife();
}

int calculateMPM()
{
    return Time.minute() + (Time.hour() * 60);
}

double calcSunrise()
{
    return sun.calcSunrise();
}

double calcSunset()
{
    return sun.calcSunset();
}

// setup() runs once, when the device is first turned on.
void setup() 
{
    g_timeSetDone = true;
    g_lightsOn = false;
    g_stationOn = false;
    g_appId = APP_ID;

    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(A0, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    digitalWrite(D0, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(A0, LOW);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
    digitalWrite(A3, LOW);
    digitalWrite(A4, LOW);
    digitalWrite(A5, LOW);

    Time.zone(currentTimeZone());
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

    bank1.turnOff();
    bank2.turnOff();
    bank3.turnOff();
    station.turnOff();

    Particle.variable("mpm", calculateMPM);
    Particle.variable("sunset", calcSunset);
    Particle.variable("sunrise", calcSunrise);
    Particle.variable("appid", g_appId);
    Particle.variable("timezone", currentTimeZone);
    Particle.function("turnon", turnOnBank);
    Particle.function("switch", turnOnSwitch);
    Particle.function("turnoff", turnOffBank);
    Log.info("Setup done");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() 
{
    static system_tick_t nextTimeout = 0;
    int mpm = Time.minute() + (Time.hour() * 60);

    /* Update time and date. Note this is done after 2am to handle changes
     * in timezone. */
    if (Time.hour() == 3 && !g_timeSetDone) {
        Log.info("Setting time");
        Particle.syncTime();
        waitUntil(Particle.syncTimeDone);
        Time.zone(currentTimeZone());
        sun.setCurrentDate(Time.year(), Time.month(), Time.day());
        sun.setTZOffset(currentTimeZone());
        g_timeSetDone = true;
    }
    else if (Time.hour() != 3 && g_timeSetDone) {
        g_timeSetDone = false;
        Log.info("Stopping time set");
    }

    double sunset = sun.calcSunset();
    double sunrise = sun.calcSunrise();

    if (mpm >= (sunrise - 15) && g_lightsOn) {
/*
        Log.info("After sunrise, turning lights off");
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
                            Log.info("Turned off light %d:%d", bank, entry);
                        }
                        else {
                            pixelCount--;
                        }
                        break;
                    case 1:
                        entry = random(0, bank1.getNumLeds());
                        if (bank1.getPixelColor(entry) != 0) {
                            bank1.setPixelColor(entry, 0, 0, 0);
                            Log.info("Turned off light %d:%d", bank, entry);
                        }
                        else {
                            pixelCount--;
                        }
                        break;
                    case 2:
                        entry = random(0, bank1.getNumLeds());
                        if (bank1.getPixelColor(entry) != 0) {
                            bank1.setPixelColor(entry, 0, 0, 0);
                            Log.info("Turned off light %d:%d", bank, entry);
                        }
                        else {
                            pixelCount--;
                        }
                        break;
                    default:
                        Log.error("Shouldn't have gotten here");
                        break;
                }
                nextTimeout = random(ONE_MINUTE, FIVE_MINUTES);
            }
            if (pixelCount == 0)
                g_lightsOn = false;
        }
        */
    }

    if ((mpm >= sunrise) && g_stationOn) {
        Log.info("It's after sunrise (%d:%f), turning the station off", mpm, sunrise);
        station.turnOff();
        g_stationOn = false;
    }

    if (mpm >= (sunset - 15) && !g_lightsOn) {

    }

    if ((mpm >= sunset) && !g_stationOn) {
        Log.info("It's after sunset (%d:%f), turn on the station", mpm, sunset);
        station.blinkToLife();
        g_stationOn = true;
    }
}