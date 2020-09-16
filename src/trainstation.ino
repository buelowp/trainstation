#include <sunset.h>
#include <MQTT.h>
#include <map>

#include "block.h"
#include "houses.h"
#include "station.h"
#include "lamps.h"
#include "lamp.h"

#define APP_ID              79

#define TIME_BASE_YEAR      2020
#define CST_OFFSET          -6
#define DST_OFFSET          (CST_OFFSET + 1)

#define ONE_SECOND          (1000)
#define TWO_SECONDS         (ONE_SECOND * 2)
#define TEN_SECONDS         (ONE_SECOND * 10)
#define THIRTY_SECONDS      (ONE_SECOND * 30)
#define ONE_MINUTE          (ONE_SECOND * 60)
#define THREE_MINUTES       (ONE_MINUTE * 3)
#define FIVE_MINUTES        (ONE_MINUTE * 5)
#define ONE_HOUR            (ONE_MINUTE * 60)
#define TWELVE_HOURS        (ONE_HOUR * 12)

#define LATITUDE            41.12345
#define LONGITUDE           -87.98765

#define BANK_1_LEDS         6
#define BANK_2_LEDS         6
#define BANK_3_LEDS         6
#define BANK_4_LEDS         14

#define BANK_1_PIN          D6
#define BANK_2_PIN          D3
#define BANK_3_PIN          D4
#define BANK_4_PIN          D5

#define MQTT_BUFF_SIZE      512

const uint8_t _usDSTStart[22] = { 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9};
const uint8_t _usDSTEnd[22]   = { 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2};
char g_mqttBuffer[MQTT_BUFF_SIZE];
String g_name = "trainstation-";
String g_mqttName = g_name + System.deviceID().substring(0, 8);
byte mqttServer[] = { 172, 24, 1, 13 };
MQTT client(mqttServer, 1883, mqttCallback, 512);
char mqttBuffer[512];
int g_appId;
bool g_stationOn;
bool g_lightsOn;
bool g_lightsOff;
bool g_timeSetDone;
bool g_streetLightsOn;

Houses bank1(BANK_1_PIN, BANK_1_LEDS);
Houses bank2(BANK_2_PIN, BANK_2_LEDS);
Houses bank3(BANK_3_PIN, BANK_3_LEDS);
Station station(BANK_4_PIN, BANK_4_LEDS, &client);
Block blocks;
Lamp lampD0(D0);
Lamp lampD1(D1);
Lamp lampD2(D2);
Lamp lampA0(A0);
Lamp lampA1(A1);
Lamp lampA2(A2);
Lamp lampA3(A3);
Lamp lampA4(A4);
Lamp lampA5(A5);
Lamps lamps;

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
    lamps.turnOff(num.toInt());
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

bool afterSunset(int offset)
{
    int sunset = static_cast<int>(sun.calcSunset());
    int mpm = Time.minute() + (Time.hour() * 60);

    if (mpm >= (sunset + offset)) {
        return true;
    }

    return false;
}

/**
 * You need to account for the fact that after sunset might
 * be earlier than expected, and we can't know what that value
 * is, so we just set it to be 60 minutes earlier.
 */
bool afterSunrise(int offset)
{
    int sunrise = static_cast<int>(sun.calcSunrise());
    int sunset = static_cast<int>(sun.calcSunset());
    int mpm = Time.minute() + (Time.hour() * 60);

    if (mpm >= (sunrise + offset) && (mpm < (sunset - 60))) {
        return true;
    }

    return false;
}

void turnOffNeighborhood()
{
    int mpm = Time.minute() + (Time.hour() * 60);
    double sunrise = sun.calcSunrise();

    Log.info("loop: It's after sunrise (%d:%f), turning the neighborhood lights off", mpm, sunrise);
    station.turnOff();
    lamps.turnOff();
    g_stationOn = false;
    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("appid").value(g_appId);
    writer.name("time");
        writer.beginObject();
        writer.name("mpm").value(mpm);
        writer.endObject();
    writer.name("neighborhood");
        writer.beginObject();
        writer.name("result").value("off");
        writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/state", writer.buffer());
    station.turnOff();
}

void turnOnNeighborhood()
{
    int mpm = Time.minute() + (Time.hour() * 60);
    double sunset = sun.calcSunset();

    Log.info("loop: It's after sunset (%d:%f), turn on the station", mpm, sunset);
    station.blinkToLife();
    lamps.staggerOn();
    g_stationOn = true;
    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("appid").value(g_appId);
    writer.name("time");
        writer.beginObject();
        writer.name("mpm").value(mpm);
        writer.endObject();
    writer.name("neighborhood");
        writer.beginObject();
        writer.name("result").value("on");
        writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/state", writer.buffer());
}

