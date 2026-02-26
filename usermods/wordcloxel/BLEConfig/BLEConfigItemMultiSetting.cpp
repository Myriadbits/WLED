///////////////////////////////////////////////////////////////////////////////
// BLEConfigItemMultiSetting class
// Class for storing multiple settings in a single characteristic
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemMultiSetting.h"

//
// Encode all multi settings into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item
int BLEConfigItemMultiSetting::onEncodeData(uint8_t *pdata, int dataLen, int idx)
{
    pdata[idx++] = m_version;
    for(int i = 0; i < MULTISETTING_MAX_SETTINGS; i++)
    {
        pdata[idx++] = m_values[i];
    }   
    return idx;
}

//
// Decode all incoming config item
void BLEConfigItemMultiSetting::onDecodeData(std::string data)
{
    m_version = (uint8_t) data[0];
    for(int i = 0; i < MULTISETTING_MAX_SETTINGS; i++)
    {
        m_values[i] = (uint8_t) data[i + 1];
    }
}

uint8_t BLEConfigItemMultiSetting::getValue(int idx)
{
    if (idx < 0 || idx >= MULTISETTING_MAX_SETTINGS)
        return 0;
    return m_values[idx];
}

void BLEConfigItemMultiSetting::setValue(int idx, const uint8_t newValue)
{
    if (idx < 0 || idx >= MULTISETTING_MAX_SETTINGS)
        return;
    m_values[idx] = newValue;
}
