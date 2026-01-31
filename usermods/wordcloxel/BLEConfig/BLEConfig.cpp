//#include <string.h>
//#include <sys/param.h>

#include "BLEConfig.h"
#include "esp32-hal-bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

inline size_t getFreeHeapSize() { return heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); } // returns free heap (ESP.getFreeHeap() can include other memory types)

//
// Constructor, use this to change the timeouts
// BLEConfig uses SmartConfig create a WiFi connection, upon connection losses
// and incorrect credentials the SmartConfig procedure will be restarted. When SmartConfig
// times-out, stored WiFi credentials will be retried.
// 
// @param productName The name of this product
// @param version The version of this product
// @param appearance The appearance, see: https://developer.nordicsemi.com/nRF5_SDK/nRF51_SDK_v4.x.x/doc/html/group___b_l_e___a_p_p_e_a_r_a_n_c_e_s.html
BLEConfig::BLEConfig(const std::string model, const std::string manufacturer, const std::string version, int appearance)
    : m_model(model)
    , m_manufacturer(manufacturer)
    , m_version(version)
    , m_appearance(appearance)
    , m_isDeviceConnected(false)
{    
}

BLEConfigItemWiFi* BLEConfig::registerWifi(uint8_t id, const std::string name, bool secure)
{
    BLEConfigItemWiFi *pitem = new BLEConfigItemWiFi(id, name, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemString* BLEConfig::registerString(uint8_t id, const std::string name, const std::string defaultValue,bool secure)
{
    BLEConfigItemString *pitem = new BLEConfigItemString(id, name, defaultValue, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemUInt32* BLEConfig::registerValue(uint8_t id, const std::string name, uint32_t defaultValue, bool secure)
{
    BLEConfigItemUInt32 *pitem = new BLEConfigItemUInt32(id, name, defaultValue, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemUInt32* BLEConfig::registerRGBColor(uint8_t id, const std::string name, uint32_t defaultColor, bool secure)
{
    BLEConfigItemUInt32 *pitem = new BLEConfigItemUInt32(id, CT_RGBCOLOR, name, defaultColor, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemUInt8* BLEConfig::registerSlider(uint8_t id, const std::string name, uint8_t defaultValue, bool secure)
{
    BLEConfigItemUInt8 *pitem = new BLEConfigItemUInt8(id, CT_SLIDER, name, defaultValue, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemOption* BLEConfig::registerOption(uint8_t id, const std::string name, uint8_t defaultValue, bool secure)
{
    BLEConfigItemOption *pitem = new BLEConfigItemOption(id, name, defaultValue, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemDate* BLEConfig::registerDate(uint8_t id, const std::string name, uint16_t defaultYear, uint8_t defaultMonth, uint8_t defaultDay, bool secure)
{
    BLEConfigItemDate *pitem = new BLEConfigItemDate(id, name, defaultYear, defaultMonth, defaultDay, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemTime* BLEConfig::registerTime(uint8_t id, const std::string name, uint8_t defaultHour, uint8_t defaultMinute, uint8_t defaultSecond, bool secure)
{
    BLEConfigItemTime *pitem = new BLEConfigItemTime(id, name, defaultHour, defaultMinute, defaultSecond, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

BLEConfigItemCommand* BLEConfig::registerCommandOption(uint8_t id, const std::string name, bool secure)
{
    BLEConfigItemCommand *pitem = new BLEConfigItemCommand(id, name, secure);
    m_vecConfigItems.push_back(pitem);
    return pitem;
}

//
// Return a config item with a specific id
BLEConfigItemBase* BLEConfig::getConfigItem(const uint8_t id)
{
    for (auto it : m_vecConfigItems)
    {
        if (it->getId() == id)
            return it;
    }
    return NULL;
}

//
// Return the value of a config item with a specific id (only for value based)
uint32_t BLEConfig::getConfigValue(const uint8_t id)
{
    BLEConfigItemBase* pconfig = getConfigItem(id);
    if (pconfig != NULL)
    {
        switch (pconfig->getType())
        {
            case CT_DATE:
            case CT_TIME:
            case CT_UINT32:
            case CT_RGBCOLOR:
                {
                    BLEConfigItemUInt32* pconfigvalue = (BLEConfigItemUInt32*) pconfig;
                    if (pconfigvalue != NULL)
                        return pconfigvalue->getValue();
                }
                break;

            case CT_SLIDER:
            case CT_OPTION:
                {
                    BLEConfigItemUInt8* pconfigvalue = (BLEConfigItemUInt8*) pconfig;
                    if (pconfigvalue != NULL)
                        return pconfigvalue->getValue();
                }
                break;

            default:
                return 0;
        }
    }
    return 0;
}

//
// Return the string value of a config item of a specific id (only for string based)
std::string BLEConfig::getConfigValueString(const uint8_t id)
{
    BLEConfigItemBase* pconfig = getConfigItem(id);
    if (pconfig != NULL)
    {
        switch (pconfig->getType())
        {
            case CT_STRING:
                {
                    BLEConfigItemString* pconfigvalue = (BLEConfigItemString*) pconfig;
                    if (pconfigvalue != NULL)
                        return pconfigvalue->getValue();
                }
                break;
            default:
                return std::string("");
        }
    }
    return std::string("");
}

//
// setup the BLE Configurator, call this method in the setup function
// This setup will:
// - Start WiFi using the stored credentials (if any and if valid)
// - If no credentials are found, a SoftAP is started with a unique deviceID as SSID
//   and the password (as argument)
// Note that the password should be entered by the user of the configuration tool
// Might be good to add that as a QR code to the product itself
void BLEConfig::start(IBLEConfigCallbacks* pCallBacks)
{
    m_preferences.begin(BLECONFIG_PREFERENCES);

    m_pCallBacks = pCallBacks;

    // Default device ID is the serial number of the Chip
    if (m_deviceId.empty())
    {
        uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
        // Chip ID is 64 bit, I find that a bit large for an ID, make it 16 bit (I know, there is a chance some are the same)
        char hex[32];
        snprintf(hex, 32, "%04X", (uint16_t)((chipid >> 32) & 0xFFFF) ^ (uint16_t)((chipid >> 16) & 0xFFFF) ^ (uint16_t)(chipid & 0xFFFF));
        m_deviceId = std::string(hex);
    }
    m_deviceName = m_model + "-" + m_deviceId;
   // WiFi.setHostname(m_deviceName.c_str());

    BLECONFIG_LOG("Starting BLE Config");
    BLECONFIG_LOG("- Product:  %s", m_model.c_str());
    BLECONFIG_LOG("- DeviceId: %s", m_deviceId.c_str());
    BLECONFIG_LOG("- DeviceName: %s", m_deviceName.c_str());
    BLECONFIG_LOG("- Version:  %s", m_version.c_str());

    
    //
    // BLE initializatons
    //
    // BLECONFIG_LOG("BLE initialized: %d", BLEDevice::getInitialized());
    // if (!btStart()) {
    //     BLECONFIG_LOG("ERROR: btStart failed");
    //     return;
    // }
    // vTaskDelay(200 / portTICK_PERIOD_MS); // Delay for 200 msecs as a workaround to an apparent Arduino environment issue.

    BLEDevice::init(m_deviceName.c_str());
    // BLECONFIG_LOG("BLE initialized: %d", BLEDevice::getInitialized());

    // BLECONFIG_LOG("btStarted: %d", btStarted());


   // init(); 
    

    m_pBLEServer = BLEDevice::createServer();
    m_pBLEServer->setCallbacks(this);

    // For BLE number, see: https://btprodspecificationrefs.blob.core.windows.net/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf
    // For the different standard services, see: https://www.bluetooth.com/specifications/gatt/

    ///////////////////////////////////
    // Device information service
    BLEUUID uuidDeviceInfo((uint16_t) 0x180a);
    BLEService *pDeviceInfoService = m_pBLEServer->createService(uuidDeviceInfo, 16, 0);    
    // Manufacturer
    BLECharacteristic *pCharManufacturer = pDeviceInfoService->createCharacteristic(BLEUUID((uint16_t) 0x2a29), BLECharacteristic::PROPERTY_READ);
    pCharManufacturer->setAccessPermissions(ESP_GATT_PERM_READ);
    pCharManufacturer->setValue(m_manufacturer);
    // Model
    BLECharacteristic *pCharModel = pDeviceInfoService->createCharacteristic(BLEUUID((uint16_t) 0x2a24), BLECharacteristic::PROPERTY_READ);
    pCharModel->setAccessPermissions(ESP_GATT_PERM_READ);
    pCharModel->setValue(m_model);
    // Serial number
    BLECharacteristic *pCharSerialNumber = pDeviceInfoService->createCharacteristic(BLEUUID((uint16_t) 0x2a25), BLECharacteristic::PROPERTY_READ);
    pCharSerialNumber->setAccessPermissions(ESP_GATT_PERM_READ);
    pCharSerialNumber->setValue(m_deviceId);  
    // Software revision string    
    BLECharacteristic *pCharRevision = pDeviceInfoService->createCharacteristic(BLEUUID((uint16_t) 0x2a28), BLECharacteristic::PROPERTY_READ);
    pCharRevision->setAccessPermissions(ESP_GATT_PERM_READ);
    pCharRevision->setValue(m_version);
      
    // // Start all device info
    pDeviceInfoService->start();

    //
    // Load all data for all config items
    for (auto it : m_vecConfigItems)
        it->load(m_preferences);

    // BLEConfig service
    BLECONFIG_LOG("Starting BLE service with %d config items", m_vecConfigItems.size());
    BLEUUID uuidBLEConfigService(BLECONFIG_SERVICE_UUID);
    BLEService *pBLEConfigService = m_pBLEServer->createService(uuidBLEConfigService, m_vecConfigItems.size() * 4, 2);
    for (auto it : m_vecConfigItems)
    {
        addConfigCharacteristic(pBLEConfigService, it);
    }

    // Finally: start the service
    pBLEConfigService->start();
     
    // Create + start the advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(uuidDeviceInfo);
    pAdvertising->addServiceUUID(uuidBLEConfigService);
    pAdvertising->setAppearance(m_appearance);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    BLECONFIG_LOG("Advertising started");

    // Set the security features
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setCapability(ESP_IO_CAP_OUT);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    BLEDevice::setSecurityCallbacks(this);

     BLECONFIG_LOG("Security setup completed");
}

//
// Add a characteristic to the BLE Config service
// Note that 'id' should be 1 or higher!
// Returns the BLEUUID of the config item
void BLEConfig::addConfigCharacteristic(BLEService *pBLEConfigService, BLEConfigItemBase* pitem)
{
    char uuid[64];
    snprintf(uuid, 64, BLECONFIG_CHAR_CONFIG, pitem->getId());
    BLEUUID uuidConfig(uuid);
    
    BLECharacteristic *pChar = pBLEConfigService->createCharacteristic(uuidConfig, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
    //pChar->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED); // TODO CHANGE BACK WHEN WE WANT SECURITY
    pChar->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    pChar->setCallbacks(this);

    // Let the config item know the characteristic it is linked too
    pitem->setCharacteristic(pChar);

    // Value consist
    uint8_t byteCount = pitem->updateCharacteristicValue();
    BLECONFIG_LOG("- Adding characteristic for '%s' [%d bytes]", pitem->getName().c_str(), byteCount);

    // snprintf(charName, 64, BLECONFIG_CHAR_CONFIG, n + 0x0101); // Do NOT start at 0!
    // BLEDescriptor *pdesc = new BLEDescriptor(charName);       
    // pdesc->setAccessPermissions(ESP_GATT_PERM_READ);         
    // snprintf(text, 64, "Description [%d]", n + 1);
    // pdesc->setValue(text);
    // pChar->addDescriptor(pdesc);
}

//
// Store all settings
void BLEConfig::store()
{
     for (auto it : m_vecConfigItems)
         it->store(m_preferences);
}

//
// BLE Characteristic is written
void BLEConfig::onWrite(BLECharacteristic* pCharacteristic)
{
    // Parse the 
    uint32_t uid = 0; 
    if (sscanf(pCharacteristic->getUUID().toString().c_str(), BLECONFIG_CHAR_CONFIG, &uid) == 1)
    {
        BLECONFIG_LOG("- Data received for config item %d", uid);

        // Find the matching config item
        bool found = false;
        for (auto it : m_vecConfigItems)
        {
            if (it != NULL && it->getId() == uid)
            {
                it->decode(pCharacteristic->getValue());//, pCharacteristic->getData());

                BLECONFIG_LOG("Setting config item [%d]: '%s' to '%s'", uid, it->getName().c_str(), it->valueToString().c_str());

                // Store the new changed value
                it->store(m_preferences);

                // Forward to the callbacks that the config item is changed!
                if (m_pCallBacks != NULL)
                    m_pCallBacks->onConfigItemChanged(it);

                // Now set the value back to the full-descriptive text
                it->updateCharacteristicValue();
                found = true;
                break;
            }
        }
        if (!found)
            BLECONFIG_LOG("Error. Data received for unknown config item %d", uid);
    }
}

// BLEServer callbacks

void BLEConfig::onConnect(BLEServer* pServer)
{
    BLECONFIG_LOG("onConnect");
    m_isDeviceConnected = true;
}

void BLEConfig::onDisconnect(BLEServer* pServer)
{
    BLECONFIG_LOG("OnDisconnect");
    BLEDevice::startAdvertising();
    m_isDeviceConnected = false;
}

uint32_t BLEConfig::onPassKeyRequest()
{
    BLECONFIG_LOG("OnPassKeyRequest");
    return 123456;
}

void BLEConfig::onPassKeyNotify(uint32_t pass_key)
{       
    BLECONFIG_LOG("The passkey Notify number:%d", pass_key); // <--- this one
    if (m_pCallBacks)
        m_pCallBacks->onDisplayPassKey(pass_key);
}

bool BLEConfig::onConfirmPIN(uint32_t pass_key)
{
    BLECONFIG_LOG("The passkey YES/NO number:%d", pass_key);
    vTaskDelay(1000);
    return true;
}

bool BLEConfig::onSecurityRequest()
{
    BLECONFIG_LOG("Security Request\n");
    return true;
}

void BLEConfig::onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
{
    if(auth_cmpl.success)
    {
        BLECONFIG_LOG("remote BD_ADDR: %d.%d.%d.%d.%d.%d", auth_cmpl.bd_addr[0], auth_cmpl.bd_addr[1], auth_cmpl.bd_addr[2], auth_cmpl.bd_addr[3], auth_cmpl.bd_addr[4], auth_cmpl.bd_addr[5]);
        //esp_log_buffer_hex(LOG_TAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
        BLECONFIG_LOG("address type = %d", auth_cmpl.addr_type);
    }
    BLECONFIG_LOG("Pair status = %s", auth_cmpl.success ? "success" : "fail");

    if (m_pCallBacks)
        m_pCallBacks->onBluetoothConnection(auth_cmpl.success);
}
