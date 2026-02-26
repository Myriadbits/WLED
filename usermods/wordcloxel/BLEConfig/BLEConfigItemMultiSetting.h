#pragma once

#include "BLEConfigItemBase.h"

#define MULTISETTING_MAX_SETTINGS 16    

///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemMultiSetting : public BLEConfigItemBase
{
public:
    BLEConfigItemMultiSetting(uint16_t id)
        : BLEConfigItemBase(id, EConfigType::CT_MULTI)
    {        
    }

    uint8_t getVersion() { return m_version;}
    void setVersion(const uint8_t newValue) { m_version = newValue; }

    uint8_t getValue(int idx);
    void setValue(int idx, const uint8_t newValue);


protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);

protected:
    uint8_t        m_version {0};
    uint8_t        m_values[MULTISETTING_MAX_SETTINGS] {};
};
