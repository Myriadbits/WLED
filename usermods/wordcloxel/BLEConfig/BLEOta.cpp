///////////////////////////////////////////////////////////////////////////////
// BLEOta
// Firmware update over BLE, see BLEOta.h for the protocol
///////////////////////////////////////////////////////////////////////////////

#include "BLEOta.h"
#include "BLEConfigItemBase.h" // BLECONFIG_LOG

#include <Update.h>
#include "wled.h"

namespace
{
    void putU16(uint8_t* p, uint16_t v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; }
    void putU32(uint8_t* p, uint32_t v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; p[2] = (v >> 16) & 0xFF; p[3] = (v >> 24) & 0xFF; }
    uint32_t getU32(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
}

BLEOta::BLEOta()
{
    mbedtls_sha256_init(&m_shaContext);
}

BLEOta::~BLEOta()
{
    mbedtls_sha256_free(&m_shaContext);
    free(m_pWindow);
}

//
// Create the OTA service with its control and data characteristics
void BLEOta::createService(NimBLEServer* pServer, NimBLECharacteristicCallbacks* pCallbacks)
{
    NimBLEService* pService = pServer->createService(NimBLEUUID(BLEOTA_SERVICE_UUID));

    // NimBLE adds the CCCD (0x2902) for NOTIFY automatically
    m_pCharControl = pService->createCharacteristic(NimBLEUUID(BLEOTA_CHAR_CONTROL_UUID),
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    m_pCharControl->setCallbacks(pCallbacks);

    m_pCharData = pService->createCharacteristic(NimBLEUUID(BLEOTA_CHAR_DATA_UUID), NIMBLE_PROPERTY::WRITE_NR);
    m_pCharData->setCallbacks(pCallbacks);

    updateReadValue();
    pService->start();
    BLECONFIG_LOG("OTA service started");
}

bool BLEOta::isOtaCharacteristic(NimBLECharacteristic* pCharacteristic) const
{
    return pCharacteristic != nullptr && (pCharacteristic == m_pCharControl || pCharacteristic == m_pCharData);
}

//
// Incoming write on one of the OTA characteristics (BT task)
// Only parse and buffer here, the real work is done in loop()
void BLEOta::onWrite(NimBLECharacteristic* pCharacteristic)
{
    NimBLEAttValue value = pCharacteristic->getValue();
    const uint8_t* pdata = value.data();
    size_t len = value.size();

    if (pCharacteristic == m_pCharControl)
    {
        if (len < 1)
            return;
        switch (pdata[0])
        {
            case BLEOTA_CMD_START:
                if (len < 1 + 4 + 32)
                {
                    m_protocolError = true;
                    return;
                }
                m_pendingSize = getU32(pdata + 1);
                memcpy(m_pendingHash, pdata + 5, 32);
                m_pendingCommand = EPendingCommand::Start;
                break;
            case BLEOTA_CMD_FINISH:
                m_pendingCommand = EPendingCommand::Finish;
                break;
            case BLEOTA_CMD_ABORT:
                m_pendingCommand = EPendingCommand::Abort;
                break;
            default:
                m_protocolError = true;
                break;
        }
    }
    else if (pCharacteristic == m_pCharData)
    {
        if (m_state != EBLEOtaState::Receiving || len < 5)
            return; // Stale data after an abort, ignore

        // The app should wait for the ACK before sending the next window
        if (m_windowReady)
        {
            m_protocolError = true;
            return;
        }

        uint32_t offset = getU32(pdata);
        const uint8_t* ppayload = pdata + 4;
        size_t payloadLen = len - 4;
        m_lastDataTime = millis();

        if (offset != m_received)
        {
            // Drop the current window and let the app resume from the last written offset.
            // Only report once, the chunks still in flight are silently dropped.
            if (m_windowLen > 0 || m_received != m_written || !m_resyncing)
            {
                m_received = m_written;
                m_windowLen = 0;
                m_resyncing = true;
                m_offsetError = true;
            }
            return;
        }
        m_resyncing = false;

        if (m_windowLen + payloadLen > BLEOTA_WINDOW_SIZE || m_received + payloadLen > m_totalSize)
        {
            m_protocolError = true;
            return;
        }

        memcpy(m_pWindow + m_windowLen, ppayload, payloadLen);
        m_windowLen += payloadLen;
        m_received += payloadLen;

        if (m_windowLen == BLEOTA_WINDOW_SIZE || m_received == m_totalSize)
            m_windowReady = true;
    }
}

//
// Client disconnected (BT task), abort a running update
void BLEOta::onDisconnect()
{
    if (m_state == EBLEOtaState::Receiving)
        m_pendingCommand = EPendingCommand::Abort;
}

//
// Process pending work (main loop)
void BLEOta::loop()
{
    EPendingCommand cmd = m_pendingCommand.exchange(EPendingCommand::None);
    if (cmd == EPendingCommand::Abort)
    {
        if (m_state == EBLEOtaState::Receiving)
        {
            BLECONFIG_LOG("OTA aborted");
            abortUpdate();
        }
        return;
    }
    if (cmd == EPendingCommand::Start)
    {
        startUpdate();
        return;
    }

    if (m_state == EBLEOtaState::Done)
    {
        if (millis() > m_rebootTime)
        {
            BLECONFIG_LOG("OTA done, rebooting");
            doReboot = true; // Handled by WLED::loop()
        }
        return;
    }

    if (m_protocolError.exchange(false))
    {
        fail(BLEOTA_ERR_PROTOCOL, m_received);
        return;
    }

    if (m_state != EBLEOtaState::Receiving)
    {
        if (cmd == EPendingCommand::Finish)
            fail(BLEOTA_ERR_PROTOCOL);
        return;
    }

    if (m_windowReady)
        processWindow();

    if (m_state == EBLEOtaState::Receiving && m_offsetError.exchange(false))
    {
        BLECONFIG_LOG("OTA offset mismatch, resume from %u", m_written);
        uint8_t rsp[6] = {BLEOTA_RSP_ERROR, BLEOTA_ERR_OFFSET};
        putU32(rsp + 2, m_written);
        notifyControl(rsp, sizeof(rsp));
    }

    if (m_state == EBLEOtaState::Receiving && cmd == EPendingCommand::Finish)
        finishUpdate();

    if (m_state == EBLEOtaState::Receiving && !m_windowReady && (millis() - m_lastDataTime) > BLEOTA_DATA_TIMEOUT)
        fail(BLEOTA_ERR_TIMEOUT, m_written);
}

uint8_t BLEOta::getProgress() const
{
    if (m_state == EBLEOtaState::Done)
        return 255;
    if (m_totalSize == 0)
        return 0;
    return (uint8_t)(((uint64_t) m_written * 255) / m_totalSize);
}

//
// START received: prepare the OTA partition
void BLEOta::startUpdate()
{
    if (m_state == EBLEOtaState::Receiving)
        abortUpdate(); // Restart

    if (m_state == EBLEOtaState::Done)
    {
        fail(BLEOTA_ERR_BUSY);
        return;
    }

    uint32_t size = m_pendingSize;
    BLECONFIG_LOG("OTA start, %u bytes, %u available", size, ESP.getFreeSketchSpace());
    if (size == 0 || size > ESP.getFreeSketchSpace())
    {
        fail(BLEOTA_ERR_SIZE, ESP.getFreeSketchSpace());
        return;
    }

    if (m_pWindow == nullptr)
        m_pWindow = (uint8_t*) malloc(BLEOTA_WINDOW_SIZE);
    if (m_pWindow == nullptr)
    {
        fail(BLEOTA_ERR_BEGIN, getFreeHeapSize());
        return;
    }

    // Same preparation as the WLED web OTA (see ota_update.cpp)
    #if WLED_WATCHDOG_TIMEOUT > 0
    WLED::instance().disableWatchdog();
    #endif
    UsermodManager::onUpdateBegin(true);
    backupConfig();

    if (!Update.begin(size))
    {
        BLECONFIG_LOG("OTA begin failed: %s", Update.errorString());
        uint8_t error = Update.getError();
        abortUpdate();
        fail(BLEOTA_ERR_BEGIN, error);
        return;
    }

    mbedtls_sha256_init(&m_shaContext);
    mbedtls_sha256_starts(&m_shaContext, 0); // 0 = SHA256

    m_totalSize = size;
    memcpy(m_expectedHash, m_pendingHash, sizeof(m_expectedHash));
    m_received = 0;
    m_written = 0;
    m_windowLen = 0;
    m_resyncing = false;
    m_windowReady = false;
    m_offsetError = false;
    m_protocolError = false;
    m_lastDataTime = millis();
    m_state = EBLEOtaState::Receiving; // Last, the BT task accepts data from now on

    uint8_t rsp[3] = {BLEOTA_RSP_READY};
    putU16(rsp + 1, BLEOTA_WINDOW_SIZE);
    notifyControl(rsp, sizeof(rsp));
}

//
// A complete window has been received, write it to flash
void BLEOta::processWindow()
{
    size_t len = m_windowLen;

    // The first window contains the WLED metadata, only accept firmware for this release
    if (m_written == 0)
    {
        wled_metadata_t desc;
        char errorMessage[128] = {0};
        bool ok = findWledMetadata(m_pWindow, len, &desc);
        if (ok)
            ok = shouldAllowOTA(desc, errorMessage, sizeof(errorMessage));
        if (!ok)
        {
            BLECONFIG_LOG("OTA declined: %s", errorMessage[0] ? errorMessage : "no metadata found");
            fail(BLEOTA_ERR_VALIDATION);
            return;
        }
    }

    if (Update.write(m_pWindow, len) != len)
    {
        BLECONFIG_LOG("OTA write failed at %u: %s", m_written, Update.errorString());
        fail(BLEOTA_ERR_WRITE, m_written);
        return;
    }
    mbedtls_sha256_update(&m_shaContext, m_pWindow, len);

    m_written += len;
    m_windowLen = 0;
    m_lastDataTime = millis();
    m_windowReady = false; // Last, the BT task accepts data from now on

    uint8_t rsp[5] = {BLEOTA_RSP_ACK};
    putU32(rsp + 1, m_written);
    notifyControl(rsp, sizeof(rsp));
}

//
// FINISH received: verify the image and activate it
void BLEOta::finishUpdate()
{
    if (m_windowReady || m_written != m_totalSize)
    {
        fail(BLEOTA_ERR_PROTOCOL, m_written);
        return;
    }

    uint8_t hash[32];
    mbedtls_sha256_finish(&m_shaContext, hash);
    mbedtls_sha256_free(&m_shaContext);
    if (memcmp(hash, m_expectedHash, sizeof(hash)) != 0)
    {
        BLECONFIG_LOG("OTA hash mismatch");
        fail(BLEOTA_ERR_HASH);
        return;
    }

    if (!Update.end(true))
    {
        BLECONFIG_LOG("OTA end failed: %s", Update.errorString());
        fail(BLEOTA_ERR_END, Update.getError());
        return;
    }

    BLECONFIG_LOG("OTA success, %u bytes written", m_written);
    m_state = EBLEOtaState::Done;
    m_rebootTime = millis() + BLEOTA_REBOOT_DELAY;
    free(m_pWindow);
    m_pWindow = nullptr;

    uint8_t rsp[1] = {BLEOTA_RSP_DONE};
    notifyControl(rsp, sizeof(rsp));
}

//
// Stop a running update and restore normal operation
void BLEOta::abortUpdate()
{
    if (Update.isRunning())
        Update.abort();
    mbedtls_sha256_free(&m_shaContext);

    m_state = EBLEOtaState::Idle;
    m_windowReady = false;
    m_windowLen = 0;
    free(m_pWindow);
    m_pWindow = nullptr;

    UsermodManager::onUpdateBegin(false);
    #if WLED_WATCHDOG_TIMEOUT > 0
    WLED::instance().enableWatchdog();
    #endif
    updateReadValue();
}

//
// Report an error to the app, a running update is aborted
void BLEOta::fail(uint8_t errorCode, uint32_t value)
{
    BLECONFIG_LOG("OTA error %u (%u)", errorCode, value);
    if (m_state == EBLEOtaState::Receiving)
        abortUpdate();

    uint8_t rsp[6] = {BLEOTA_RSP_ERROR, errorCode};
    putU32(rsp + 2, value);
    notifyControl(rsp, sizeof(rsp));
}

//
// Send a notification on the control characteristic, then restore the read value
void BLEOta::notifyControl(const uint8_t* pdata, size_t len)
{
    if (m_pCharControl == nullptr)
        return;
    m_pCharControl->setValue((uint8_t*) pdata, len);
    m_pCharControl->notify();
    updateReadValue();
}

void BLEOta::updateReadValue()
{
    if (m_pCharControl == nullptr)
        return;
    uint8_t value[6] = {BLEOTA_PROTOCOL_VERSION, (uint8_t) m_state};
    putU32(value + 2, m_written);
    m_pCharControl->setValue(value, sizeof(value));
}
