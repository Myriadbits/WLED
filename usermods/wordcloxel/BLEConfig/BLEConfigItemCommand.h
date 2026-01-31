#pragma once

#include "BLEConfigItemOption.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemCommand : public BLEConfigItemOption
{
public:
    BLEConfigItemCommand(uint16_t id, const std::string name, bool secure = true)
        : BLEConfigItemOption(id, EConfigType::CT_COMMAND, name, 0, secure)
        , m_lastCommandID(0)
    {        
    }

    uint8_t getLastCommandID() { return m_lastCommandID; }

protected:
    virtual void onStore(Preferences &preferences, char* pkey);

private:
    uint8_t m_lastCommandID;
};