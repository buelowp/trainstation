#ifndef __STATION_H__
#define __STATION_H__

#include <neopixel.h>
#include <MQTT.h>

class Station
{
public:
    Station(int, int, MQTT*);
    ~Station();

    void turnOnStandard();
    void turnOnStandardColor(int, int, int, int);
    void turnOnAllColor(int, int, int, int);
    void turnOn();
    void turnOnCircle();
    void turnOnCircleColor(int, int, int, int);
    void blinkToLife();
    void turnOff();

private:
    Adafruit_NeoPixel *m_station;
    int m_leds;
    int m_pin;
    MQTT *m_mqtt;
};
#endif