#pragma once

///////////////////////////////////////////////////////////////////////////////
// BLEOtaMetadata
// Firmware build metadata, used to validate firmware images before an OTA update.
//
// Port of wled00/wled_metadata.h from WLED 0.16, which WLED 0.15 does not have.
// The structure, magic and location (.rodata_custom_desc) are identical, so:
// - devices running a WLED 0.16 based Cloxel firmware accept this firmware
// - this firmware accepts WLED 0.16 based Cloxel firmware
// as long as the release name (WLED_RELEASE_NAME) is the same.
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstddef>

#define BLEOTA_VERSION_MAX_LEN 48
#define BLEOTA_RELEASE_NAME_MAX_LEN 48

typedef struct {
    uint32_t magic;               // Magic number to identify WLED custom description
    uint32_t desc_version;        // Structure version for future compatibility
    char wled_version[BLEOTA_VERSION_MAX_LEN];
    char release_name[BLEOTA_RELEASE_NAME_MAX_LEN]; // Release name (null-terminated)
    uint32_t hash;                // Structure sanity check
} __attribute__((packed)) bleota_metadata_t;

// Metadata of this build
extern const bleota_metadata_t BLEOTA_BUILD_DESCRIPTION;

// Find the metadata in (the first part of) a firmware image
bool findWledMetadata(const uint8_t* binaryData, size_t dataSize, bleota_metadata_t* extractedDesc);

// Only allow firmware with the same release name as this build
bool shouldAllowOTA(const bleota_metadata_t& firmwareDescription, char* errorMessage, size_t errorMessageLen);
