#pragma once

#include "BLEConfig.h"

typedef struct 
{
    uint8_t         m_value; // The value of the config option
    std::string     m_sName; // Name of this option item
} MIOTWiFiOption_t;


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemWiFi : public BLEConfigItemBase
{
public:
    BLEConfigItemWiFi(uint16_t id)
        : BLEConfigItemBase(id)
    {        
    }

    std::string getSSID() { return m_sSSID; }
    std::string getPassphrase() { return m_sPassphrase; }
    bool isConnected() { return m_isConnected; }
    void setConnected(bool isConnected);
    void refreshSSIDList();

    virtual void onSetup() override;

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);

private:
    MIOTWiFiOption_t* getOptionByIndex(const uint8_t index);
    bool isOptionPresent(const std::string optionText);
    void addWiFiSSIDOptions();
   
private:
    std::string m_sSSID;
    std::string m_sPassphrase;
    bool m_isConnected = false;
    uint8_t                         m_value;
    std::vector<MIOTWiFiOption_t>   m_vecOptions;
};