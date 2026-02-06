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
class BLEConfigItemOption : public BLEConfigItemUInt8
{
public:
    BLEConfigItemOption(uint16_t id, EConfigType type, const std::string name, uint8_t defaultValue, bool secure = true)
        : BLEConfigItemUInt8(id, type, name, defaultValue, secure)
    {        
    }

    BLEConfigItemOption(uint16_t id, const std::string name, uint8_t defaultValue, bool secure = true)
        : BLEConfigItemUInt8(id, EConfigType::CT_OPTION, name, defaultValue, secure)
    {        
    }

    void clearOptions();
    void addOption(const uint8_t optionValue, const std::string optionText);
    MIOTConfigOption_t* getOption(const uint8_t optionValue);
    bool isOptionPresent(const std::string optionText);
    MIOTConfigOption_t* getOptionByIndex(const uint8_t index);

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    int addOptionData(uint8_t *pdata, int dataLen, int idx);

protected:
    std::vector<MIOTConfigOption_t> m_vecOptions;
};
