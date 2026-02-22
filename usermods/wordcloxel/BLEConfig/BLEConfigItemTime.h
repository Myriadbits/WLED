#pragma once

#include "BLEConfigItemUInt32.h"


///////////////////////////////////////////////////////////////////////////////
// Single Configuration item to control the time
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemTime : public BLEConfigItemUInt32
{
public:
    BLEConfigItemTime(uint16_t id, const char *pName)
        : BLEConfigItemUInt32(id, EConfigType::CT_TIME, pName)
    {        
    }

    int getHour() { return (m_value >> 16) & 0xFF;}
    int getMinute() { return (m_value >> 8) & 0xFF;}
    int getSecond() { return (m_value & 0xFF);}
};
