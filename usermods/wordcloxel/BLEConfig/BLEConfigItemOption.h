#pragma once

#include <vector>
#include "BLEConfigItemUInt8.h"

typedef struct 
{
    uint8_t         m_value; // The value of the config option
    std::string     m_sName; // Name of this option item
} MIOTConfigOption_t;


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemOption : public BLEConfigItemBase
{
public:
    BLEConfigItemOption(uint16_t id, EConfigType type, const char *pName)
        : BLEConfigItemBase { id, type, pName }
        , m_value { 0 }
    {        
    }

    BLEConfigItemOption(uint16_t id, const char *pName)
        : BLEConfigItemBase{ id, EConfigType::CT_OPTION, pName}
        , m_value { 0 }
    {        
    }

    void clearOptions();
    void addOption(const uint8_t optionValue, const std::string optionText);
    MIOTConfigOption_t* getOption(const uint8_t optionValue);
    bool isOptionPresent(const std::string optionText);
    MIOTConfigOption_t* getOptionByIndex(const uint8_t index);

    uint8_t getValue() { return m_value;}
    void setValue(const uint8_t newValue) { m_value = newValue; }

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);

protected:
    uint8_t                         m_value;
    std::vector<MIOTConfigOption_t> m_vecOptions;
};
