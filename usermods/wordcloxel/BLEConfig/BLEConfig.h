/*  BLEConfig
    Myriadbits BLE Config tool for IOT devices 
    
    Copyright (c) 2021 Jochem Bakker. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Dependencies: 
        Wifi.h


    The purpose of this class is to have a fast and stable deploy and discovery mechanism
    for ESP32's (and ESP8266). 
    - The initial deployment is done by creating a softAP with a fixed SSID and softAPPassword (configurable). 
    - When a connection is made by a client to this softAP, the softAP listens for a json packet with the WiFi credentials for
      the host WiFi network.
    - After receiving the credentials, this class will try to connect to the WiFi with the supplied credentials
    - Upon success, it will join a UDP multicast group and start broadcasting its information (productname, deviceid and version)
    - The WiFi credentials will be stored. Upon a reboot, those credentials will be re-used
    - When the WiFi connection is lost for some time a retry mechanism kicks in, after repeated failures, the initial
      deployment sequence is restarted.

    This class works in collaboration with the BLEConfig, an App (android/ios) to fully automate this process
*/

#pragma once

#include <Preferences.h>

#include <BLEDevice.h>
//#include <BLEUtils.h>
#include <BLEServer.h>

#if defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
    #include <WiFi.h>
#endif

#include <vector>

#include "BLEConfigItemBase.h"
#include "BLEConfigItemOption.h"
#include "BLEConfigItemString.h"
#include "BLEConfigItemUInt8.h"
#include "BLEConfigItemUInt32.h"
#include "BLEConfigItemWiFi.h"
#include "BLEConfigItemDate.h"
#include "BLEConfigItemTime.h"
#include "BLEConfigItemCommand.h"

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
// Defines
///////////////////////////////////////////////////////////////////////////////

// Default names
#define BLECONFIG_PREF_CONFIG                "BLEConfig"  
#define BLECONFIG_DEFAULT_MODELNAME          "BLEConfigDevice"
#define BLECONFIG_DEFAULT_MANUFACTURERNAME   "Myriadbits"
#define BLECONFIG_DEFAULT_VERSION            "1.0.0"
#define BLECONFIG_DEFAULT_APPEARANCE         256 // Default to clock see also: https://developer.nordicsemi.com/nRF5_SDK/nRF51_SDK_v4.x.x/doc/html/group___b_l_e___a_p_p_e_a_r_a_n_c_e_s.html

// BLE Defines
#define BLECONFIG_SERVICE_UUID               "ebd7dc16-04a0-4f9c-96f3-05644d494f54"
#define BLECONFIG_CHAR_CONFIG                "ebd7%04x-04a0-4f9c-96f3-05644d494f54" // snprintf Format (do not use 0000)

//
// Interface for callbacks called by BLEConfig class
class IBLEConfigCallbacks
{
public:
    virtual ~IBLEConfigCallbacks() {};

	//
    // Callback that will be called with a random passkey to should be displayed
    //
	virtual void onDisplayPassKey(uint32_t passkey) = 0;

    //
    // Callback that will be called when the bluetooth connection is established or has failed
    //
	virtual void onBluetoothConnection(bool success) = 0;

    //
    // Callback that is called when an config item is changed
    //
    virtual void onConfigItemChanged(BLEConfigItemBase *pconfigItem);
};


///////////////////////////////////////////////////////////////////////////////
// BLEConfig
// Automatic SmartConfig/Wifi connection
///////////////////////////////////////////////////////////////////////////////
class BLEConfig : public BLESecurityCallbacks, public BLECharacteristicCallbacks, public BLEServerCallbacks
{
public:
    BLEConfig(const std::string model = BLECONFIG_DEFAULT_MODELNAME, const std::string manufacturer = BLECONFIG_DEFAULT_MANUFACTURERNAME, const std::string version = BLECONFIG_DEFAULT_VERSION, int appearance = BLECONFIG_DEFAULT_APPEARANCE);

