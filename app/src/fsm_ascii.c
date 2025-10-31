#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/smf.h>

#include "BTN.h"
#include "LED.h"

#include "fsm_ascii.h"

/**
 * Function Prototypes
 */

static void input_1_state_entry(void *o);
static enum smf_state_result input_1_state_run(void *o);
static void input_2_state_entry(void *o);
static enum smf_state_result input_2_state_run(void *o);
static void send_state_entry(void *o);
static enum smf_state_result send_state_run(void *o);
static void standby_state_entry(void *o);
static enum smf_state_result standby_state_run(void *o);

/**
 * Typedefs
 */

enum state_machine_states {
    INPUT_1,
    INPUT_2,
    SEND,
    STANDBY
};

typedef struct {
    struct smf_ctx ctx;

    //uint16_t count;
    int code;
    char saved_codes[3];
} state_object_t;

/**
 * Local Variables
 */

static const struct smf_state states[] = {
    [INPUT_1] = SMF_CREATE_STATE(input_1_state_entry, input_1_state_run, NULL, NULL, NULL),
    [INPUT_2] = SMF_CREATE_STATE(input_2_state_entry, input_2_state_run, NULL, NULL, NULL),
    [SEND] = SMF_CREATE_STATE(send_state_entry, send_state_run, NULL, NULL, NULL),
    [STANDBY] = SMF_CREATE_STATE(standby_state_entry, standby_state_run, NULL, NULL, NULL)
};

static state_object_t state_object;

void state_machine_init() {
    //state_object.count = 0;
    smf_set_initial(SMF_CTX(&state_object), &states[INPUT_1]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&state_object));
}


static void input_1_state_entry(void *o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_blink(LED3, LED_1HZ);
    state_object.saved_codes[0] = 0;
    state_object.saved_codes[1] = 0;
    state_object.code = 00000000;
}
static enum smf_state_result input_1_state_run(void *o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    
    if (BTN_check_clear_pressed(BTN0)) { // adds a 1 to code
        LED_set(LED0, LED_ON);
        state_object.code += 1;
        state_object.code = state_object.code << 1;
    }
    if (BTN_check_clear_pressed(BTN1)) { // adds a 0 to code
        LED_set(LED1, LED_ON);
        state_object.code = state_object.code << 1;
    }
    if (BTN_check_clear_pressed(BTN2)) { //reset entry
        smf_set_state(SMF_CTX(&state_object), &states[INPUT_1]);
    }
    if (BTN_check_clear_pressed(BTN3)) {
        state_object.code = state_object.code >> 1;
        state_object.saved_codes[0] = state_object.code;
        smf_set_state(SMF_CTX(&state_object), &states[INPUT_2]);
    }

    return SMF_EVENT_HANDLED;
}


static void input_2_state_entry(void *o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_blink(LED3, LED_4HZ);
    state_object.code = 00000000;
}
static enum smf_state_result input_2_state_run(void *o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    
    if (BTN_check_clear_pressed(BTN0)) { // adds a 1 to code
        LED_set(LED0, LED_ON);
        state_object.code += 1;
        state_object.code = state_object.code << 1;
    }
    if (BTN_check_clear_pressed(BTN1)) { // adds a 0 to code
        LED_set(LED1, LED_ON);
        state_object.code = state_object.code << 1;
    }
    if (BTN_check_clear_pressed(BTN2)) { //reset entry
        smf_set_state(SMF_CTX(&state_object), &states[INPUT_1]);
    }
    if (BTN_check_clear_pressed(BTN3)) {
        state_object.code = state_object.code >> 1;
        state_object.saved_codes[1] = state_object.code;
        smf_set_state(SMF_CTX(&state_object), &states[SEND]);
    }

    return SMF_EVENT_HANDLED;
}


static void send_state_entry(void *o) {
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_blink(LED3, LED_16HZ);
}
static enum smf_state_result send_state_run(void *o){
    if (BTN_check_clear_pressed(BTN2)) { //reset entry
        smf_set_state(SMF_CTX(&state_object), &states[INPUT_1]);
    }
    if (BTN_check_clear_pressed(BTN3)) {
        printk("%s", state_object.saved_codes);
    }

    return SMF_EVENT_HANDLED;
}


static void standby_state_entry(void *o) {

}
static enum smf_state_result standby_state_run(void *o) {
    return SMF_EVENT_HANDLED;
}

