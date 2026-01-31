///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemUInt32.h"

//
// Encode this config string item into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item of this string
int BLEConfigItemUInt32::onEncodeData(uint8_t *pdata, int dataLen, int idx)
{
    pdata[idx++] = (uint8_t)((m_value & 0xFF000000) >> 24);
    pdata[idx++] = (uint8_t)((m_value & 0x00FF0000) >> 16);
    pdata[idx++] = (uint8_t)((m_value & 0x0000FF00) >> 8);
    pdata[idx++] = (uint8_t)((m_value & 0x000000FF) >> 0);
    return idx;
}

//
// Decode an incoming config item
// data: string containing the data
void BLEConfigItemUInt32::onDecodeData(std::string data)
{
    m_value = (uint32_t)(data[0] << 24) | (uint32_t)(data[1] << 16) | (uint32_t)(data[2] << 8) | (uint32_t)(data[3]);
}

//
// Load this config item from the preferences
void BLEConfigItemUInt32::onLoad(Preferences &preferences, char* pkey)
{
    m_value = preferences.getUInt(pkey, m_valueDefault);   
    BLECONFIG_LOG("Loading key '%s' (%s) with value: %d", this->getName().c_str(), pkey, m_value);

}

//
// Store this config item to the preferences
void BLEConfigItemUInt32::onStore(Preferences &preferences, char* pkey)
{
    preferences.putUInt(pkey, m_value);
    BLECONFIG_LOG("Storing key '%s' (%s) with value: %d", this->getName().c_str(), pkey, m_value);
}

//
// Return the value as a hex string (for debugging)
std::string BLEConfigItemUInt32::valueToString()
{
    char stemp[32];
    snprintf(stemp, 32, "%08X", m_value);
    return std::string(stemp);
}
