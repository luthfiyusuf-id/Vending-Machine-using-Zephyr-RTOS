#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "button.h"

typedef enum {
    STATE_IDLE,
    STATE_CREDIT,
    STATE_DISPENSE,
    STATE_COMPLETE
} vending_state_t;

int state_machine_post_event(button_event_t event);

void state_machine_run(void);

#endif