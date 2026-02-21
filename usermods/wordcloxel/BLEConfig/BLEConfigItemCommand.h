#pragma once

#include "BLEConfigItemOption.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemCommand : public BLEConfigItemOption
{
public:
    BLEConfigItemCommand(uint16_t id, const char *pName)
        : BLEConfigItemOption(id, EConfigType::CT_COMMAND, pName)
    {        
    }

protected:
    virtual void onStore(Preferences &preferences, char* pkey);

private:
};