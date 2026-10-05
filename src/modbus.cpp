#include "modbus.h"
#include "config.h"

#include "hardware/uart.h"
#include "hardware/gpio.h"

#include <iostream>

static void rs485_transmit_mode(
bool transmit
)
{
gpio_put(
Config::RS485_DE_PIN,
transmit ? 1 : 0
);
}

void modbus_init()
{
uart_init(
Config::MODBUS_UART,
Config::MODBUS_BAUD
);

```
gpio_set_function(
    Config::MODBUS_TX_PIN,
    GPIO_FUNC_UART
);

gpio_set_function(
    Config::MODBUS_RX_PIN,
    GPIO_FUNC_UART
);

gpio_init(Config::RS485_DE_PIN);

gpio_set_dir(
    Config::RS485_DE_PIN,
    GPIO_OUT
);

rs485_transmit_mode(false);

std::cout
    << "Modbus UART initialized at "
    << Config::MODBUS_BAUD
    << " baud.\n";
```

}

bool modbus_read_holding_registers(
uint8_t address,
uint16_t register_address,
uint16_t *data,
uint16_t count
)
{
/*
* Modbus RTU implementation will go here.
*
* Do not invent device register addresses.
*/

```
(void)address;
(void)register_address;
(void)data;
(void)count;

return false;
```

}

bool modbus_write_single_register(
uint8_t address,
uint16_t register_address,
uint16_t value
)
{
/*
* Modbus RTU write implementation.
*/

```
(void)address;
(void)register_address;
(void)value;

return false;
```

}

bool produal_set_fan_percent(
uint8_t percent
)
{
if (percent > 100)
percent = 100;

```
/*
 * Produal AO1:
 *
 * 0 V  = 0 %
 * 10 V = 100 %
 *
 * Exact register still needs to be confirmed.
 */

(void)percent;

return false;
```

}

bool produal_read_fan_pulse_counter(
uint32_t &count
)
{
/*
* Produal AI1 counter.
*
* Counter clears after Modbus read.
*/

```
count = 0;

return false;
```

}

bool gmp252_read_co2(
uint16_t &co2_ppm
)
{
/*
* Vaisala GMP252.
*
* Exact Modbus register needs to be confirmed.
*/

```
co2_ppm = 0;

return false;
```

}

bool hmp60_read_environment(
float &temperature_c,
float &humidity_percent
)
{
/*
* Vaisala HMP60.
*
* Exact Modbus registers need to be confirmed.
*/

```
temperature_c = 0.0f;

humidity_percent = 0.0f;

return false;
```

}
