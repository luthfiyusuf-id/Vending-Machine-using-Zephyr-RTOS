#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef enum {
    EVENT_COIN_500,
    EVENT_COIN_1000,
    EVENT_COIN_2000,
    EVENT_PRODUCT
} button_event_t;

int button_init(void);

int button_read_event(button_event_t *event);

#endif