#pragma once

#include "BLEConfigItemBase.h"

#define MULTISETTING_MAX_SETTINGS 32    

///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemMultiSetting : public BLEConfigItemBase
{
public:
    BLEConfigItemMultiSetting(uint16_t id)
        : BLEConfigItemBase(id)
    {        
    }

    uint8_t getValue(int idx);
    uint8_t* getData();
    void setValue(int idx, const uint8_t newValue);
    void setData(uint8_t* pNewData, uint8_t length);

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);

protected:
    uint8_t        m_values[MULTISETTING_MAX_SETTINGS] {};
};
