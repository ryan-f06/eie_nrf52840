/**
 * @file my_state_machine.c
 */

#include <zephyr/smf.h>
#include <zephyr/sys/printk.h>

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>

#include "lv_data_obj.h"

#include "LED.h"
#include "BTN.h"
#include "my_state_machine.h"

static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t *screen = NULL;

extern const lv_image_dsc_t circuit;
extern const lv_image_dsc_t grid;


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
    uint16_t player; //1 = X, 2 = O
    uint16_t score1;
    uint16_t score2;
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

void lv_button_callback(lv_event_t *event) {
  lv_obj_t *data_obj = (lv_obj_t *)lv_event_get_user_data(event);
  char label_text[2];
  if (state_object.player == 1){
    snprintf(label_text, 2, "X");
    lv_label_set_text(data_obj, label_text);
    state_object.player = 2; //switch to O
  }
  else if (state_object.player == 2) {
    snprintf(label_text, 2, "O");
    lv_label_set_text(data_obj, label_text);
    state_object.player = 1; //switch to X
  }
  else if (state_object.player == 3) {
    snprintf(label_text, 2, "-");
    lv_label_set_text(data_obj, label_text);
  }
  

}

void state_machine_init() {
    state_object.count = 0;
    state_object.score1 = 0;
    state_object.score2 = 0;
    state_object.player = 0;

    // if (!device_is_ready(display_dev)) { relocate later
    //     return 0;
    // }
    // screen = lv_screen_active();
    // if (screen == NULL) {
    //     return 0;
    // }
    device_is_ready(display_dev);
    screen = lv_screen_active();

    lv_obj_t *image = lv_image_create(screen);
    lv_image_set_src(image, &grid);
    lv_obj_align(image, LV_ALIGN_CENTER, 0, 0);

    smf_set_initial(SMF_CTX(&state_object), &states[STATE_1]);
}

int state_machine_run() {
    lv_timer_handler();
    return smf_run_state(SMF_CTX(&state_object));
}

static void STATE_1_entry(void* o) { // start up
    printk("STATE_1\n");
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
    
    for (uint8_t i = 0; i < 9; i++) {
    lv_obj_t *ui_btn = lv_button_create(screen);
    // place the buttons in a 2x2 grid in the center of the screen
    // matching the orientations of the LEDs on the board
    if (i < 3) {
      lv_obj_align(ui_btn, LV_ALIGN_TOP_LEFT, 70 * (i + 1), 50);
    }
    else if (i < 6) {
      lv_obj_align(ui_btn, LV_ALIGN_TOP_LEFT, 70 * (i - 2), 105);
    }
    else {
      lv_obj_align(ui_btn, LV_ALIGN_TOP_LEFT, 70 * (i - 5), 160);
    }
    lv_obj_t *button_label = lv_label_create(ui_btn);
    char label_text[2];
    snprintf(label_text, 2, "-");
    lv_label_set_text(button_label, label_text);
    lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *data_obj = button_label;
    lv_obj_add_event_cb(ui_btn, lv_button_callback, LV_EVENT_CLICKED, data_obj);

    lv_obj_t *score1_label = lv_label_create(screen);
    lv_label_set_text_fmt(score1_label, "Score: %d - %d", state_object.score1, state_object.score2);
    lv_obj_align(score1_label, LV_ALIGN_TOP_LEFT, 10, 10);

    //lv_obj_t *score2_label = lv_label_create(screen);
    //lv_label_set_text_fmt(score2_label, "Score: %d", state_object.score2);
    //lv_obj_align(score2_label, LV_ALIGN_TOP_RIGHT, -10, 10);
  }

  display_blanking_off(display_dev);
}

static enum smf_state_result STATE_1_run(void* o) {
    
    if (BTN_check_clear_pressed(BTN0)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN3)) { 
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    return SMF_EVENT_HANDLED;
}

static void STATE_2_entry(void* o) { //player 1 turn
    printk("STATE_2\n");
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
    state_object.player = 1; 
}

static enum smf_state_result STATE_2_run(void* o) {

    if (state_object.player == 2) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_3]); //switch to O
    }
    
    if (BTN_check_clear_pressed(BTN0)) {
        state_object.score1 += 1;
        //lv_label_set_text_fmt(score1_label, "Score: %d", state_object.score1);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        state_object.score2 += 1;
        //lv_label_set_text_fmt(score2_label, "Score: %d", state_object.score2);
    }
    if (BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_4]); //clear
    }
    if (BTN_check_clear_pressed(BTN3)) { 
        smf_set_state(SMF_CTX(&state_object), &states[STATE_4]); //clear
    }

    return SMF_EVENT_HANDLED;
}

static void STATE_3_entry(void* o) { //player 2 turn
    printk("STATE_3\n");
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_ON);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
    state_object.player = 2;
}

static enum smf_state_result STATE_3_run(void* o) {

    if (state_object.player == 1) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]); //switch to X
    }
    
    if (BTN_check_clear_pressed(BTN0)) {
        state_object.score1 += 1;
        //lv_label_set_text_fmt(score1_label, "Score: %d", state_object.score1);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        state_object.score2 += 1;
        //lv_label_set_text_fmt(score2_label, "Score: %d", state_object.score2);
    }
    if (BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_4]); //clear
    }
    if (BTN_check_clear_pressed(BTN3)) { 
        smf_set_state(SMF_CTX(&state_object), &states[STATE_4]); //clear
    }

    return SMF_EVENT_HANDLED;
}

static void STATE_4_entry(void* o) { //clear
    printk("STATE_4\n");
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_ON);
    LED_set(LED2, LED_ON);
    LED_set(LED3, LED_ON);
    state_object.player = 3;
}

static enum smf_state_result STATE_4_run(void* o) {

    if (BTN_check_clear_pressed(BTN0)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }
    if (BTN_check_clear_pressed(BTN3)) { 
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }

    return SMF_EVENT_HANDLED;
}