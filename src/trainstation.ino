#include <sunset.h>
#include <MQTT.h>
#include <vl6180x.h>
#include <map>

#include "village.h"
#include "adafruithouses.h"
#include "fastledhouses.h"
#include "station.h"
#include "lamps.h"
#include "lamp.h"

#define APP_ID              130

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

#define JEWEL_LED_COUNT     7
#define BANK_1_LEDS         (6 * JEWEL_LED_COUNT)      // 6 houses * 7 leds
#define BANK_2_LEDS         (6 * JEWEL_LED_COUNT)
#define BANK_3_LEDS         (6 * JEWEL_LED_COUNT)
#define BANK_4_LEDS         (2 * JEWEL_LED_COUNT)      // 2 jewels * 7 leds

#define BANK_1_PIN          D6
#define BANK_2_PIN          D3
#define BANK_3_PIN          D4
#define BANK_4_PIN          D5

#define MQTT_BUFF_SIZE      512
#define VL6180X_ADDRESS     0x29

#define MIN_ENABLE_DISTANCE     100
#define TURNOFF_THRESHOLD       8.0

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
bool g_pendingNotification;

VL6180xIdentification identification;
VL6180x sensor(VL6180X_ADDRESS);

AdafruitHouses block1(BANK_1_PIN, BANK_1_LEDS);
AdafruitHouses block2(BANK_2_PIN, BANK_2_LEDS);
AdafruitHouses block3(BANK_3_PIN, BANK_3_LEDS);
Station station(BANK_4_PIN, BANK_4_LEDS);
Village village;
Lamp lampTX(TX);
Lamp lampRX(RX);
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

