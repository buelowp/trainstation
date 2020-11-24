#include "village.h"

Village::Village() : m_totalHouses(0), m_lastRandomHouse(-1), m_lastRandomBlock(-1)
{
}

Village::~Village()
{
    turnOff();
}

void Village::addBlock(Houses *block)
{
    int index = m_blocks.size();
    m_blocks.insert(std::pair<int, Houses*>(index, block));
    m_totalHouses += block->numHouses();
    Log.info("Block: Added %d houses to the block, total houses %d", block->numHouses(), m_totalHouses);
}

void Village::turnOff()
{
    for (auto it : m_blocks) {
        it.second->turnOff();
    }
}

/**
 * Returns true if we found something to turn off, false otherwise
 */
bool Village::turnOnRandomHouse()
{
    int index = random(0, m_blocks.size());     /* Start somewhere */
    Houses *block = m_blocks[index];
    int house = random(0, block->numHouses());

    if (!block->isOn(house)) {
        Log.info("%s:%d: Block: Turning on block:house %d:%d", __FUNCTION__, __LINE__, index, house);
        block->turnOn(house);
        m_lastRandomBlock = index;
        m_lastRandomHouse = house;
        return true;
    }
    else {
        /*
         * If we can't find a random house, to avoid getting too complicated, just start from the beginning.
         * This will have a pseudo random effect as many random houses will be chosen, even if not all
         */
        for (auto it : m_blocks) {
            for (int i = 0; i < it.second->numHouses(); i++) {
                if (!it.second->isOn(i)) {
                    Log.info("%s:%d: Block: Turning on block:house %d:%d", __FUNCTION__, __LINE__, it.first, i);
                    it.second->turnOn(i);
                    m_lastRandomBlock = index;
                    m_lastRandomHouse = house;
                    return true;
                }
            }
        }
    }
    Log.info("Block: All houses appear to be on");
    return false;
}

/**
 * Returns true if we found something to turn off, false otherwise
 */
bool Village::turnOnRandomHouseWithRandomColor()
{
    int index = random(0, m_blocks.size());     /* Start somewhere */
    Houses *block = m_blocks[index];
    int house = random(0, block->numHouses());
    static int color = 0;
    int bright = random(70, 100);

    if (!block->isOn(house)) {
        Log.info("%s:%d: Block: Turning on block:house %d:%d to color %ld", __FUNCTION__, __LINE__, index, house, Color[color]);
        block->turnOn(house, Color[color++], bright);
        m_lastRandomBlock = index;
        m_lastRandomHouse = house;
        if (color == 4)
            color = 0;
        return true;
    }
    else {
        /*
         * If we can't find a random house, to avoid getting too complicated, just start from the beginning.
         * This will have a pseudo random effect as many random houses will be chosen, even if not all
         */
        for (auto it : m_blocks) {
            for (int i = 0; i < it.second->numHouses(); i++) {
                if (!it.second->isOn(i)) {
                    Log.info("%s:%d: Block: Turning on block:house %d:%d to color %ld", __FUNCTION__, __LINE__, it.first, i, Color[color]);
                    it.second->turnOn(i, Color[color++], bright);
                    m_lastRandomBlock = index;
                    m_lastRandomHouse = house;
                    if (color == 4)
                        color = 0;
                    return true;
                }
            }
        }
    }
    Log.info("Block: All houses appear to be on");
    color = 0;
    return false;
}

bool Village::turnOffRandomHouse()
{
    int index = random(0, m_blocks.size());     /* Start somewhere */
    Houses *block = m_blocks[index];
    int house = random(0, block->numHouses());

    if (block->isOn(house)) {
        Log.info("%s:%d: Block: Turning off block:house %d:%d", __FUNCTION__, __LINE__, index, house);
        block->turnOff(house);
        m_lastRandomBlock = index;
        m_lastRandomHouse = house;
        return true;
    }
    else {
        /*
         * If we can't find a random house, to avoid getting too complicated, just start from the beginning.
         * This will have a pseudo random effect as many random houses will be chosen, even if not all
         */
        for (auto it : m_blocks) {
            for (int i = 0; i < it.second->numHouses(); i++) {
                if (it.second->isOn(i)) {
                    Log.info("%s:%d: Block: Turning on block:house %d:%d", __FUNCTION__, __LINE__, it.first, i);
                    it.second->turnOff(i);
                    m_lastRandomBlock = index;
                    m_lastRandomHouse = house;
                    return true;
                }
            }
        }
    }
    Log.info("Block: All houses appear to be on");
    return false;
}

bool Village::stateOfHouseByIndex(int block, int house)
{
    if (static_cast<std::map<int, Houses*>::size_type>(block) < m_blocks.size()) {
        Log.info("Block: House state for %d:%d is %d", block, house, m_blocks[block]->isOn(house));
        return m_blocks[block]->isOn(house);
    }
    Log.error("%s: Invalid block %d", __FUNCTION__, block);
    return false;
}

bool Village::turnOnHouseByIndex(int block, int house)
{
    if (static_cast<std::map<int, Houses*>::size_type>(block) < m_blocks.size()) {
        m_blocks[block]->turnOn(house);
        Log.info("Block: Turning on %d:%d", block, house);
        return true;
    }

    Log.error("%s: Invalid block %d", __FUNCTION__, block);
    return false;
}

bool Village::turnOffHouseByIndex(int block, int house)
{
    if (static_cast<std::map<int, Houses*>::size_type>(block) < m_blocks.size()) {
        m_blocks[block]->turnOff(house);
        Log.info("Block: Turning on %d:%d", block, house);
        return true;
    }

    Log.error("%s: Invalid block %d", __FUNCTION__, block);
    return false;
}

void Village::turnOnBlock(int block)
{
    if (static_cast<std::map<int, Houses*>::size_type>(block) < m_blocks.size()) {
        Log.info("Block: Turning on entire block");
        m_blocks[block]->turnOn();
    }
}

void Village::turnOffBlock(int block)
{
    if (static_cast<std::map<int, Houses*>::size_type>(block) < m_blocks.size()) {
        Log.info("Block: Turning off entire block");
        m_blocks[block]->turnOff();
    }
}

void Village::setHouseColors(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    Log.info("Block: Setting block colors to %d:%d:%d", r, g, b);
    for (auto it : m_blocks) {
        if (it.second->isRGBWCapable())
            it.second->setColors(r, g, b, w);
        else
            it.second->setColors(r, g, b, 0);
        
    }
}
