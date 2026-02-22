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



