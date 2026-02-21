#pragma once

#include "BLEConfigItemUInt32.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemDate : public BLEConfigItemUInt32
{
public:
    BLEConfigItemDate(uint16_t id, const char *pName)
        : BLEConfigItemUInt32(id, EConfigType::CT_DATE, pName)
    {        
    }

    int getDay() { return (m_value >> 24) & 0xFF;}
    int getMonth() { return (m_value >> 16) & 0xFF;}
    int getYear() { return (m_value & 0xFFFF);}
};
