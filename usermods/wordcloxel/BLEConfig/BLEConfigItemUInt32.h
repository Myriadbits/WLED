#pragma once

#include "BLEConfigItemBase.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemUInt32 : public BLEConfigItemBase
{
public:
    BLEConfigItemUInt32(uint16_t id, EConfigType type, const std::string name, uint32_t defaultValue, bool secure = true)
        : BLEConfigItemBase(id, type, name, secure)
        , m_valueDefault(defaultValue)
    {        
    }

    BLEConfigItemUInt32(uint16_t id, const std::string name, uint32_t defaultValue, bool secure = true)
        : BLEConfigItemBase(id, EConfigType::CT_UINT32, name, secure)
        , m_valueDefault(defaultValue)
    {        
    }

    uint32_t getValue() { return m_value;}
    void setValue(const uint32_t newValue) { m_value = newValue; }
    void setDefaultValue(const uint32_t newValue) { m_valueDefault = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);
    virtual void onLoad();
    virtual void onStore();
    virtual std::string valueToString();

protected:
    uint32_t        m_value;
    uint32_t        m_valueDefault;
};
