#ifndef __LAMPS_H__
#define __LAMPS_H__

#include <vector>
#include <Particle.h>
#include "lamp.h"

class Lamps
{
public:
    Lamps();
    ~Lamps();

    void turnOn(int);
    void turnOn();
    void turnOff(int);
    void turnOff();
    void addLamp(Lamp*);
    void staggerOn();
    size_t size();

private:
    std::vector<Lamp*> m_lamps;
};
#endif