///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemUInt8.h"

//
// Encode this config string item into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item of this string
int BLEConfigItemUInt8::onEncodeData(uint8_t *pdata, int dataLen, int idx)
{
    pdata[idx++] = (uint8_t)(m_value & 0xFF);
    return idx;
}

//
// Decode an incoming config item
// data: string containing the data
void BLEConfigItemUInt8::onDecodeData(std::string data)
{
    m_value = (uint8_t) data[0];
}

//
// Load this config item from the preferences
void BLEConfigItemUInt8::onLoad(Preferences &preferences, char* pkey)
{
    m_value = preferences.getUChar(pkey, m_valueDefault);    
    BLECONFIG_LOG("Loading key '%s' (%s) with value: %d", this->getName().c_str(), pkey, m_value);
}

//
// Store this config item to the preferences
void BLEConfigItemUInt8::onStore(Preferences &preferences, char* pkey)
{
    preferences.putUChar(pkey, m_value);
    BLECONFIG_LOG("Storing key '%s' (%s) with value: %d", this->getName().c_str(), pkey, m_value);
}

//
// Return the value as a hex string (for debugging)
std::string BLEConfigItemUInt8::valueToString()
{
    char stemp[8];
    snprintf(stemp, 8, "%02X", m_value);
    return std::string(stemp);
}
