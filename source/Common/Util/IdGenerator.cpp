/*
 * IdGenerator.cpp
 *
 *  Created on: 9 Apr 2020
 *      Author: wenhe
 */
#include <atomic>
#include <random>


#include "Common.h"

using namespace Util;

IdGenerator* IdGenerator::getInstance()
{
    static IdGenerator instance;
    return &instance;
}

void IdGenerator::setIndex(uint8_t index)
{
    _index = index;
}

uint64_t IdGenerator::get() // 10W = 35419us(with clock)
{
    static std::atomic_uint16_t seq(0);
    union IdUnion
    {
        IdUnion() :
                as64bit(0LL)
        {
        }
        uint64_t as64bit;
        uint32_t as32bit[2];
        uint16_t as16bit[4];
        uint8_t as8bit[8];
    } id;

    // hight               low
    // 77-66-55-44-33-22-11-00
    // little:5  4  3  2  1  0
    // big:   2  3  4  5  6  7

    if(Util::isLittleEndian())
    {
        id.as32bit[1] = ((uint32_t) time(NULL)) & 0xFFFFFFFF;
        id.as16bit[1] = seq++;
        id.as8bit[1] = clock() & 0xFF;
        id.as8bit[0] = _index;
    }
    else
    {
        id.as32bit[0] = ((uint32_t) time(NULL)) & 0xFFFFFFFF;
        id.as16bit[2] = seq++;
        id.as8bit[6] = clock() & 0xFF;
        id.as8bit[7] = _index;
    }

    return id.as64bit;
}