system_tick_t turnOffNextHouse(bool fast)
{   
    int start = ONE_MINUTE;
    int end = FIVE_MINUTES;
    int mpm = Time.minute() + (Time.hour() * 60);

    if (fast) {
        start = ONE_SECOND;
        end = TEN_SECONDS;
    }
    
    system_tick_t nextTimeout = random(start, end) + millis();
    g_lightsOn = blocks.turnOffRandomHouse();
    if (g_lightsOn) {
        Log.info("%s: Turned off random house, will do the next one in %ld millis", __FUNCTION__, nextTimeout - millis());
        JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
        writer.beginObject();
        writer.name("appid").value(g_appId);
        writer.name("time");
            writer.beginObject();
            writer.name("mpm").value(mpm);
            writer.endObject();
        writer.name("house");
            writer.beginObject();
            writer.name("action").value("off");
            writer.name("house").value(blocks.getLastRandomHouse());
            writer.name("block").value(blocks.getLastRandomBlock());
            writer.name("g_lightsOn").value(g_lightsOn);
            writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/state", writer.buffer());
    }
    else {
        Log.info("%s: It seems all lights are off now", __FUNCTION__);
        JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
        writer.beginObject();
        writer.name("appid").value(g_appId);
        writer.name("time");
            writer.beginObject();
            writer.name("mpm").value(mpm);
            writer.endObject();
        writer.name("house");
            writer.beginObject();
            writer.name("action").value("complete");
            writer.name("g_lightsOn").value(g_lightsOn);
            writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/state", writer.buffer());
        nextTimeout = 0;
    }
    return nextTimeout;
}

system_tick_t turnOnNextHouse(bool fast)
{
    int mpm = Time.minute() + (Time.hour() * 60);
    int start = ONE_MINUTE;
    int end = FIVE_MINUTES;

    if (fast) {
        start = ONE_SECOND;
        end = TEN_SECONDS;
    }
    
    system_tick_t nextTimeout = random(start, end) + millis();
    g_lightsOn = !blocks.turnOnRandomHouse();
    if (!g_lightsOn) {
        Log.info("%s: Turned on random house, will do the next one in %ld millis", __FUNCTION__, nextTimeout - millis());
        JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
        writer.beginObject();
        writer.name("appid").value(g_appId);
        writer.name("time");
            writer.beginObject();
            writer.name("mpm").value(mpm);
            writer.endObject();
        writer.name("house");
            writer.beginObject();
            writer.name("action").value("on");
            writer.name("house").value(blocks.getLastRandomHouse());
            writer.name("block").value(blocks.getLastRandomBlock());
            writer.name("g_lightsOn").value(g_lightsOn);
            writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/state", writer.buffer());
    }
    else {
        Log.info("%s: It seems all lights are on now", __FUNCTION__);
        JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
        writer.beginObject();
        writer.name("appid").value(g_appId);
        writer.name("time");
            writer.beginObject();
            writer.name("mpm").value(mpm);
            writer.endObject();
        writer.name("house");
            writer.beginObject();
            writer.name("action").value("complete");
            writer.name("g_lightsOn").value(g_lightsOn);
            writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/state", writer.buffer());
        nextTimeout = 0;
    }
    return nextTimeout;
}

void sendMQTTHeartBeat()
{
    int mpm = Time.minute() + (Time.hour() * 60);
    double sunset = sun.calcSunset();
    double sunrise = sun.calcSunrise();

    Log.info("%s: Sunset at %f, current mpm %d, lights on %d", __FUNCTION__, sunset, mpm, g_lightsOn);
    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("appid").value(g_appId);
    writer.name("village");
        writer.beginObject();
        writer.name("houses").value(g_lightsOn);
        writer.name("station").value(g_stationOn);
        writer.endObject();
    writer.name("time");
        writer.beginObject();
        writer.name("mpm").value(mpm);
        writer.name("sunrise").value(sunrise);
        writer.name("sunset").value(sunset);
        writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/heartbeat", writer.buffer());
}

