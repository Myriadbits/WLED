#pragma once

#include "BLEConfigItemUInt32.h"


///////////////////////////////////////////////////////////////////////////////
// Single Configuration item to control the time
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemTime : public BLEConfigItemUInt32
{
public:
    BLEConfigItemTime(uint16_t id, const std::string name, uint8_t defaultHour, uint8_t defaultMinute, uint8_t defaultSecond, bool secure = true)
        : BLEConfigItemUInt32(id, EConfigType::CT_TIME, name, (((uint32_t) defaultHour) << 16) | (((uint32_t) defaultMinute) << 8) | ((uint32_t) defaultSecond), secure)
    {        
    }

    int getHour() { return (m_value >> 16) & 0xFF;}
    int getMinute() { return (m_value >> 8) & 0xFF;}
    int getSecond() { return (m_value & 0xFF);}
};
