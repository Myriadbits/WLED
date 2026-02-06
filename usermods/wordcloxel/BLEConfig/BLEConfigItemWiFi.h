#pragma once

#include "BLEConfigItemOption.h"


///////////////////////////////////////////////////////////////////////////////
// Single MIOT Configuration item
///////////////////////////////////////////////////////////////////////////////
class BLEConfigItemWiFi : public BLEConfigItemOption
{
public:
    BLEConfigItemWiFi(uint16_t id, const std::string name, bool secure = true)
        : BLEConfigItemOption(id, EConfigType::CT_WIFI, name, 0, secure)
    {        
    }

    std::string getSSID() { return m_sSSID; }
    std::string getPassphrase() { return m_sPassphrase; }
    bool isConnected() { return m_isConnected; }
    void setConnected(bool isConnected);
    void refreshSSIDList();

protected:
    virtual int onEncodeData(uint8_t *pdata, int dataLen, int idx);
    virtual void onDecodeData(std::string data);
    virtual void onLoad() override;
    virtual void onStore() override;
    virtual void onConnect() override;
    virtual std::string valueToString();

private:
    void connectToWiFi();
    void addWiFiSSIDOptions();
   
private:
    std::string m_sSSID;
    std::string m_sPassphrase;
    bool m_isConnected = false;
};