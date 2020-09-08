#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <vector>
#include <map>
#include "houses.h"

class Block {
public:
    Block();
    ~Block() {}

    void addHouses(Houses *bank);
    bool turnOffRandomHouse();
    bool turnOnRandomHouse();
    bool turnOffHouseByIndex(int, int);
    bool turnOnHouseByIndex(int, int);
    void turnOnBlock(int);
    void turnOffBlock(int);
    void turnOff();
    bool stateOfHouseByIndex(int, int);
    int totalHousesControlled() { return m_totalHouses; }

private:
    bool hasBeenTested(int, int);
    void resetTestedValues();

    std::vector<Houses*> m_banks;
    int m_totalHouses;
    int m_testedValues[18];
};
#endif