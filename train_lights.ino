#include <PubSubClient.h>

#include <sunset.h>

#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <Time.h>

#include <NTPClient.h>

#include <WiFiClient.h>
#include <WiFiMulti.h>
#include <WiFiGeneric.h>
#include <WiFiSTA.h>
#include <ETH.h>
#include <WiFiType.h>
#include <WiFiScan.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiAP.h>

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

#define SIGNAL_OFF_THRESHOLD    1500    // Wait 1500 ms to turn off the signals after the last hall effect event

#define IS_ALIVE            2
#define CROSSING_SIGNAL     26
#define CROSSING_SPEAKER    14
#define CROSSING_ON         25
#define CROSSING_OFF        7
#define RIGHTHAND_SWITCH    16
#define LEFTHAND_SWITCH     

const char *ssid = "Office";
const char *password = "Motorazr2V8";
const char *mqtt_server = "mqttserver";

const uint8_t _usDSTStart[22] = { 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9};
const uint8_t _usDSTEnd[22]   = { 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2};
int g_timeZone;
bool g_state;
bool g_signalsOn;
bool g_signalSpeaker;
bool g_enableSignalSpeaker;
bool g_sunsetTimerNotSet;
unsigned long g_lastOffEvent;

int g_circle[] = { 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 };
int g_overheadEvening[] = { 0, 1, 4, 7, 8, 11 };

Adafruit_NeoPixel station(14, 27, NEO_GRBW + NEO_KHZ800);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

SunSet sun;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

/*
typedef enum PINMAP {
    D2 = 26,
    D3 = 25,
    D4 = 7,
    D5 = 16,
    D6 = 27,
    D7 = 14,
    A0 = 2,
    A1 = 4,
    A2 = 36,
    A3 = 34,
    A4 = 38,
    A5 = 39,
}
*/

/*
 * Just blink the blue LED on and off to show we are alive
 */
