///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemString.h"

//
// Returns the current value as a string
std::string BLEConfigItemString::getValue() 
{
    return m_valueString;
}

//
// Encode this config string item into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item of this string
int BLEConfigItemString::onEncodeData(uint8_t *pdata, int dataLen, int idx)
{
    uint8_t slen = (uint8_t) std::min((int) m_valueString.length(), 0xF0);
    pdata[idx++] = slen;
    for(uint8_t i = 0; i < slen; i++)
        pdata[idx++] = m_valueString.c_str()[i];
    return idx;
}

//
// Decode an incoming config item
// data: string containing the data
void BLEConfigItemString::onDecodeData(std::string data)
{
    m_valueString = data; 
}

//
// Load this config item from the preferences
void BLEConfigItemString::onLoad(Preferences &preferences, char* pkey)
{
    m_valueString = std::string(preferences.getString(pkey, m_valueStringDefault.c_str()).c_str());    
    BLECONFIG_LOG("Loading key '%s' (%s) with value: %s", this->getName().c_str(), pkey, m_valueString.c_str());
}

//
// Store this config item to the preferences
void BLEConfigItemString::onStore(Preferences &preferences, char* pkey)
{
    preferences.putString(pkey, m_valueString.c_str());
    BLECONFIG_LOG("Storing key '%s' (%s) with value: %s", this->getName().c_str(), pkey, m_valueString.c_str());
}


