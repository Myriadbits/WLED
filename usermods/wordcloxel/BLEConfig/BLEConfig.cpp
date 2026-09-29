//#include <string.h>
//#include <sys/param.h>

#include "BLEConfig.h"
#include "wled.h"

//
// Constructor, use this to change the timeouts
// BLEConfig uses SmartConfig create a WiFi connection, upon connection losses
// and incorrect credentials the SmartConfig procedure will be restarted. When SmartConfig
// times-out, stored WiFi credentials will be retried.
// 
// @param productName The name of this product
// @param version The version of this product
// @param appearance The appearance, see: https://developer.nordicsemi.com/nRF5_SDK/nRF51_SDK_v4.x.x/doc/html/group___b_l_e___a_p_p_e_a_r_a_n_c_e_s.html
BLEConfig::BLEConfig(const char *pModel, const char *pManufacturer, const char *pVersion, int appearance)
    : m_pModel(pModel)
    , m_pManufacturer(pManufacturer)
    , m_pVersion(pVersion)
    , m_appearance(appearance)
    , m_isDeviceConnected(false)
{
    //m_vecConfigItems.reserve(10);    
}

void BLEConfig::addConfigItem(BLEConfigItemBase* pitem)
{
    m_vecConfigItems.push_back(pitem);
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
// setup the BLE Configurator, call this method in the setup function
// This setup will:
// - Start WiFi using the stored credentials (if any and if valid)
// - If no credentials are found, a SoftAP is started with a unique deviceID as SSID
//   and the password (as argument)
// Note that the password should be entered by the user of the configuration tool
// Might be good to add that as a QR code to the product itself
void BLEConfig::start(IBLEConfigCallbacks* pCallBacks)
{
    m_pCallBacks = pCallBacks;

    // Default device ID is the serial number of the Chip
    char deviceId[MAX_DEVICE_ID_LENGTH] = {0};
    if (strlen(m_pDeviceName) == 0)
    {
        uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
        // Chip ID is 64 bit, I find that a bit large for an ID, make it 16 bit (I know, there is a chance some are the same)
        snprintf(deviceId, 8, "%04X", (uint16_t)((chipid >> 32) & 0xFFFF) ^ (uint16_t)((chipid >> 16) & 0xFFFF) ^ (uint16_t)(chipid & 0xFFFF));
        snprintf(m_pDeviceName, MAX_DEVICE_NAME_LENGTH, "%s-%s", m_pModel, deviceId);
    }

    BLECONFIG_LOG("Starting BLE Config");
    BLECONFIG_LOG("- Product:  %s", m_pModel);
    BLECONFIG_LOG("- DeviceId: %s", deviceId);
    BLECONFIG_LOG("- DeviceName: %s", m_pDeviceName);
    BLECONFIG_LOG("- Version:  %s", m_pVersion);
   
    //Initialize the BLE stack
    NimBLEDevice::init(m_pDeviceName);
    NimBLEDevice::setMTU(BLECONFIG_PREFERRED_MTU);
    BLECONFIG_LOG("BLE Initialized");

    m_pBLEServer = NimBLEDevice::createServer();
    m_pBLEServer->setCallbacks(this, false); // BLEConfig is a member of the usermod, never delete it
    m_pBLEServer->advertiseOnDisconnect(false); // Advertising is restarted in onDisconnect

    // For BLE number, see: https://btprodspecificationrefs.blob.core.windows.net/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf
    // For the different standard services, see: https://www.bluetooth.com/specifications/gatt/

    ///////////////////////////////////
    // Device information service
    NimBLEUUID uuidDeviceInfo((uint16_t) 0x180a);
    NimBLEService *pDeviceInfoService = m_pBLEServer->createService(uuidDeviceInfo);

    // Manufacturer
    NimBLECharacteristic *pCharManufacturer = pDeviceInfoService->createCharacteristic(NimBLEUUID((uint16_t) 0x2a29), NIMBLE_PROPERTY::READ);
    pCharManufacturer->setValue((const uint8_t*) m_pManufacturer, strlen(m_pManufacturer));
    // Model
    NimBLECharacteristic *pCharModel = pDeviceInfoService->createCharacteristic(NimBLEUUID((uint16_t) 0x2a24), NIMBLE_PROPERTY::READ);
    pCharModel->setValue((const uint8_t*) m_pModel, strlen(m_pModel));
    // Serial number
    NimBLECharacteristic *pCharSerialNumber = pDeviceInfoService->createCharacteristic(NimBLEUUID((uint16_t) 0x2a25), NIMBLE_PROPERTY::READ);
    pCharSerialNumber->setValue((const uint8_t*) deviceId, strlen(deviceId));
    // Software revision string
    NimBLECharacteristic *pCharRevision = pDeviceInfoService->createCharacteristic(NimBLEUUID((uint16_t) 0x2a28), NIMBLE_PROPERTY::READ);
    pCharRevision->setValue((const uint8_t*) m_pVersion, strlen(m_pVersion));

    // char s[32];
    // IPAddress localIP = Network.localIP();
    // sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);    
    // // Networkaddress 0308: 5.1.20 Interoperability Requirements for Bluetooth technology as a WAP Bearer (WAP)
    // BLECharacteristic *pCharNetworkAddress = pDeviceInfoService->createCharacteristic(BLEUUID((uint16_t) 0x0308), BLECharacteristic::PROPERTY_READ);
    // pCharNetworkAddress->setAccessPermissions(ESP_GATT_PERM_READ);
    // pCharNetworkAddress->setValue((uint8_t*) s, strlen(s));    
      
    // Start all device info
    pDeviceInfoService->start();
    
    //
    // Load all data for all config items
    for (auto it : m_vecConfigItems)
        it->onSetup();

    // BLEConfig service
    BLECONFIG_LOG("Starting BLE service with %d config items", m_vecConfigItems.size());
    NimBLEUUID uuidBLEConfigService(BLECONFIG_SERVICE_UUID);
    NimBLEService *pBLEConfigService = m_pBLEServer->createService(uuidBLEConfigService);
    for (auto it : m_vecConfigItems)
    {
        addConfigCharacteristic(pBLEConfigService, it);
    }

    // Finally: start the service
    pBLEConfigService->start();

    // Firmware update service (not advertised, the app finds it after connecting)
    m_ota.createService(m_pBLEServer, this);

    // Set the security features (same as before: display only, secure connections + bonding).
    // The characteristics do not require encryption, so this only applies when a client asks to pair.
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEDevice::setSecurityAuth(true, false, true);
    BLECONFIG_LOG("Security setup completed");

    // Create + start the advertising
    // The 31 byte advertising packet holds: flags (3) + 128-bit config service (18) + 16-bit device info (4) + appearance (4).
    // NimBLE silently drops UUIDs that do not fit, so the config service (the app scans for it) is added first.
    // The device name goes into the scan response. Do not add setMin/MaxPreferred: those 6 bytes would push a UUID out.
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(uuidBLEConfigService);
    pAdvertising->addServiceUUID(uuidDeviceInfo);
    pAdvertising->setAppearance(m_appearance);
    pAdvertising->setScanResponse(true);

    NimBLEDevice::startAdvertising();
    BLECONFIG_LOG("Advertising started");
}

//
// Add a characteristic to the BLE Config service
// Note that 'id' should be 1 or higher!
// Returns the BLEUUID of the config item
void BLEConfig::addConfigCharacteristic(NimBLEService *pBLEConfigService, BLEConfigItemBase* pitem)
{
    char uuid[64];
    snprintf(uuid, 64, BLECONFIG_CHAR_CONFIG, pitem->getId());
    NimBLEUUID uuidConfig(uuid);

    // NimBLE adds the CCCD (0x2902) for NOTIFY automatically.
    // For encryption use NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC // TODO CHANGE WHEN WE WANT SECURITY
    NimBLECharacteristic *pChar = pBLEConfigService->createCharacteristic(uuidConfig, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pChar->setCallbacks(this);

    // Let the config item know the characteristic it is linked too
    pitem->setCharacteristic(pChar);

    // Value consist
    uint8_t byteCount = pitem->updateCharacteristicValue();
    BLECONFIG_LOG("- Adding characteristic for id:%d [%d bytes]", pitem->getId(), byteCount);
}

//
// BLE Characteristic is written
void BLEConfig::onWrite(NimBLECharacteristic* pCharacteristic)
{
    if (m_ota.isOtaCharacteristic(pCharacteristic))
    {
        m_lastBTActionTime = millis();
        m_ota.onWrite(pCharacteristic);
        return;
    }

    // Parse the 
    uint32_t uid = 0; 
    if (sscanf(pCharacteristic->getUUID().toString().c_str(), BLECONFIG_CHAR_CONFIG, &uid) == 1)
    {
        m_lastBTActionTime = millis();
        BLECONFIG_LOG("Data received for config item %d", uid);

        // Find the matching config item
        bool found = false;
        for (auto it : m_vecConfigItems)
        {
            if (it != NULL && it->getId() == uid)
            {
                NimBLEAttValue value = pCharacteristic->getValue();
                it->decode(std::string(value.c_str(), value.size()));

                //BLECONFIG_LOG("Setting config item [%d]: '%s' to '%s'", uid, it->getName().c_str(), it->valueToString().c_str());

                // Forward to the callbacks that the config item is changed!
                if (m_pCallBacks != NULL)
                    m_pCallBacks->onConfigItemChanged(it);

                // Now set the value back to the full-descriptive text <= TODO this seems weird....
                it->updateCharacteristicValue();
                found = true;
                break;
            }
        }
        if (!found)
            BLECONFIG_LOG("Error. Data received for unknown config item %d", uid);
    }
}

//
// BLE Characteristic is written
void BLEConfig::onRead(NimBLECharacteristic* pCharacteristic)
{
    m_lastBTActionTime = millis();
}


// BLEServer callbacks

void BLEConfig::onConnect(NimBLEServer* pServer)
{
    BLECONFIG_LOG("OnConnect");
    NimBLEDevice::stopAdvertising();
    
    m_lastBTActionTime = millis();
    m_isDeviceConnected = true;
    if (m_pCallBacks != NULL)
        m_pCallBacks->onBluetoothConnection(m_isDeviceConnected);
}

void BLEConfig::onDisconnect(NimBLEServer* pServer)
{
    BLECONFIG_LOG("OnDisconnect");
    m_ota.onDisconnect();
    NimBLEDevice::startAdvertising();
    
    m_isDeviceConnected = false;
    if (m_pCallBacks != NULL)
        m_pCallBacks->onBluetoothConnection(m_isDeviceConnected);
}

// Disconnect the client
void BLEConfig::disconnectClient()
{
    if (m_pBLEServer != NULL)
    {
        for (uint16_t connHandle : m_pBLEServer->getPeerDevices())
        {
            BLECONFIG_LOG("Disconnecting client with connId %d", connHandle);
            m_pBLEServer->disconnect(connHandle);
        }
    }
}
