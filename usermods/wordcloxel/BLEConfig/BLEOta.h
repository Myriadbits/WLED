/*  BLEOta
    Firmware update over BLE for the BLEConfig tool

    Protocol (all values little-endian)
    -----------------------------------
    Control characteristic (write + notify + read):
      App -> device:
        0x01 START  [u32 size][32 bytes SHA-256 of the image]
        0x02 FINISH
        0x03 ABORT
      Device -> app (notify):
        0x81 READY  [u16 window size]
        0x82 ACK    [u32 bytes written to flash]
        0x83 DONE   (image verified, device restarts)
        0x84 ERROR  [u8 error code][u32 value]
      Read: [u8 protocol version][u8 state][u32 bytes written]

    Data characteristic (write without response):
      [u32 offset][payload]
      The app sends one window (BLEOTA_WINDOW_SIZE bytes, or the rest of the image) and
      then waits for the ACK before it sends the next window. When a chunk arrives with an
      unexpected offset the current window is discarded and an ERROR (BLEOTA_ERR_OFFSET)
      with the offset to resume from is sent.

    Flash writes are done from loop() and not in the BLE callbacks, so the BT stack is
    never blocked by flash erases.
*/

#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <atomic>
#include <mbedtls/sha256.h>

#define BLEOTA_SERVICE_UUID         "ebd7f0a0-04a0-4f9c-96f3-05644d494f54"
#define BLEOTA_CHAR_CONTROL_UUID    "ebd7f0a1-04a0-4f9c-96f3-05644d494f54"
#define BLEOTA_CHAR_DATA_UUID       "ebd7f0a2-04a0-4f9c-96f3-05644d494f54"

#define BLEOTA_PROTOCOL_VERSION     1
#define BLEOTA_WINDOW_SIZE          4096 // Must hold the WLED metadata (offset 256 + 512 search range)
#define BLEOTA_DATA_TIMEOUT         20000 // ms without data before the update is aborted
#define BLEOTA_REBOOT_DELAY         1500 // ms between DONE notification and the restart

// Commands (app -> device)
#define BLEOTA_CMD_START            0x01
#define BLEOTA_CMD_FINISH           0x02
#define BLEOTA_CMD_ABORT            0x03

// Responses (device -> app)
#define BLEOTA_RSP_READY            0x81
#define BLEOTA_RSP_ACK              0x82
#define BLEOTA_RSP_DONE             0x83
#define BLEOTA_RSP_ERROR            0x84

// Error codes
#define BLEOTA_ERR_BUSY             1 // Update already running / window not processed yet
#define BLEOTA_ERR_SIZE             2 // Image does not fit in the OTA partition
#define BLEOTA_ERR_BEGIN            3 // Update.begin() failed
#define BLEOTA_ERR_OFFSET           4 // Unexpected offset, value = offset to resume from
#define BLEOTA_ERR_WRITE            5 // Flash write failed
#define BLEOTA_ERR_VALIDATION       6 // Image is not a firmware for this device (WLED release name)
#define BLEOTA_ERR_HASH             7 // SHA-256 mismatch
#define BLEOTA_ERR_END              8 // Update.end() failed
#define BLEOTA_ERR_PROTOCOL         9 // Unexpected command/data for the current state
#define BLEOTA_ERR_TIMEOUT          10 // No data received for BLEOTA_DATA_TIMEOUT ms

enum class EBLEOtaState : uint8_t
{
    Idle = 0,
    Receiving,
    Done,
};

class BLEOta
{
public:
    BLEOta();
    ~BLEOta();

    // Create the OTA service on the given server, writes are delivered to pCallbacks
    // (which should forward them to onWrite)
    void    createService(NimBLEServer* pServer, NimBLECharacteristicCallbacks* pCallbacks);

    // Returns true when the characteristic belongs to the OTA service
    bool    isOtaCharacteristic(NimBLECharacteristic* pCharacteristic) const;

    // BLE callbacks (called from the BT task)
    void    onWrite(NimBLECharacteristic* pCharacteristic);
    void    onDisconnect();

    // Process pending work, call this from the main loop
    void    loop();

    bool    isActive() const { return m_state != EBLEOtaState::Idle; }
    uint8_t getProgress() const; // 0-255

private:
    void    startUpdate();
    void    processWindow();
    void    finishUpdate();
    void    abortUpdate();
    void    fail(uint8_t errorCode, uint32_t value = 0);
    void    notifyControl(const uint8_t* pdata, size_t len);
    void    updateReadValue();

private:
    enum class EPendingCommand : uint8_t
    {
        None = 0,
        Start,
        Finish,
        Abort,
    };

    NimBLECharacteristic*  m_pCharControl {nullptr};
    NimBLECharacteristic*  m_pCharData {nullptr};

    // Shared between the BT task and the loop
    std::atomic<EPendingCommand>    m_pendingCommand {EPendingCommand::None};
    std::atomic<bool>               m_windowReady {false};
    std::atomic<bool>               m_offsetError {false};
    std::atomic<bool>               m_protocolError {false};
    volatile EBLEOtaState           m_state {EBLEOtaState::Idle};
    volatile uint32_t               m_received {0}; // Bytes received (including the current window)
    volatile uint32_t               m_written {0}; // Bytes written to flash
    volatile unsigned long          m_lastDataTime {0};
    volatile bool                   m_resyncing {false}; // Dropping chunks until the resume offset arrives

    uint8_t*            m_pWindow {nullptr};
    volatile size_t     m_windowLen {0};

    uint32_t            m_pendingSize {0};
    uint8_t             m_pendingHash[32] {0};
    uint32_t            m_totalSize {0};
    uint8_t             m_expectedHash[32] {0};
    mbedtls_sha256_context m_shaContext;
    unsigned long       m_rebootTime {0};
};
