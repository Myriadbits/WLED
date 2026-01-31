#pragma once

#include "BLEConfigItemUInt32.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemDate : public BLEConfigItemUInt32
{
public:
    BLEConfigItemDate(uint16_t id, const std::string name, uint16_t defaultYear, uint8_t defaultMonth, uint8_t defaultDay, bool secure = true)
        : BLEConfigItemUInt32(id, EConfigType::CT_DATE, name, (((uint32_t) defaultDay) << 24) | (((uint32_t) defaultMonth) << 16) | ((uint32_t) defaultYear), secure)
    {        
    }

    int getDay() { return (m_value >> 24) & 0xFF;}
    int getMonth() { return (m_value >> 16) & 0xFF;}
    int getYear() { return (m_value & 0xFFFF);}
};
