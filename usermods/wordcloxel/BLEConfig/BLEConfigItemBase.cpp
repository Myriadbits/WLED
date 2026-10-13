///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemBase.h"

//
// Constructor, create/fill this config item
BLEConfigItemBase::BLEConfigItemBase(uint16_t id)
    : m_id(id)
{
}

//
// Encode this config item into a byte array/buffer
// pdata: pointer to the buffer
// dataLen: length of the buffer
// Note that the buffer should (for now) hold at least 256 bytes
int BLEConfigItemBase::encode(uint8_t *pdata, int dataLen)
{
    // Add the data
    return onEncodeData(pdata, dataLen, 0);
}

//
// Decode an incoming config item
// data: string containing the data
bool BLEConfigItemBase::decode(std::string data)
{
    onDecodeData(data);
    return true;
}

//
// Store the current value of this config item into the characteristic
uint8_t BLEConfigItemBase::updateCharacteristicValue(bool shouldNotify)
{
    uint8_t numBytes = 0;
    if (m_pChar != NULL)
    {
        uint8_t buffer[256];
        numBytes = encode(buffer, 256);
        m_pChar->setValue(buffer, numBytes);
        if (shouldNotify)
        {
            m_pChar->notify(); // Notify that the value has changed
        }
    }
    return numBytes; 
}


