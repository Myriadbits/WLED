///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfig.h"
#include "BLEConfigItemWiFi.h"
#include "wled.h"

//
// Decode an incoming config item
// data: string containing the data
void BLEConfigItemWiFi::onDecodeData(std::string data)
{
    // The first byte contains the index of the WiFi option
    m_value = data[0]; // Store the index
    // And get the matching SSID
    MIOTConfigOption_t* poption = getOptionByIndex(m_value); // TODO Is actually an error in the app. App should return value not the index
    if (poption != NULL)
    {
        m_sSSID = poption->m_sName.c_str();
    }

    // Next bytes contain the passphrase
    if (data.length() > 1)
        m_sPassphrase = data.substr(1); 
    BLECONFIG_LOG("WiFi onDecodeData SSID '%s', passphrase '%s'", m_sSSID.c_str(), m_sPassphrase.c_str());
}

//
// Load this config item from the preferences
void BLEConfigItemWiFi::onSetup()
{
    // Load the strongest network names as options
    addWiFiSSIDOptions();
    BLECONFIG_LOG("Loaded WiFi settings: '%s' (index %d) with passphrase: %s", m_sSSID.c_str(), m_value, m_sPassphrase.c_str());
}

//
// Fill all the SSIDs
void BLEConfigItemWiFi::addWiFiSSIDOptions()
{
    BLECONFIG_LOG("Scanning for WiFi networks...");
    int numSsid = WiFi.scanNetworks(false);
    if (numSsid == -1) 
    {
        BLECONFIG_LOG("Error: Couldn't get a wifi connection! [-1]");
        return;        
    }
    else if (numSsid == -2) 
    {
        BLECONFIG_LOG("Error: WiFi Scan failed! [-2]");
        return;
    }
    else if (numSsid < 0)
    {
        // Is not in the spec, but just to be sure
        BLECONFIG_LOG("Error: WiFi.scanNetworks unknown error!");
        return;
    }

    uint8_t indices[numSsid];
    for (int i = 0; i< numSsid; i++)
        indices[i] = i;

    // Now do a simple sort on RSSI to have the strongest networks first
    for (int i = 0; i < numSsid; i++) 
    {
        for (int j = i + 1; j < numSsid; j++) 
        {
            if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) 
            {
                std::swap(indices[i], indices[j]);  
            }
        }
    }

    BLECONFIG_LOG("Found %d WiFi networks", numSsid);
    clearOptions();
    int selected = -1;
    for (int i = 0; i < numSsid && i < 16; i++)  // Add a maximum of 16 networks!
    {
        if (selected < 0 && WiFi.SSID(indices[i]) == WiFi.SSID())
            selected = i; // Remember the active SSID
        if (WiFi.SSID(indices[i]).length() > 0)
        {
            // Check if we already have this SSID as an option (can happen when we have multiple APs with the same SSID)
            if (isOptionPresent(WiFi.SSID(indices[i]).c_str()))
                continue;
            BLECONFIG_LOG("- Adding WiFi SSID %d: '%s' [%d]", indices[i], WiFi.SSID(indices[i]).c_str(), WiFi.RSSI(indices[i]));
            addOption(indices[i], WiFi.SSID(indices[i]).c_str());
        }
    }
    if (selected < 0)
        selected = 0;
    setValue(selected);

    WiFi.scanDelete();
}

//
// Change the connection state 
void BLEConfigItemWiFi::setConnected(bool isConnected)
{
    if (isConnected != m_isConnected)
    {
        // Only change/update and notify the value when it is changed
        m_isConnected = isConnected;
        updateCharacteristicValue(true);
        BLECONFIG_LOG("WiFi connection state changed to: %s", m_isConnected ? "CONNECTED" : "DISCONNECTED");
    }
}

//
// Refresh the SSID list
void BLEConfigItemWiFi::refreshSSIDList()
{
    BLECONFIG_LOG("Refreshing SSID list");
    addWiFiSSIDOptions();
    updateCharacteristicValue(true);
}

//
// Encode this WiFi data into a byte array/buffer
// pdata: pointer to the buffer
// idx: index where the data should be stored
// returns the index of the last item of this string
int BLEConfigItemWiFi::onEncodeData(uint8_t *pdata, int dataLen, int idx)
{
    // Start by adding the option data
    idx = BLEConfigItemOption::onEncodeData(pdata, dataLen, idx);
    pdata[idx++] = (uint8_t)(m_isConnected);
    return idx;
}