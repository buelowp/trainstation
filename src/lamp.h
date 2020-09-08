#ifndef __LAMP_H__
#define __LAMP_H__

#include <Particle.h>

class Lamp
{
public:
    Lamp(int pin);
    ~Lamp();

    void turnOn();
    void turnOff();
    int state();

private:
    int m_pin;
};
#endif