    // Methods to register a new config item:
    //----------------------------------------------------------------
    // Please note that after adding/removing a configuration item, you have to 'forget' the bluetooth connection on your smartphone to the device and
    // reconnect/pair the device or the changes will NOT be visible!!
    //----------------------------------------------------------------
    BLEConfigItemWiFi*    registerWifi(uint8_t id, const std::string name, bool secure = true);    
    BLEConfigItemString*  registerString(uint8_t id, const std::string name, const std::string defaultValue,bool secure = true);    
    BLEConfigItemUInt32*  registerValue(uint8_t id, const std::string name, uint32_t defaultValue, bool secure = true);    
    BLEConfigItemUInt32*  registerRGBColor(uint8_t id, const std::string name, uint32_t defaultColor, bool secure = true);    
    BLEConfigItemUInt8*   registerSlider(uint8_t id, const std::string name, uint8_t defaultValue, bool secure = true);    
    BLEConfigItemOption*  registerOption(uint8_t id, const std::string name, uint8_t defaultValue, bool secure = true);    
    BLEConfigItemDate*    registerDate(uint8_t id, const std::string name, uint16_t defaultYear = 0, uint8_t defaultMonth = 0, uint8_t defaultDay = 0, bool secure = true);    
    BLEConfigItemTime*    registerTime(uint8_t id, const std::string name, uint8_t defaultHour = 0, uint8_t defaultMinute = 0, uint8_t defaultSecond = 0, bool secure = true);    
    BLEConfigItemCommand* registerCommandOption(uint8_t id, const std::string name, bool secure = true);
    
    BLEConfigItemBase*    getConfigItem(const uint8_t id);

    uint32_t              getConfigValue(const uint8_t id);
    std::string           getConfigValueString(const uint8_t id);

    static void gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    static void gattClientEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param);
    static void gattServerEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
    void init();

    // Start the BLE Config services
    void start(IBLEConfigCallbacks* pCallBacks);   

    // Setters
    void setProductName(const std::string model) { m_model = model; }
    void setManufacturerName(const std::string manufacturer) { m_manufacturer = manufacturer; }
    void setDeviceId(const std::string deviceId) { m_deviceId = deviceId; }
    void setVersion(const std::string version) { m_version = version; }
    void setAppearance(int appearance) { m_appearance = appearance; }

    // Getters
    std::string getProductName() {return  m_model; }
    std::string getManufacturerName() {return m_manufacturer; }
    std::string getDeviceId() { return m_deviceId; }
    std::string getVersion() { return m_version; }
    int getAppearance() { return m_appearance; }
    bool isDeviceConnected() { return m_isDeviceConnected; }

protected:
	virtual uint32_t onPassKeyRequest();
	virtual void onPassKeyNotify(uint32_t pass_key);
	virtual bool onSecurityRequest();
	virtual void onAuthenticationComplete(esp_ble_auth_cmpl_t);
	virtual bool onConfirmPIN(uint32_t pin);

	virtual void onWrite(BLECharacteristic* pCharacteristic);

    // BLEServer callbacks
    virtual void onConnect(BLEServer* pServer);
    virtual void onDisconnect(BLEServer* pServer);

private:
    void    addConfigCharacteristic(BLEService *pBLEConfigService, BLEConfigItemBase* pitem);
    void    addWiFiSSIDOptions(BLEConfigItemBase* pitem);

private:
    IBLEConfigCallbacks*    m_pCallBacks; // Pointer to the callback interface
    BLEServer*              m_pBLEServer; // The BLE server

    // All config items
    std::vector<BLEConfigItemBase*>     m_vecConfigItems;

    // General data
    std::string             m_model;
    std::string             m_manufacturer;
    std::string             m_deviceId;
    std::string             m_deviceName;
    std::string             m_version;
    int                     m_appearance;
    bool                    m_isDeviceConnected;
};
