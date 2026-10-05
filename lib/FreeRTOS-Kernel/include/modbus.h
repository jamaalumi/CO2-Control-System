#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include <stdbool.h>

void modbus_init();

bool modbus_read_holding_registers(
uint8_t address,
uint16_t register_address,
uint16_t *data,
uint16_t count
);

bool modbus_write_single_register(
uint8_t address,
uint16_t register_address,
uint16_t value
);

bool produal_set_fan_percent(uint8_t percent);

bool produal_read_fan_pulse_counter(uint32_t &count);

bool gmp252_read_co2(uint16_t &co2_ppm);

bool hmp60_read_environment(
float &temperature_c,
float &humidity_percent
);

#endif
