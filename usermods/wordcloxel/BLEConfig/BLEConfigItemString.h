#pragma once

#include "BLEConfigItemBase.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemString : public BLEConfigItemBase
{
public:
    BLEConfigItemString(uint16_t id, const std::string name, const std::string defaultValue, bool secure = true)
        : BLEConfigItemBase(id, EConfigType::CT_STRING, name, secure)
        , m_valueStringDefault(defaultValue)
    {        
    }

    // When value fits is a string
    std::string getValue();
    void setValue(const std::string newValue) { m_valueString = newValue; }
    void setDefaultValue(const std::string newValue) { m_valueStringDefault = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);
    virtual void onLoad(Preferences &preferences, char* pkey);
    virtual void onStore(Preferences &preferences, char* pkey);
    virtual std::string valueToString() { return m_valueString; }

private:
    std::string     m_valueString;
    std::string     m_valueStringDefault;
};
