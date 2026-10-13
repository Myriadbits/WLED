///////////////////////////////////////////////////////////////////////////////
// BLEOtaMetadata
// Port of wled00/wled_metadata.cpp from WLED 0.16, see BLEOtaMetadata.h
///////////////////////////////////////////////////////////////////////////////

#include "BLEOtaMetadata.h"
#include "wled.h"

#include <type_traits>

namespace
{
    constexpr uint32_t WLED_CUSTOM_DESC_MAGIC = 0x57535453;  // "WSTS" (WLED System Tag Structure)
    constexpr uint32_t WLED_CUSTOM_DESC_VERSION = 1;

    // DJB2 hash, compile time variant (recursive, so not usable at runtime)
    constexpr uint32_t djb2_hash_constexpr(const char* str, uint32_t hash = 5381)
    {
        return (*str == '\0') ? hash : djb2_hash_constexpr(str + 1, ((hash << 5) + hash) + *str);
    }

    uint32_t djb2_hash_runtime(const char* str)
    {
        uint32_t hash = 5381;
        while (*str)
            hash = ((hash << 5) + hash) + *str++;
        return hash;
    }
}

#define BLEOTA_RELEASE_NAME TOSTRING(WLED_RELEASE_NAME)

static_assert(sizeof(BLEOTA_RELEASE_NAME) <= BLEOTA_RELEASE_NAME_MAX_LEN, "WLED_RELEASE_NAME is too long");

// Placed in the same section as WLED 0.16 does, so it ends up at an (almost) fixed offset at the start of the image
const bleota_metadata_t __attribute__((section(".rodata_custom_desc"))) BLEOTA_BUILD_DESCRIPTION = {
    WLED_CUSTOM_DESC_MAGIC,
    WLED_CUSTOM_DESC_VERSION,
    TOSTRING(WLED_VERSION),
    BLEOTA_RELEASE_NAME,
    std::integral_constant<uint32_t, djb2_hash_constexpr(BLEOTA_RELEASE_NAME)>::value,
};

bool findWledMetadata(const uint8_t* binaryData, size_t dataSize, bleota_metadata_t* extractedDesc)
{
    if (!binaryData || !extractedDesc || dataSize < sizeof(bleota_metadata_t))
        return false;

    for (size_t offset = 0; offset <= dataSize - sizeof(bleota_metadata_t); offset++)
    {
        uint32_t data_magic;
        memcpy(&data_magic, binaryData + offset, sizeof(data_magic));
        if (data_magic != WLED_CUSTOM_DESC_MAGIC)
            continue;

        bleota_metadata_t candidate;
        memcpy(&candidate, binaryData + offset, sizeof(candidate));
        if (candidate.desc_version != WLED_CUSTOM_DESC_VERSION)
            continue;
        candidate.release_name[BLEOTA_RELEASE_NAME_MAX_LEN - 1] = '\0';
        if (candidate.hash != djb2_hash_runtime(candidate.release_name))
            continue;

        *extractedDesc = candidate;
        return true;
    }
    return false;
}

bool shouldAllowOTA(const bleota_metadata_t& firmwareDescription, char* errorMessage, size_t errorMessageLen)
{
    if (errorMessage && errorMessageLen > 0)
        errorMessage[0] = '\0';

    char safeFirmwareRelease[BLEOTA_RELEASE_NAME_MAX_LEN];
    strncpy(safeFirmwareRelease, firmwareDescription.release_name, BLEOTA_RELEASE_NAME_MAX_LEN - 1);
    safeFirmwareRelease[BLEOTA_RELEASE_NAME_MAX_LEN - 1] = '\0';
    if (strlen(safeFirmwareRelease) == 0)
        return false;

    // Compare with our own metadata, this also keeps BLEOTA_BUILD_DESCRIPTION in the image
    if (strncmp(safeFirmwareRelease, BLEOTA_BUILD_DESCRIPTION.release_name, BLEOTA_RELEASE_NAME_MAX_LEN) != 0)
    {
        if (errorMessage && errorMessageLen > 0)
        {
            snprintf_P(errorMessage, errorMessageLen, PSTR("Firmware compatibility mismatch: current='%s', uploaded='%s'."),
                       BLEOTA_BUILD_DESCRIPTION.release_name, safeFirmwareRelease);
            errorMessage[errorMessageLen - 1] = '\0';
        }
        return false;
    }
    return true;
}