void setDeviceTime()
{
    Log.info("%s: Setting time", __FUNCTION__);
    Particle.syncTime();
    waitUntil(Particle.syncTimeDone);
    g_timeSetDone = true;
    Time.zone(currentTimeZone());
    sun.setCurrentDate(Time.year(), Time.month(), Time.day());
    sun.setTZOffset(currentTimeZone());
    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("appid").value(g_appId);
    writer.name("time").value("set");
    writer.name("epoch").value(static_cast<unsigned int>(Time.now()));
    writer.name("flag").value(g_timeSetDone);
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/timesync", writer.buffer());
}

int setHouseColor(String colors)
{
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    int index = 0;
    int prev = 0;
    int which = 0;

    while (true) {
        index = colors.indexOf(',', prev);
        if (index == -1 && which != 2) {
            break;
        }
        switch (which) {
            case 0:
                r = colors.substring(prev, index).toInt();
                prev = index;
                which = 1;
                break;
            case 1:
                g = colors.substring(prev + 1, index).toInt();
                prev = index;
                which = 2;
                break;
            case 2:
                b = colors.substring(prev + 1).toInt();
                blocks.setHouseColors(r, g, b);
                return 3;
            default:
                continue;
        }
    }
    return 0;
}

// setup() runs once, when the device is first turned on.
void setup() 
{
    g_timeSetDone = true;
    g_lightsOn = false;
    g_lightsOff = true;
    g_stationOn = false;
    g_streetLightsOn = false;
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
        Log.info("%s: MQTT client connected as %s", __FUNCTION__, g_mqttName.c_str());
    else
    {
        Log.error("%s: Unable to connect to MQTT", __FUNCTION__);
    }

    blocks.addHouses(&bank1);
    blocks.addHouses(&bank2);
    blocks.addHouses(&bank3);
    lamps.addLamp(&lampD0);
    lamps.addLamp(&lampD1);
    lamps.addLamp(&lampD2);
    lamps.addLamp(&lampA0);
    lamps.addLamp(&lampA1);
    lamps.addLamp(&lampA2);
    lamps.addLamp(&lampA3);
    lamps.addLamp(&lampA4);
    lamps.addLamp(&lampA5);

    blocks.turnOff();
    station.turnOff();
    lamps.turnOff();

    Particle.variable("mpm", calculateMPM);
    Particle.variable("sunset", calcSunset);
    Particle.variable("sunrise", calcSunrise);
    Particle.variable("appid", g_appId);
    Particle.variable("timezone", currentTimeZone);
    Particle.function("turnon", turnOnBank);
    Particle.function("switch", turnOnSwitch);
    Particle.function("turnoff", turnOffBank);
    Particle.function("housecolor", setHouseColor);

    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("village");
    writer.beginObject();
    writer.name("action").value("startup");
    writer.name("appid").value(g_appId);
    writer.name("houses").value(g_lightsOn);
    writer.name("station").value(g_stationOn);
    writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/startup", writer.buffer());
    station.turnOff();  // Do this twice, it doesn't seem to take the first time
    int sunrise = static_cast<int>(sun.calcSunrise());
    int sunset = static_cast<int>(sun.calcSunset());
    int mpm = Time.minute() + (Time.hour() * 60);
    Log.info("%s: Setup done for app version %d, mpm=%d, sunrise=%d, sunset=%d", __FUNCTION__, g_appId, mpm, sunrise, sunset);
}

void loop() 
{
    static system_tick_t nextTimeout = 0;
    static int lastHour = 24;
    int currentHour = Time.hour();
    int mpm = Time.minute() + (Time.hour() * 60);

    EVERY_N_MILLIS(ONE_SECOND) {
        if (client.isConnected())
            client.loop();
        else 
            client.connect(g_mqttName.c_str());
    }

    /* Update time and date. Note this is done at 2am to handle changes
     * in timezone which generally happens at 2 am */
    if ((lastHour != currentHour) && currentHour == 2) {
        setDeviceTime();
    }
    lastHour = currentHour;

    EVERY_N_MILLIS(ONE_HOUR) {
        sendMQTTHeartBeat();
    }

    if ((mpm >= 1320) && g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOffNextHouse(true);
        }
    }
    else if ((mpm < 1320) && afterSunset(-10) && !g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOnNextHouse(true);
        }
    }
    else if (afterSunrise(-10) && !afterSunset(-355) && g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOffNextHouse(true);
        }
    }
    else if ((mpm >= 270) && !afterSunrise(-10) && !g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOnNextHouse(false);
        }
    }

    if (afterSunset(0) && !g_stationOn) {
        turnOnNeighborhood();
    }
    if (afterSunrise(5) && g_stationOn) {
        turnOffNeighborhood();
    }
}