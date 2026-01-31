///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfig.h"
#include "BLEConfigItemCommand.h"

//
// Store this config item to the preferences
void BLEConfigItemCommand::onStore(Preferences &preferences, char* pkey)
{    
    // Do not store the command in the preferences, just keep it in mem
    m_lastCommandID = m_value;
}