void blink(void*)
{
    while(1) {
        digitalWrite(IS_ALIVE, HIGH);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        digitalWrite(IS_ALIVE, LOW);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
    Serial.print("Message arrived for topic ");
    Serial.println(topic);
}

void reconnect() 
{
    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (mqttClient.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            mqttClient.publish("outTopic", "hello world");
            // ... and resubscribe
            mqttClient.subscribe("inTopic");
        } 
        else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setTimeFromNetwork()
{
    timeClient.update();
    setTime(timeClient.getEpochTime());
    timeClient.setTimeOffset(currentTimeZone() * 3600);
    sun.setTZOffset(currentTimeZone());
    sun.setCurrentDate(year(), month(), day());
    Serial.println("Set time done");
}

/**
 * Since we must set the timer for when to turn on the lights when we boot
 * we set the time, then set the timer, then we loop forever, as we only
 * need to set the timer immediately on boot.
 */
void timeTask(void*)
{
    while (1) {
        setTimeFromNetwork();
        vTaskDelay(ONE_HOUR / portTICK_PERIOD_MS);
    }
}

int currentTimeZone()
{
    g_timeZone = DST_OFFSET;
    
    if (month() > 3 && month() < 11) {
        return DST_OFFSET;
    }
    if (month() == 3) {
        if ((day() == _usDSTStart[year() - TIME_BASE_YEAR]) && hour() >= 2)
            return DST_OFFSET;
        if (day() > _usDSTStart[year() - TIME_BASE_YEAR])
            return DST_OFFSET;
    }
    if (month() == 11) {
        if ((day() == _usDSTEnd[year() - TIME_BASE_YEAR]) && hour() <=2)
            return DST_OFFSET;
        if (day() < _usDSTEnd[year() - TIME_BASE_YEAR])
            return DST_OFFSET;
    }
    g_timeZone = CST_OFFSET;
    return CST_OFFSET;
}

void IRAM_ATTR enableSignals()
{
    digitalWrite(CROSSING_SIGNAL, HIGH);
    if (g_signalSpeaker)
        digitalWrite(CROSSING_SPEAKER, HIGH);
        
    g_signalsOn = true;
}

void IRAM_ATTR disableSignals()
{
    g_lastOffEvent = millis();
}

void mqttKeepAlive(void*)
{
    while (1) {
        if (!mqttClient.connected()) {
            reconnect();
        }
        mqttClient.loop();
        vTaskDelay(TWO_SECONDS / portTICK_PERIOD_MS);
    }
}

/*
 * Draws a light circle with the colors chosen
 */
void drawCircle(int r, int g, int b, int w)
{
    station.begin();
    
    for (int i = 0; i < 12; i++) {
        station.setPixelColor(g_circle[i], r, g, b, w);
    }
    station.show();
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
    vTaskDelay(400 / portTICK_PERIOD_MS);
    station.clear();
    station.show();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    drawOverheadLights(0, 0, 0, 200);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    station.clear();
    station.show();
    vTaskDelay(300 / portTICK_PERIOD_MS);
    drawOverheadLights(100, 0, 0, 150);
    vTaskDelay(150 / portTICK_PERIOD_MS);    
    station.clear();
    station.show();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    drawOverheadLights(0, 0, 0, 200);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    station.clear();
    station.show();
    vTaskDelay(900 / portTICK_PERIOD_MS);
    drawOverheadLights(0, 0, 0, 200);
    vTaskDelay(400 / portTICK_PERIOD_MS);
    station.clear();
    station.show();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    drawOverheadLights(0, 0, 100, 100);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    station.clear();
    station.show();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    drawOverheadLights(0, 0, 0, 255);
    station.show();
}

void turnOnStationLights()
{
    blinkToLife();
}

void turnOffStationLights()
{
    station.clear();
    station.show();    
}

void setup() 
{
    Serial.begin(112500);
    
    xTaskCreate(blink,"blink", 2048, NULL, 1, NULL);

    pinMode(IS_ALIVE, OUTPUT);              // A0
    pinMode(CROSSING_SIGNAL, OUTPUT);       // D2 Relay 1
    pinMode(CROSSING_SPEAKER, OUTPUT);      // D7 Relay 2
    pinMode(CROSSING_ON, INPUT_PULLUP);     // D3 Hall Effect
    pinMode(CROSSING_OFF, INPUT_PULLUP);    // D4 Hall Effect

    digitalWrite(CROSSING_SIGNAL, LOW);
    digitalWrite(CROSSING_SPEAKER, LOW);

    attachInterrupt(CROSSING_ON, enableSignals, FALLING);
    attachInterrupt(CROSSING_OFF, disableSignals, FALLING);
   
    g_state = false;
    g_signalSpeaker = false;

    sun.setPosition(LATITUDE, LONGITUDE, 0);
     
    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqttCallback);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    randomSeed(micros());

    timeClient.begin();
    timeClient.update();
    setTime(timeClient.getEpochTime());
    timeClient.setTimeOffset(currentTimeZone() * 3600);
    delay(2000);
    xTaskCreate(timeTask, "timetask", 2048, NULL, 1, NULL);
    xTaskCreate(mqttKeepAlive, "mqttKeepAlive", 2048, NULL, 1, NULL);
    station.begin();
    station.show(); // Initialize all pixels to 'off'
}

void loop() 
{
    static bool lightsOn = false;
    
    int mpm = hour() * 60 + minute();
    double sunrise = sun.calcSunrise();
    double sunset = sun.calcSunset();

    if (g_signalsOn) {
        if ((millis() - g_lastOffEvent) > SIGNAL_OFF_THRESHOLD) {
            digitalWrite(CROSSING_SPEAKER, LOW);
            digitalWrite(CROSSING_SIGNAL, LOW);
        }
    }

    if (mpm < sunrise - 15) {
        if (!lightsOn) {
            turnOnStationLights();
            lightsOn = true;
        }
    }
    else if (mpm > sunset - 20) {
        if (!lightsOn) {
            turnOnStationLights();
            lightsOn = true;
        }
    }
    else {
        if (lightsOn) {
            station.begin();
            station.show();
            lightsOn = false;
        }
    }
}
