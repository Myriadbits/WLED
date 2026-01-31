///////////////////////////////////////////////////////////////////////////////
// MyriadclockSettings class
// Class for storing app wide settings
///////////////////////////////////////////////////////////////////////////////

#include "BLEConfig.h"
#include "BLEConfigItemWiFi.h"

//
// Decode an incoming config item
// data: string containing the data
void BLEConfigItemWiFi::onDecodeData(std::string data)
{
    // The first byte contains the index of the WiFi option
    m_value = data[0]; // Store the index
    // And get the matching SSID
    MIOTConfigOption_t* poption = getOption(m_value);
    if (poption != NULL)
        m_sSSID = poption->m_sName.c_str();

    // Next bytes contain the passphrase
    if (data.length() > 1)
        m_sPassphrase = data.substr(1); 
}

//
// Load this config item from the preferences
void BLEConfigItemWiFi::onLoad(Preferences &preferences, char* pkey)
{
    m_sSSID = std::string(preferences.getString(BLECONFIG_PREFERENCES_SSID, "").c_str());
    m_sPassphrase = std::string(preferences.getString(BLECONFIG_PREFERENCES_PASSPHRASE, "").c_str());         

    // Load the X(10) strongest network names as options
    addWiFiSSIDOptions();

    // Select the new WiFi network
    connectToWiFi();
    
    // Now select the sSSID if found
    for (auto it : m_vecOptions)
    {
        if (it.m_sName == m_sSSID)
        {
            m_value = it.m_value;
            break;
        }
    }

    BLECONFIG_LOG("Loaded WiFi settings: '%s' (index %d) with passphrase: %s", m_sSSID.c_str(), m_value, m_sPassphrase.c_str());
}

//
// Store this config item to the preferences
void BLEConfigItemWiFi::onStore(Preferences &preferences, char* pkey)
{
    preferences.putString(BLECONFIG_PREFERENCES_SSID, m_sSSID.c_str());
    preferences.putString(BLECONFIG_PREFERENCES_PASSPHRASE, m_sPassphrase.c_str());

    // Special case, we do the WiFi ourself
    BLECONFIG_LOG("Setting WiFi to index %d", getValue());
    MIOTConfigOption_t* poption = getOption((uint8_t) getValue());
    if (poption != NULL)
    {
        BLECONFIG_LOG("Switching to network: '%s' with passphrase '%s'", poption->m_sName.c_str(), m_sPassphrase.c_str());
        m_sSSID = poption->m_sName;

        // Disconnect
        WiFi.disconnect();

        // Connect to the new WiFi network
        connectToWiFi();
    }
}

//
// Connect to the WiFi network
void BLEConfigItemWiFi::connectToWiFi()
{
    // Select the new WiFi network
    int status = WiFi.begin(m_sSSID.c_str(), m_sPassphrase.c_str());
    if (status == WL_CONNECTED)
    {
        BLECONFIG_LOG("Connected to %s", WiFi.SSID().c_str());
        BLECONFIG_LOG(" - IP address: %s", WiFi.localIP().toString().c_str());
        BLECONFIG_LOG(" - Gateway: %s", WiFi.gatewayIP().toString().c_str());
        BLECONFIG_LOG(" - Mask: %s", WiFi.subnetMask().toString().c_str());
    }
    else
    {
        BLECONFIG_LOG("Failed to connect to WiFi SSID '%s', status: %d", m_sSSID.c_str(), status);
    } 
}

//
// Fill all the SSIDs
void BLEConfigItemWiFi::addWiFiSSIDOptions()
{
    BLECONFIG_LOG("Scanning for WiFi networks...");
    int numSsid = WiFi.scanNetworks(false, true);
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

    // Now do a simple sort 
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
    int selected = 0;
    for (int i = 0; i < numSsid && i < 16; i++)  // Add a maximum of 16 networks!
    {
        if (WiFi.SSID(indices[i]) == WiFi.SSID())
            selected = i; // Remember the active SSID
        uint8_t* pbsidd = WiFi.BSSID(indices[i]);
        BLECONFIG_LOG("- Adding WiFi SSID '%s' [%d] (%02x.%02x.%02x.%02x.%02x.%02x)", WiFi.SSID(indices[i]).c_str(), WiFi.RSSI(indices[i]), pbsidd[0], pbsidd[1], pbsidd[2], pbsidd[3], pbsidd[4], pbsidd[5]);
        addOption(indices[i], WiFi.SSID(indices[i]).c_str());
    }
    setValue(selected);
}

//
// Return the value as a hex string (for debugging)
std::string BLEConfigItemWiFi::valueToString()
{
    return m_sSSID;
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
    idx = addOptionData(pdata, dataLen, idx);
    pdata[idx++] = (uint8_t)(m_isConnected);
    return idx;
}

//