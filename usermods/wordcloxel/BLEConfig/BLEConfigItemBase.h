#pragma once

#include <Preferences.h> // We are dependent upon the preferences
#include "BLECharacteristic.h"

enum EConfigType : uint8_t
{
    CT_UNKNOWN,            
    CT_STRING,           // String
    CT_UINT32,           // Value    
    CT_RGBCOLOR,         // RGB color
    CT_SLIDER,           // Slider (0-100) 
    CT_OPTION,           // Options (combobox/selection box)
    CT_DATE,             // Date
    CT_TIME,             // Time
    CT_WIFI,             // List of all WiFi SSIDS in range (CT_OPTION with SSIDs filled in), when switching, we expect the passphrase as well
    CT_UINT8,            // Slider (0-100) 
    CT_COMMAND,          // List of commands. When written, executes a single command
};


// TODO Remove
#define  BLECONFIG_DEBUG

// Debug output destination can be defined externally with BLECONFIG_DEBUG_PORT
#ifndef BLECONFIG_DEBUG_PORT
    #define BLECONFIG_DEBUG_PORT Serial
#endif 
#ifdef BLECONFIG_DEBUG
    #define BLECONFIG_LOG(...) do {BLECONFIG_DEBUG_PORT.printf("[BLEConfig %lu] ", millis()); BLECONFIG_DEBUG_PORT.printf( __VA_ARGS__ ); BLECONFIG_DEBUG_PORT.println();} while (0)
#else
    #define BLECONFIG_LOG(...)
#endif 


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemBase
{
public:
    BLEConfigItemBase(uint16_t id, const EConfigType type, const char *pName);

    // Properties
    uint16_t getId() { return m_id;}
    EConfigType getType() { return m_eType;}

    void setCharacteristic(BLECharacteristic* pChar) { m_pChar = pChar; }
    uint8_t updateCharacteristicValue(bool shouldNotify = false);

    char* getName() { return (char*) m_pName; }

    virtual void onSetup() {};

private:
    int encode(uint8_t *pdata, int dataLen);
    bool decode(std::string data);

    friend class BLEConfig;

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx) { return idx;};
    virtual void onDecodeData(std::string data) {};
      
private:
    uint16_t            m_id; // Unique ID 
    EConfigType         m_eType; // The config item type
    const char*         m_pName = nullptr; // Pointer to the name
    const char*         m_pSynopsis = nullptr; // Short description
    BLECharacteristic*  m_pChar; // Pointer to the characteristic
};
