///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemBase.h"

//
// Constructor, create/fill this config item
BLEConfigItemBase::BLEConfigItemBase(uint16_t id, const EConfigType type, const char *pName)
    : m_id(id)
    , m_eType(type)
    , m_pName(pName)
{
}

//
// Encode this config item into a byte array/buffer
// pdata: pointer to the buffer
// dataLen: length of the buffer
// Note that the buffer should (for now) hold at least 256 bytes
int BLEConfigItemBase::encode(uint8_t *pdata, int dataLen)
{
    int slen = 0;
    if (dataLen < 256) return 0; // TODO CALCULATE THE DESIRED BUFFER/DATALENGTH

    // Small header with ID + Type
    pdata[0] = m_id;
    pdata[1] = (uint8_t) m_eType;

    // Add the flags
    pdata[2] = 0x00;
    pdata[2] |= 0x01; // Secure or not

    // Start at byte 3
    int idx = 3;

    // Add the config item name
    slen = (uint8_t) std::min((int) strlen(m_pName), 0xF0);
    pdata[idx++] = slen;
    for(int n = 0; n < slen; n++)
        pdata[idx++] = (uint8_t) m_pName[n];

    // Add the data
    return onEncodeData(pdata, dataLen, idx);
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
// Load this config item from the preferences
void BLEConfigItemBase::load()
{
    onLoad();
}

//
// Store this config item to the preferences
void BLEConfigItemBase::store()
{
    // Create the preference key
    onStore();
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


