#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <vector>
#include <map>
#include <Particle.h>
#include "houses.h"

class Village {
public:
    Village();
    ~Village();

    void addBlock(Houses *block);
    bool turnOffRandomHouse();
    bool turnOnRandomHouse();
    bool turnOnRandomHouseWithRandomColor();
    bool turnOffHouseByIndex(int, int);
    bool turnOnHouseByIndex(int, int);
    void turnOnBlock(int);
    void turnOffBlock(int);
    void turnOff();
    bool stateOfHouseByIndex(int, int);
    int totalHousesControlled() { return m_totalHouses; }
    int getLastRandomBlock() { return m_lastRandomBlock; }
    int getLastRandomHouse() { return m_lastRandomHouse; }
    void setHouseColors(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

private:
    std::map<int, Houses*> m_blocks;
    int m_totalHouses;
    int m_lastRandomHouse;
    int m_lastRandomBlock;
};
#endif