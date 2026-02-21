#pragma once

#include "BLEConfigItemBase.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemUInt8 : public BLEConfigItemBase
{
public:
    BLEConfigItemUInt8(uint16_t id, EConfigType type, const char *pName)
        : BLEConfigItemBase(id, type, pName)
    {        
    }

    BLEConfigItemUInt8(uint16_t id, const char *pName)
        : BLEConfigItemBase(id, EConfigType::CT_UINT8, pName)
    {        
    }

    uint8_t getValue() { return m_value;}
    void setValue(const uint8_t newValue) { m_value = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);
    virtual void onLoad();
    virtual void onStore();
    virtual std::string valueToString();

protected:
    uint8_t        m_value;
};
