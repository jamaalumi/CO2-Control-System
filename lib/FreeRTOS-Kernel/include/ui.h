#ifndef UI_H
#define UI_H

#include <stdint.h>

enum class UiEventType : uint8_t
{
BUTTON_PRESSED = 1
};

struct UiEvent
{
UiEventType type;
};

void ui_init();

void ui_task(void *parameter);

#endif
