#include "block.h"

Block::Block() : m_totalHouses(0)
{
    size_t n = sizeof(m_testedValues) / sizeof(m_testedValues[0]);
    Log.info("Block: m_testedValues size is %d", n);
}

void Block::addHouses(Houses *bank)
{
    m_banks.push_back(bank);
    m_totalHouses += bank->numHouses();        
    Log.info("Block: Added %d houses to the block, total houses %d", bank->numHouses(), m_totalHouses);
}

void Block::turnOff()
{
    FastLED.clear();
    FastLED.show();
}

void Block::resetTestedValues()
{
    size_t n = sizeof(m_testedValues) / sizeof(m_testedValues[0]);
    memset(&m_testedValues, 0, n);
}

bool Block::hasBeenTested(int bank, int house)
{
    int index = ((bank * 6) + house);
    Log.info("Block: Testing for %d:%d which is index %d", bank, house, index);
    if (m_testedValues[index] == 0) {
        m_testedValues[index] = 1;
        return false;
    }
    return true;
}
/**
 * Returns true if we found something to turn off, false otherwise
 */
bool Block::turnOnRandomHouse()
{
    bool found = false;
    int count = m_totalHouses;

    while (!found && (count > 0)) {
        resetTestedValues();
        int bank = random(0, m_banks.size());
        int house = random(0, m_banks[bank]->numHouses());
        if (hasBeenTested(bank, house)) {
            continue;
        }
        else {
            count--;
        }
        if (!m_banks[bank]->isOn(house)) {
            Log.info("Block: Turning on house %d:%d", bank, house);
            m_banks[bank]->turnOn(house);
            found = true;
        }
        else {
            Log.info("Block: House %d:%d is already on, remaining=%d", bank, house, count);
        }
    }

    if (count == 0) {
        resetTestedValues();
    }
    Log.info("Block: Returning %d as we attempted to turn on a random house", found);
    return found;
}

bool Block::turnOffRandomHouse()
{
    bool found = false;
    int count = m_totalHouses;

    while (!found && (count > 0)) {
        resetTestedValues();
        int bank = random(0, m_banks.size());
        int house = random(0, m_banks[bank]->numHouses());
        if (hasBeenTested(bank, house)) {
            continue;
        }
        else {
            count--;
        }
        if (m_banks[bank]->isOn(house)) {
            Log.info("Block: Turning off house %d:%d", bank, house);
            m_banks[bank]->turnOff(house);
            found = true;
        }
        else {
            Log.info("Block: House %d:%d is already off, remaining=%d", bank, house, count);
        }
    }
    if (count == 0) {
        resetTestedValues();
    }
    Log.info("Block: Returning %d as we attempted to turn off a random house", found);
    return found;
}

bool Block::stateOfHouseByIndex(int bank, int house)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    if (t < m_banks.size()) {
        Log.info("Block: House state for %d:%d is %d", bank, house, m_banks[bank]->isOn(house));
        return m_banks[bank]->isOn(house);
    }
    Log.error("%s: Invalid bank %d", __FUNCTION__, bank);
    return false;
}

bool Block::turnOnHouseByIndex(int bank, int house)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    if (t < m_banks.size()) {
        Log.info("Block: Turning on %d:%d", bank, house);
        return m_banks[bank]->turnOn(house);
    }
    Log.error("%s: Invalid bank %d", __FUNCTION__, bank);
    return false;
}

bool Block::turnOffHouseByIndex(int bank, int house)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    if (t < m_banks.size()) {
        Log.info("Block: Turning off %d:%d", bank, house);
        return m_banks[bank]->turnOff(house);
    }
    Log.error("%s: Invalid bank %d", __FUNCTION__, bank);
    return false;
}

void Block::turnOnBlock(int bank)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);
    Log.info("Block: Turning on entire block");
    if (t < m_banks.size())
        m_banks[bank]->turnOn();
}

void Block::turnOffBlock(int bank)
{
    std::vector<Houses*>::size_type t = static_cast<std::vector<Houses*>::size_type>(bank);

    Log.info("Block: Turning off entire block");
    if (t < m_banks.size())
        m_banks[bank]->turnOff();
}
