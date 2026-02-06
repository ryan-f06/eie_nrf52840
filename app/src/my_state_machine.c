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
static void STATE_1_entry(void* o);
static enum smf_state_result STATE_1_run(void* o);
static void STATE_2_entry(void* o);
static enum smf_state_result STATE_2_run(void* o);
static void STATE_3_entry(void* o);
static enum smf_state_result STATE_3_run(void* o);
static void STATE_4_entry(void* o);
static enum smf_state_result STATE_4_run(void* o);

/**
 * Typedefs
 */
enum state_machine_states {
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4
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
    [STATE_1] = SMF_CREATE_STATE(STATE_1_entry, STATE_1_run, NULL, NULL, NULL),
    [STATE_2] = SMF_CREATE_STATE(STATE_2_entry, STATE_2_run, NULL, NULL, NULL),
    [STATE_3] = SMF_CREATE_STATE(STATE_3_entry, STATE_3_run, NULL, NULL, NULL),
    [STATE_4] = SMF_CREATE_STATE(STATE_4_entry, STATE_4_run, NULL, NULL, NULL)    
};

static state_object_t state_object;

void state_machine_init() {
    state_object.count = 0;
    state_object.correct_combo = 68; // 0b01000100 (1-0-2-0) default combo
    smf_set_initial(SMF_CTX(&state_object), &states[STATE_1]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&state_object));
}

static void STATE_1_entry(void* o) {
    printk("start up\n");
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_ON); 
}

static enum smf_state_result STATE_1_run(void* o) {
    if (state_object.count > 3000){
        state_object.count = 0;
        smf_set_initial(SMF_CTX(&state_object), &states[STATE_3]);     
    } else {
        state_object.count++;
    }
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        smf_set_initial(SMF_CTX(&state_object), &states[STATE_2]); 
    }

    return SMF_EVENT_HANDLED;
}

static void STATE_2_entry(void* o) {
    printk("set combo\n");
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_ON);
    LED_set(LED2, LED_ON);
    LED_set(LED3, LED_ON);
    state_object.correct_combo = 0; 
}

static enum smf_state_result STATE_2_run(void* o) {
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        state_object.correct_combo = state_object.correct_combo >> 2; //shift back into place
        smf_set_initial(SMF_CTX(&state_object), &states[STATE_3]); 
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

static void STATE_3_entry(void* o) {
    printk("STATE_3\n");
    state_object.combo = 0;
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
}

static enum smf_state_result STATE_3_run(void* o) {
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        state_object.combo = state_object.combo >> 2; //shift back into place
        smf_set_state(SMF_CTX(&state_object), &states[STATE_4]);
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

static void STATE_4_entry(void* o) {
    printk("enterd combo: %d\n", state_object.combo);
    printk("correct combo: %d\n", state_object.correct_combo);
    if (state_object.combo == state_object.correct_combo) { //on = correct    
        LED_set(LED0, LED_ON);
        LED_set(LED1, LED_ON);
        LED_set(LED2, LED_ON);
        LED_set(LED3, LED_ON);
    } else { //on = incorrect 
        LED_set(LED0, LED_OFF);
        LED_set(LED1, LED_OFF);
        LED_set(LED2, LED_OFF);
        LED_set(LED3, LED_OFF);
    }
}

static enum smf_state_result STATE_4_run(void* o) {
    if (state_object.count > 1000){
        state_object.count = 0;
        LED_toggle(LED3); 
    } else {
        state_object.count++;
    }
    if (BTN_check_clear_pressed(BTN3)) { //ENTER
        state_object.count = 0;
        smf_set_state(SMF_CTX(&state_object), &states[STATE_3]); //LOCK
    }

    return SMF_EVENT_HANDLED;
}