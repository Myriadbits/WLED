///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfigItemOption.h"

//
// Simply remove all options
//
void BLEConfigItemOption::clearOptions()
{
    m_vecOptions.clear();
}

//
// Add a new option for this configuration item, only applies to 
// optionValue: The value/number/index of this option
// optionText: The text of this option that is displayed in a combobox/selectionbox
void BLEConfigItemOption::addOption(const uint8_t optionValue, const std::string optionText)
{
    MIOTConfigOption_t option = { optionValue, optionText };
    m_vecOptions.push_back(option);
}

//
// Get a specific option identified by a value
// optionValue: The value of this option
// Returns the option
MIOTConfigOption_t* BLEConfigItemOption::getOption(const uint8_t optionValue)
{
    for (int n = 0; n < m_vecOptions.size(); n++)
    {
        if (m_vecOptions[n].m_value == optionValue)
            return &(m_vecOptions[n]);
    }
    return NULL;
}

//
// Encode this config string item into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item of this string
int BLEConfigItemOption::onEncodeData(uint8_t *pdata, int dataLen, int idx)
{
    return addOptionData(pdata, dataLen, idx);
}

//
// Helper method to encode this config item option item into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item of this string
int BLEConfigItemOption::addOptionData(uint8_t *pdata, int dataLen, int idx)
{
    // First byte is the actual data (no more then 256 options are allowed)
    pdata[idx++] = (uint8_t)(m_value & 0x000000FF);
    // Next byte is the number of options
    int numberOfOptionsIndex = idx; // Store this position (we need it later)
    pdata[idx++] = (uint8_t)(m_vecOptions.size());
    // Next are all the options, first the value followed by the text
    for(int n = 0; n < m_vecOptions.size(); n++)
    {
        int optionLen = 2 + m_vecOptions[n].m_sName.length();
        if (idx + optionLen > dataLen)
        {
            // Doesn't fit, quit adding options
            // But update the number of items we actualy added
            pdata[numberOfOptionsIndex] = (uint8_t)(n);
            break; 
        }

        // Add the option value (single byte)
        pdata[idx++] = m_vecOptions[n].m_value;
        int slen = (uint8_t) std::min((int) m_vecOptions[n].m_sName.length(), 0xF0);
        // Add the name length  
        pdata[idx++] = slen; 
        for(int i = 0; i < slen; i++)
            pdata[idx++] = m_vecOptions[n].m_sName.c_str()[i]; // And add the option name
    }
    return idx;
}