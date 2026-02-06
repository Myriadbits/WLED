#pragma once

#include "BLEConfigItemBase.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemUInt8 : public BLEConfigItemBase
{
public:
    BLEConfigItemUInt8(uint16_t id, EConfigType type, const std::string name, uint8_t defaultValue, bool secure = true)
        : BLEConfigItemBase(id, type, name, secure)
        , m_valueDefault(defaultValue)
    {        
    }

    BLEConfigItemUInt8(uint16_t id, const std::string name, uint8_t defaultValue, bool secure = true)
        : BLEConfigItemBase(id, EConfigType::CT_UINT8, name, secure)
        , m_valueDefault(defaultValue)
    {        
    }

    uint8_t getValue() { return m_value;}
    void setValue(const uint8_t newValue) { m_value = newValue; }
    void setDefaultValue(const uint8_t newValue) { m_valueDefault = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);
    virtual void onLoad();
    virtual void onStore();
    virtual std::string valueToString();

protected:
    uint8_t        m_value;
    uint8_t        m_valueDefault;
};
