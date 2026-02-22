#pragma once

#include "BLEConfigItemBase.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemUInt32 : public BLEConfigItemBase
{
public:
    BLEConfigItemUInt32(uint16_t id, EConfigType type, const char *pName)
        : BLEConfigItemBase(id, type, pName)
    {        
    }

    BLEConfigItemUInt32(uint16_t id, const char *pName)
        : BLEConfigItemBase(id, EConfigType::CT_UINT32, pName)
    {        
    }

    uint32_t getValue() { return m_value;}
    void setValue(const uint32_t newValue) { m_value = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);

protected:
    uint32_t        m_value;
};
