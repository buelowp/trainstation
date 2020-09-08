#include "lamps.h"

Lamps::Lamps()
{
}

Lamps::~Lamps()
{
}

void Lamps::addLamp(Lamp *lamp)
{
    m_lamps.push_back(lamp);
}

size_t Lamps::size()
{
    return m_lamps.size();
}

void Lamps::turnOn(int which)
{
    Log.info("Lamps: Turning on street lamp %d", which);
    std::vector<Lamp*>::size_type t = static_cast<std::vector<Lamp*>::size_type>(which);

    if (t < m_lamps.size())
        m_lamps[which]->turnOn();
}

void Lamps::turnOff(int which)
{
    Log.info("Lamps: Turning off street lamp %d", which);
    std::vector<Lamp*>::size_type t = static_cast<std::vector<Lamp*>::size_type>(which);

    if (t < m_lamps.size())
        m_lamps[which]->turnOff();
}

void Lamps::turnOn()
{
    Log.info("Lamps: Turning all street lamps on");
    for (size_t i = 0; i < m_lamps.size(); i++) {
        m_lamps[i]->turnOn();
    }
}

void Lamps::turnOff()
{
    Log.info("Lamps: Turning all street lamps off");
    for (size_t i = 0; i < m_lamps.size(); i++) {
        m_lamps[i]->turnOff();
    }
}

void Lamps::staggerOn()
{
    Log.info("Lamps: Staggering street lamps on");
    for (size_t i = 0; i < m_lamps.size(); i++) {
        m_lamps[i]->turnOn();
        delay(random(1500, 2550));
    }
}