#pragma once

#include "BLEConfigItemBase.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemString : public BLEConfigItemBase
{
public:
    BLEConfigItemString(uint16_t id, const char *pName)
        : BLEConfigItemBase(id, EConfigType::CT_STRING, pName)
    {        
    }

    // When value fits is a string
    std::string getValue();
    void setValue(const std::string newValue) { m_valueString = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);
    virtual void onLoad();
    virtual void onStore();
    virtual std::string valueToString() { return m_valueString; }

private:
    std::string     m_valueString;
};