void printIdentification(struct VL6180xIdentification *temp)
{
    JSONBufferWriter writer(g_mqttBuffer, MQTT_BUFF_SIZE);
    writer.beginObject();
    writer.name("sensor").value("vl1608x");
    writer.name("version").value(APP_ID);
    writer.name("data");
    writer.beginObject();
        writer.name("MODEL_ID").value(temp->idModel);
        writer.name("revision").value(String(temp->idModelRevMajor) + "." + String(temp->idModelRevMinor));
    writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/tof/startup", writer.buffer());
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
            block1.turnOff();
            break;
        case 1:
            block2.turnOff();
            break;
        case 2:
            block3.turnOff();
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
            Log.info("Setting %d pixels on block1 to white", block1.numPixels());
            block1.turnOn();
            break;
        case 1:
            Log.info("Setting %d pixels on block2 to white", block2.numPixels());
            block2.turnOn();
            break;
        case 2:
            Log.info("Setting %d pixels on block3 to white", block3.numPixels());
            block3.turnOn();
            break;
        case 3:
            station.turnOn();
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

bool afterSunset()
{
    int sunset = static_cast<int>(sun.calcSunset());
    int mpm = Time.minute() + (Time.hour() * 60);

    if (mpm >= (sunset)) {
        return true;
    }

    return false;
}

/**
 * You need to account for the fact that after sunset might
 * be earlier than expected, and we can't know what that value
 * is, so we just set it to be 60 minutes earlier.
 */
bool afterSunrise()
{
    int sunrise = static_cast<int>(sun.calcSunrise());
    int mpm = Time.minute() + (Time.hour() * 60);

    if (mpm >= sunrise) {
        return true;
    }

    return false;
}

void turnOffNeighborhood()
{
    int mpm = Time.minute() + (Time.hour() * 60);
    double sunrise = sun.calcSunrise();

    Log.info("%s: It's after sunrise (%d:%f), turning the neighborhood lights off", __PRETTY_FUNCTION__, mpm, sunrise);
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

    Log.info("%s: It's after sunset (%d:%f), turn on the station", __PRETTY_FUNCTION__, mpm, sunset);
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
    int start = THIRTY_SECONDS;
    int end = THREE_MINUTES;
    int mpm = Time.minute() + (Time.hour() * 60);

    if (fast) {
        start = ONE_SECOND;
        end = TEN_SECONDS;
    }
    
    system_tick_t nextTimeout = random(start, end) + millis();
    g_lightsOn = village.turnOffRandomHouse();
    if (g_lightsOn) {
        Log.info("%s: Turned off random house, will do the next one in %ld millis", __PRETTY_FUNCTION__, nextTimeout - millis());
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
            writer.name("house").value(village.getLastRandomHouse());
            writer.name("block").value(village.getLastRandomBlock());
            writer.name("g_lightsOn").value(g_lightsOn);
            writer.name("next").value(static_cast<int>(nextTimeout));
            writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/state", writer.buffer());
    }
    else {
        Log.info("%s: It seems all lights are off now", __PRETTY_FUNCTION__);
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
    int start = THIRTY_SECONDS;
    int end = THREE_MINUTES;

    if (fast) {
        start = ONE_SECOND;
        end = TEN_SECONDS;
    }
    
    system_tick_t nextTimeout = random(start, end) + millis();
    if ((Time.day() == 25 || Time.day() == 24) && (Time.month() == 12))
        g_lightsOn = !village.turnOnRandomHouseWithRandomColor();
    else
        g_lightsOn = !village.turnOnRandomHouse();

    if (!g_lightsOn) {
        Log.info("%s: Turned on random house, will do the next one in %ld millis", __PRETTY_FUNCTION__, nextTimeout - millis());
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
            writer.name("house").value(village.getLastRandomHouse());
            writer.name("block").value(village.getLastRandomBlock());
            writer.name("g_lightsOn").value(g_lightsOn);
            writer.name("next").value(static_cast<int>(nextTimeout));
            writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/state", writer.buffer());
    }
    else {
        Log.info("%s: It seems all lights are on now", __PRETTY_FUNCTION__);
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

    Log.info("%s: Sunset at %f, current mpm %d, lights on %d", __PRETTY_FUNCTION__, sunset, mpm, g_lightsOn);
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
        writer.name("sensor");
            writer.beginObject();
                writer.name("name").value("vl1680x");
                writer.name("distance").value(sensor.getDistance());
                writer.name("lux").value(sensor.getAmbientLight(GAIN_1));
            writer.endObject();
        writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/heartbeat", writer.buffer());
}

void setDeviceTime()
{
    Log.info("%s: Setting time", __PRETTY_FUNCTION__);
    Particle.syncTime();
    waitUntil(Particle.syncTimeDone);
    Time.zone(currentTimeZone());
    sun.setCurrentDate(Time.year(), Time.month(), Time.day());
    sun.setTZOffset(currentTimeZone());
    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("appid").value(g_appId);
    writer.name("time").value("set");
    writer.name("epoch").value(static_cast<unsigned int>(Time.now()));
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
                village.setHouseColors(r, g, b, 0);
                return 3;
            default:
                continue;
        }
    }
    return 0;
}

void tofSensorInit()
{
    Wire.begin(); //Start I2C library
    delay(100); // delay .1s

    if(sensor.VL6180xInit() != 0){
        Log.error("FAILED TO INITALIZE vl6180x sensor"); //Initialize device and check for errors
        JSONBufferWriter writer(g_mqttBuffer, MQTT_BUFF_SIZE);
        writer.beginObject();
        writer.name("appid").value(g_appId);
        writer.name("sensor").value("vl1680x");
        writer.name("data");
        writer.beginObject();
            writer.name("startup").value("failed");
        writer.endObject();
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/tof/startup", writer.buffer());
    }; 
    sensor.VL6180xDefautSettings(); //Load default settings to get started.
    sensor.getIdentification(&identification); // Retrieve manufacture info from device memory
    printIdentification(&identification); // Helper function to print all the Module information
}

void checkTOFSensor()
{
    JSONBufferWriter writer(g_mqttBuffer, MQTT_BUFF_SIZE);
    if (sensor.getDistance() < MIN_ENABLE_DISTANCE && !g_pendingNotification) {
        writer.beginObject();
        writer.name("type").value("high");
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/tof/event", writer.buffer());
        g_pendingNotification = true;
        return;
    }
    
    if (sensor.getDistance() >= MIN_ENABLE_DISTANCE && g_pendingNotification) {
        writer.beginObject();
        writer.name("type").value("low");
        writer.endObject();
        writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
        client.publish("village/tof/event", writer.buffer());
        g_pendingNotification = false;
        return;
    }
}

int showDistance()
{
    return sensor.getDistance();
}

double showLight()
{
    return sensor.getAmbientLight(GAIN_1);
}

// setup() runs once, when the device is first turned on.
void setup() 
{
    g_lightsOn = false;
    g_stationOn = false;
    g_appId = APP_ID;
    g_pendingNotification = false;

    Time.zone(currentTimeZone());
    sun.setPosition(LATITUDE, LONGITUDE, currentTimeZone());
    sun.setCurrentDate(Time.year(), Time.month(), Time.day());

    client.connect(g_mqttName.c_str());
    if (client.isConnected())
        Log.info("%s: MQTT client connected as %s", __PRETTY_FUNCTION__, g_mqttName.c_str());
    else
    {
        Log.error("%s: Unable to connect to MQTT", __PRETTY_FUNCTION__);
    }

    village.addBlock(&block1);
    village.addBlock(&block2);
    village.addBlock(&block3);

    lamps.addLamp(&lampD2);
    lamps.addLamp(&lampA0);
    lamps.addLamp(&lampA1);
    lamps.addLamp(&lampA2);
    lamps.addLamp(&lampA3);
    lamps.addLamp(&lampA4);
    lamps.addLamp(&lampA5);
    lamps.addLamp(&lampTX);
    lamps.addLamp(&lampRX);

    village.turnOff();
    station.turnOff();
    lamps.turnOff();

    Particle.variable("mpm", calculateMPM);
    Particle.variable("sunset", calcSunset);
    Particle.variable("sunrise", calcSunrise);
    Particle.variable("appid", g_appId);
    Particle.variable("timezone", currentTimeZone);
    Particle.variable("light", showLight);
    Particle.variable("distance", showDistance);
    Particle.function("turnon", turnOnBank);
    Particle.function("switch", turnOnSwitch);
    Particle.function("turnoff", turnOffBank);
    Particle.function("housecolor", setHouseColor);

    int sunrise = static_cast<int>(sun.calcSunrise());
    int sunset = static_cast<int>(sun.calcSunset());
    int mpm = Time.minute() + (Time.hour() * 60);

    JSONBufferWriter writer(g_mqttBuffer, sizeof(g_mqttBuffer) - 1);
    writer.beginObject();
    writer.name("village");
        writer.beginObject();
        writer.name("action").value("startup");
        writer.name("appid").value(g_appId);
        writer.name("state");
            writer.beginObject();
                writer.name("houses").value(g_lightsOn);
                writer.name("station").value(g_stationOn);
            writer.endObject();
        writer.name("timing");
            writer.beginObject();
                writer.name("sunrise").value(sunrise);
                writer.name("sunset").value(sunset);
                writer.name("now").value(mpm);
            writer.endObject();
        writer.endObject();
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    client.publish("village/startup", writer.buffer());
    station.turnOff();  // Do this twice, it doesn't seem to take the first time
    tofSensorInit();
    Log.info("%s: Setup done for app version %d, mpm=%d, sunrise=%d, sunset=%d", __PRETTY_FUNCTION__, g_appId, mpm, sunrise, sunset);
}

void loop() 
{
    static system_tick_t nextTimeout = 0;
    static int lastHour = 24;
    int currentHour = Time.hour();
    uint32_t mpm = (Time.hour() * 60) + Time.minute();

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

    EVERY_N_MILLIS(ONE_MINUTE) {
        sendMQTTHeartBeat();
    }

    if ((mpm >= 1380) && g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOffNextHouse(false);
        }
    }
    else if ((mpm < 1380) && afterSunset() && !g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOnNextHouse(false);
        }
    }
    else if (afterSunrise() && !afterSunset() && g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOffNextHouse(false);
        }
    }
    else if (!afterSunrise() && !g_lightsOn) {
        if (millis() > nextTimeout) {
            nextTimeout = turnOnNextHouse(false);
        }
    }

    if (afterSunset() && !g_stationOn) {
        turnOnNeighborhood();
    }
    if (afterSunrise() && !afterSunset() && g_stationOn) {
        turnOffNeighborhood();
    }

    checkTOFSensor();
}