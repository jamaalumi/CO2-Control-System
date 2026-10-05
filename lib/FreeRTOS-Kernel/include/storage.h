#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

#include "app.h"

bool storage_load_settings(
ControllerSettings &settings
);

bool storage_save_settings(
const ControllerSettings &settings
);

#endif
