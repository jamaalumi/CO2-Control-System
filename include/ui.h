#ifndef UI_H
#define UI_H

#include "FreeRTOS.h"
#include "task.h"

void ui_init();

void ui_task(void *parameter);

#endif
