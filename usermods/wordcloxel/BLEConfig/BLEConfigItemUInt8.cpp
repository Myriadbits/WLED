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
    pdata[idx++] = m_maxValue;
    return idx;
}

//
// Decode an incoming config item
// data: string containing the data
void BLEConfigItemUInt8::onDecodeData(std::string data)
{
    m_value = (uint8_t) data[0];
}

