#include "lamp.h"

Lamp::Lamp(int pin) : m_pin(pin)
{
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
}

Lamp::~Lamp()
{
    digitalWrite(m_pin, LOW);
}

void Lamp::turnOn()
{
    Log.info("Lamp: Turning on pin %d", m_pin);
    digitalWrite(m_pin, HIGH);
}

void Lamp::turnOff()
{
    Log.info("Lamp: Turning off pin %d", m_pin);
    digitalWrite(m_pin, LOW);
}

int Lamp::state()
{
    return digitalRead(m_pin);
}