/**
 * @file my_state_machine.c
 */

#include <zephyr/smf.h>

#include "LED.h"
#include "BTN.h"
#include "my_state_machine.h"

/**
 * Function Prototypes
 */
static void state_0_entry(void* o);
static enum smf_state_result state_0_run(void* o);
static void state_1_entry(void* o);
static enum smf_state_result state_1_run(void* o);
static void state_2_entry(void* o);
static enum smf_state_result state_2_run(void* o);
static void state_3_entry(void* o);
static enum smf_state_result state_3_run(void* o);
static void state_4_entry(void* o);
static enum smf_state_result state_4_run(void* o);

/**
 * Typedefs
 */
enum state_machine_states {
    STATE_0,
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4
};

typedef struct {
    // Context variable used by zephyr to track state machine state. Must be first
    struct smf_ctx ctx;

    uint16_t count;
} state_object_t;

/**
 * Local Variables
 */
static const struct smf_state states[] = {
    [STATE_0] = SMF_CREATE_STATE(state_0_entry, state_0_run, NULL, NULL, NULL),
    [STATE_1] = SMF_CREATE_STATE(state_1_entry, state_1_run, NULL, NULL, NULL),
    [STATE_2] = SMF_CREATE_STATE(state_2_entry, state_2_run, NULL, NULL, NULL),
    [STATE_3] = SMF_CREATE_STATE(state_3_entry, state_3_run, NULL, NULL, NULL),
    [STATE_4] = SMF_CREATE_STATE(state_4_entry, state_4_run, NULL, NULL, NULL),    
};

static state_object_t state_object;

void state_machine_init() {
    state_object.count = 0;
    smf_set_initial(SMF_CTX(&state_object), &states[STATE_0]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&state_object));
}

static void state_0_entry(void* o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result state_0_run(void* o) {
    if (BTN_check_clear_pressed(BTN0)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_1]);
    }

    return SMF_EVENT_HANDLED;
}

static void state_1_entry(void* o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result state_1_run(void* o) {
    if (state_object.count > 125) {
        state_object.count = 0;
        LED_toggle(LED0);
    } else {
        state_object.count++;
    }
    if (BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_0]);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_4]);
    }  

    return SMF_EVENT_HANDLED;
}

static void state_2_entry(void* o) {
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_ON);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result state_2_run(void* o) {
    if (state_object.count > 1000) {
        state_object.count = 0;
        smf_set_state(SMF_CTX(&state_object), &states[STATE_3]);
    } else {
        state_object.count++;    
    }
    if (BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_0]);
    }  

    return SMF_EVENT_HANDLED;
}

static void state_3_entry(void* o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_ON);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_ON);
}

static enum smf_state_result state_3_run(void* o) {
    if (state_object.count > 2000) {
        state_object.count = 0;
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    } else {
        state_object.count++;    
    }
    if (BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_0]);
    }  

    return SMF_EVENT_HANDLED;
}

static void state_4_entry(void* o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result state_4_run(void* o) {
    if (state_object.count > 62) {
        state_object.count = 0;
        LED_toggle(LED0);
        LED_toggle(LED1);
        LED_toggle(LED2);
        LED_toggle(LED3);
    } else {
        state_object.count++;
    }
    if (BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_0]);
    }  

    return SMF_EVENT_HANDLED;
}