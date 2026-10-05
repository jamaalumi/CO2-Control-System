#include "storage.h"

#include "config.h"

#include "hardware/flash.h"
#include "hardware/sync.h"

#include <cstring>

namespace
{
constexpr uint32_t FLASH_TARGET_OFFSET =
(2 * 1024 * 1024)
- FLASH_SECTOR_SIZE;


struct StoredSettings
{
    uint32_t magic;

    uint16_t co2_setpoint_ppm;

    uint16_t reserved;
};


}

bool storage_load_settings(
ControllerSettings &settings
)
{
const uint8_t *flash_data =
reinterpret_cast<
const uint8_t *
>(
XIP_BASE +
FLASH_TARGET_OFFSET
);


StoredSettings stored;

std::memcpy(
    &stored,
    flash_data,
    sizeof(stored)
);

if (
    stored.magic !=
    Config::SETTINGS_MAGIC
)
{
    return false;
}

if (
    stored.co2_setpoint_ppm == 0 ||
    stored.co2_setpoint_ppm >
    Config::MAX_CO2_SETPOINT_PPM
)
{
    return false;
}

settings.co2_setpoint_ppm =
    stored.co2_setpoint_ppm;

return true;


}

bool storage_save_settings(
const ControllerSettings &settings
)
{
if (
settings.co2_setpoint_ppm == 0 ||
settings.co2_setpoint_ppm >
Config::MAX_CO2_SETPOINT_PPM
)
{
return false;
}


StoredSettings stored = {
    Config::SETTINGS_MAGIC,
    settings.co2_setpoint_ppm,
    0
};

uint8_t buffer[
    FLASH_SECTOR_SIZE
];

std::memset(
    buffer,
    0xFF,
    sizeof(buffer)
);

std::memcpy(
    buffer,
    &stored,
    sizeof(stored)
);

uint32_t interrupts =
    save_and_disable_interrupts();

flash_range_erase(
    FLASH_TARGET_OFFSET,
    FLASH_SECTOR_SIZE
);

flash_range_program(
    FLASH_TARGET_OFFSET,
    buffer,
    FLASH_SECTOR_SIZE
);

restore_interrupts(
    interrupts
);

return true;


}
