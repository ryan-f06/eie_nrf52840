/**
 * @file my_state_machine.c
 */

#include <zephyr/smf.h>
#include <zephyr/sys/printk.h>

#include "LED.h"
#include "BTN.h"
#include "my_state_machine.h"

/**
 * Function Prototypes
 */
static void start_up_entry(void* o);
static enum smf_state_result start_up_run(void* o);
static void set_combo_entry(void* o);
static enum smf_state_result set_combo_run(void* o);
static void inputting_entry(void* o);
static enum smf_state_result inputting_run(void* o);
static void waiting_entry(void* o);
static enum smf_state_result waiting_run(void* o);

/**
 * Typedefs
 */
enum state_machine_states {
    START_UP,
    SET_COMBO,
    INPUTTING,
    WAITING
};

typedef struct {
    // Context variable used by zephyr to track state machine state. Must be first
    struct smf_ctx ctx;

    uint16_t count;
    uint16_t combo; //BUTTON 0 = 00, BUTTON 1 = 01, BUTTON 2 = 01
    uint16_t correct_combo; // 0b01000100 (1-0-2-0)
} state_object_t;

/**
 * Local Variables
 */

static const struct smf_state states[] = {
    [START_UP] = SMF_CREATE_STATE(start_up_entry, start_up_run, NULL, NULL, NULL),
    [SET_COMBO] = SMF_CREATE_STATE(set_combo_entry, set_combo_run, NULL, NULL, NULL),
    [INPUTTING] = SMF_CREATE_STATE(inputting_entry, inputting_run, NULL, NULL, NULL),
    [WAITING] = SMF_CREATE_STATE(waiting_entry, waiting_run, NULL, NULL, NULL)    
};

static state_object_t state_object;

void state_machine_init() {
    state_object.count = 0;
    state_object.correct_combo = 68; // 0b01000100 (1-0-2-0) default combo
    smf_set_initial(SMF_CTX(&state_object), &states[START_UP]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&state_object));
}

static void start_up_entry(void* o) {
    printk("start up\n");
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_ON); 
}

static enum smf_state_result start_up_run(void* o) {
    if (state_object.count > 3000){
        state_object.count = 0;
        smf_set_initial(SMF_CTX(&state_object), &states[INPUTTING]);     
    } else {
        state_object.count++;
    }
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        smf_set_initial(SMF_CTX(&state_object), &states[SET_COMBO]); 
    }

    return SMF_EVENT_HANDLED;
}

static void set_combo_entry(void* o) {
    printk("set combo\n");
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_ON);
    LED_set(LED2, LED_ON);
    LED_set(LED3, LED_ON);
    state_object.correct_combo = 0; 
}

static enum smf_state_result set_combo_run(void* o) {
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        state_object.correct_combo = state_object.correct_combo >> 2; //shift back into place
        smf_set_initial(SMF_CTX(&state_object), &states[INPUTTING]); 
    }
    if (BTN_check_clear_pressed(BTN0)) {
        state_object.correct_combo += 0;
        state_object.correct_combo = state_object.correct_combo << 2;
    }
    if (BTN_check_clear_pressed(BTN1)) {
        state_object.correct_combo += 1;
        state_object.correct_combo = state_object.correct_combo << 2;
    }
    if (BTN_check_clear_pressed(BTN2)) {
        state_object.correct_combo += 2;
        state_object.correct_combo = state_object.correct_combo << 2;
    }

    return SMF_EVENT_HANDLED;
}

static void inputting_entry(void* o) {
    printk("inputting\n");
    state_object.combo = 0;
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result inputting_run(void* o) {
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        state_object.combo = state_object.combo >> 2; //shift back into place
        smf_set_state(SMF_CTX(&state_object), &states[WAITING]);
    }
    if (BTN_check_clear_pressed(BTN0)) {
        state_object.combo += 0;
        state_object.combo = state_object.combo << 2;
    }
    if (BTN_check_clear_pressed(BTN1)) {
        state_object.combo += 1;
        state_object.combo = state_object.combo << 2;
    }
    if (BTN_check_clear_pressed(BTN2)) {
        state_object.combo += 2;
        state_object.combo = state_object.combo << 2;
    }

    return SMF_EVENT_HANDLED;
}

static void waiting_entry(void* o) {
    printk("enterd combo: %d\n", state_object.combo);
    printk("correct combo: %d\n", state_object.correct_combo);
    if (state_object.combo == state_object.correct_combo) {         
        LED_set(LED0, LED_ON);
        LED_set(LED1, LED_ON);
        LED_set(LED2, LED_ON);
        LED_set(LED3, LED_ON);
    } else {
        LED_set(LED0, LED_OFF);
        LED_set(LED1, LED_OFF);
        LED_set(LED2, LED_OFF);
        LED_set(LED3, LED_OFF);
    }
}

static enum smf_state_result waiting_run(void* o) {
    if (state_object.count > 1000){
        state_object.count = 0;
        LED_toggle(LED3); 
    } else {
        state_object.count++;
    }
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        state_object.count = 0;
        smf_set_state(SMF_CTX(&state_object), &states[INPUTTING]); //LOCK
    }

    return SMF_EVENT_HANDLED;
}