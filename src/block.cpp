#include "block.h"

Block::Block() : m_totalHouses(0)
{
}

void Block::addHouses(Houses *bank)
{
    m_banks.push_back(bank);
    m_totalHouses += bank->numHouses();
    Log.info("Added %d houses to the block, total houses %d", bank->numHouses(), m_totalHouses);
}

/**
 * Returns true if we found something to turn off, false otherwise
 */
bool Block::turnOnRandomHouse()
{
    bool found = false;
    int count = m_totalHouses;

    while (!found && (count-- > 0)) {
        int bank = random(0, m_banks.size());
        int house = random(0, m_banks[bank]->numHouses());
        Log.info("Turning off house %d:%d", bank, house);
        if (!m_banks[bank]->isOn(house)) {
            Log.info("Turning on house %d:%d", bank, house);
            m_banks[bank]->turnOn(house);
            found = true;
        }
        else {
            Log.info("House %d:%d is already on, remaining=%d", bank, house, count);
        }
    }

    Log.info("Returning %d as we attempted to turn on a random house", found);
    return found;
}

bool Block::turnOffRandomHouse()
{
    bool found = false;
    int count = m_totalHouses;

    while (!found && (count-- > 0)) {
        int bank = random(0, m_banks.size());
        int house = random(0, m_banks[bank]->numHouses());
        if (m_banks[bank]->isOn(house)) {
            Log.info("Turning off house %d:%d", bank, house);
            m_banks[bank]->turnOff(house);
            found = true;
        }
        else {
            Log.info("House %d:%d is already off, remaining=%d", bank, house, count);
        }
    }
    Log.info("Returning %d as we attempted to turn off a random house", found);
    return found;
}

bool Block::stateOfHouseByIndex(int bank, int house)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    if (t < m_banks.size()) {
        Log.info("House state for %d:%d is %d", bank, house, m_banks[bank]->isOn(house));
        return m_banks[bank]->isOn(house);
    }
    Log.error("%s: Invalid bank %d", __FUNCTION__, bank);
    return false;
}

bool Block::turnOnHouseByIndex(int bank, int house)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    if (t < m_banks.size()) {
        Log.info("Turning on %d:%d", bank, house);
        return m_banks[bank]->turnOn(house);
    }
    Log.error("%s: Invalid bank %d", __FUNCTION__, bank);
    return false;
}

bool Block::turnOffHouseByIndex(int bank, int house)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    if (t < m_banks.size()) {
        Log.info("Turning off %d:%d", bank, house);
        return m_banks[bank]->turnOff(house);
    }
    Log.error("%s: Invalid bank %d", __FUNCTION__, bank);
    return false;
}

void Block::turnOnBlock(int bank)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);
    Log.info("Turning on entire block");
    if (t < m_banks.size())
        m_banks[bank]->turnOn();
}

void Block::turnOffBlock(int bank)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    Log.info("Turning off entire block");
    if (t < m_banks.size())
        m_banks[bank]->turnOff();
}
