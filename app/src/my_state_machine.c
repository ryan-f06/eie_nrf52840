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

static lv_obj_t *start_screen = NULL;

extern const lv_image_dsc_t start_img;
extern const lv_image_dsc_t grid;


/**
 * Function Prototypes
 */
static void STATE_1_entry(void* o);
static enum smf_state_result STATE_1_run(void* o);
static void STATE_1_exit(void* o);
static void STATE_2_entry(void* o);
static enum smf_state_result STATE_2_run(void* o);
static void STATE_3_entry(void* o);
static enum smf_state_result STATE_3_run(void* o);
static void STATE_4_entry(void* o);
static enum smf_state_result STATE_4_run(void* o);
static void STATE_5_entry(void* o);
static enum smf_state_result STATE_5_run(void* o);

/**
 * Typedefs
 */
enum state_machine_states {
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4,
		STATE_5
};

typedef struct {
	char sym;
	lv_obj_t *btn;
} pos_data_t;

typedef struct {
    // Context variable used by zephyr to track state machine state. Must be first
    struct smf_ctx ctx;

    uint16_t count;
    char player; //X, O
    uint16_t score1;
    uint16_t score2;
		uint8_t win_flag;
    lv_obj_t *score_label;
		pos_data_t grid[3][3];
    
} state_object_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} pos_t;

/**
 * Local Variables
 */

static const struct smf_state states[] = {
    [STATE_1] = SMF_CREATE_STATE(STATE_1_entry, STATE_1_run, STATE_1_exit, NULL, NULL),
    [STATE_2] = SMF_CREATE_STATE(STATE_2_entry, STATE_2_run, NULL, NULL, NULL),
    [STATE_3] = SMF_CREATE_STATE(STATE_3_entry, STATE_3_run, NULL, NULL, NULL),
    [STATE_4] = SMF_CREATE_STATE(STATE_4_entry, STATE_4_run, NULL, NULL, NULL),
		[STATE_5] = SMF_CREATE_STATE(STATE_5_entry, STATE_5_run, NULL, NULL, NULL)    
};

static state_object_t state_object;

void lv_button_callback(lv_event_t *event) {

	lv_obj_t * btn = lv_event_get_target_obj(event);
	lv_obj_t * label = lv_obj_get_child(btn, 0);

    lv_obj_t *data_obj = lv_event_get_user_data(event);

	pos_t *pos = lv_data_obj_get_data_ptr(data_obj);

	
  char label_text[2];
  if (state_object.player == 'X' && state_object.grid[pos->x][pos->y].sym == '\0' && !state_object.win_flag){
    snprintf(label_text, 2, "X");
    lv_label_set_text(label, label_text);
    //state_object.player = 2; //switch to O
		state_object.grid[pos->x][pos->y].sym = 'X';
		smf_set_state(SMF_CTX(&state_object), &states[STATE_5]);
  }
  else if (state_object.player == 'O' && state_object.grid[pos->x][pos->y].sym == '\0' && !state_object.win_flag) {
    snprintf(label_text, 2, "O");
    lv_label_set_text(label, label_text);
    //state_object.player = 1; //switch to X
		state_object.grid[pos->x][pos->y].sym = 'O';
		smf_set_state(SMF_CTX(&state_object), &states[STATE_5]);
  }

}

int check(char player) {
	if (state_object.grid[0][0].sym == player &&
			state_object.grid[1][1].sym == player &&
			state_object.grid[2][2].sym == player) {
		return 1; // top left - bot right win
	}
		

	if (state_object.grid[2][0].sym == player &&
			state_object.grid[1][1].sym == player &&
			state_object.grid[0][2].sym == player) {
		return 1; // bot left - top right win
	}

	for (uint8_t i = 0; i < 3; i++){
		if (state_object.grid[i][0].sym == player &&
				state_object.grid[i][1].sym == player &&
				state_object.grid[i][2].sym == player) {
			return 1; // each row in a column win
		}

		if (state_object.grid[0][i].sym == player &&
				state_object.grid[1][i].sym == player &&
				state_object.grid[2][i].sym == player) {
			return 1; // each column in a row win
		}
	}
	return 0;
}

void state_machine_init() {
    state_object.count = 0;
    state_object.score1 = 0;
    state_object.score2 = 0;
	state_object.win_flag = 0;
    state_object.player = 0;

    device_is_ready(display_dev);

    start_screen = lv_screen_active();

    lv_obj_t *start_image = lv_image_create(start_screen);
    lv_image_set_src(start_image, &start_img);
    lv_obj_align(start_image, LV_ALIGN_CENTER, 0, 0);

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

    lv_obj_t * start_label = lv_label_create(start_screen);
	lv_label_set_text_fmt(start_label, "Tic Tac Toe");
	lv_obj_align(start_label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * start_label_2 = lv_label_create(start_screen);
	lv_label_set_text_fmt(start_label_2, "Press Any Button");
	lv_obj_align(start_label_2, LV_ALIGN_BOTTOM_MID, 0, -10);

    display_blanking_off(display_dev);
}

static enum smf_state_result STATE_1_run(void* o) {
    
    if (BTN_check_clear_pressed(BTN0) || BTN_check_clear_pressed(BTN1) || 
    BTN_check_clear_pressed(BTN2) || BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);
    }

    return SMF_EVENT_HANDLED;
}

static void STATE_1_exit(void* o) { // start up
    
    lv_obj_clean(start_screen);

    screen = lv_screen_active();

    lv_obj_t *image = lv_image_create(screen);
    lv_image_set_src(image, &grid);
    lv_obj_align(image, LV_ALIGN_CENTER, 0, 0);

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

			pos_t pos = {.x = i % 3, .y = i / 3};

			lv_obj_t * data_obj = lv_data_obj_create_alloc_assign(ui_btn, &pos, sizeof(pos));
			lv_obj_add_event_cb(ui_btn, lv_button_callback, LV_EVENT_CLICKED, data_obj);

			state_object.grid[pos.x][pos.y].btn = ui_btn;
			state_object.grid[pos.x][pos.y].sym = '\0';
		}

		state_object.score_label = lv_label_create(screen);
		lv_label_set_text_fmt(state_object.score_label, "Score: %d - %d", state_object.score1, state_object.score2);
		lv_obj_align(state_object.score_label, LV_ALIGN_TOP_LEFT, 10, 10);

  display_blanking_off(display_dev);
}

static void STATE_2_entry(void* o) { //player 1 turn
    printk("STATE_2\n");
    LED_set(LED0, LED_ON);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
    state_object.player = 'X'; 
}

static enum smf_state_result STATE_2_run(void* o) {
    
    if (BTN_check_clear_pressed(BTN0)) {
        state_object.score1 += 1;
        lv_label_set_text_fmt(state_object.score_label, "Score: %d - %d", state_object.score1, state_object.score2);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        state_object.score2 += 1;
        lv_label_set_text_fmt(state_object.score_label, "Score: %d - %d", state_object.score1, state_object.score2);
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
    state_object.player = 'O';
}

static enum smf_state_result STATE_3_run(void* o) {

    // if (state_object.player == 'X') {
    //     smf_set_state(SMF_CTX(&state_object), &states[STATE_2]); //switch to X
    // }
    
    if (BTN_check_clear_pressed(BTN0)) {
        state_object.score1 += 1;
        lv_label_set_text_fmt(state_object.score_label, "Score: %d - %d", state_object.score1, state_object.score2);
    }
    if (BTN_check_clear_pressed(BTN1)) {
        state_object.score2 += 1;
        lv_label_set_text_fmt(state_object.score_label, "Score: %d - %d", state_object.score1, state_object.score2);
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

		char label_text[2];
		snprintf(label_text, 2, "-");
		lv_obj_t * label;

    uint8_t i, j;
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				state_object.grid[i][j].sym = '\0';
				label = lv_obj_get_child(state_object.grid[i][j].btn, 0);
    		lv_label_set_text(label, label_text);
			}
		}
}

static enum smf_state_result STATE_4_run(void* o) {

    smf_set_state(SMF_CTX(&state_object), &states[STATE_2]);

    return SMF_EVENT_HANDLED;
}


static void STATE_5_entry(void* o) { //check
    printk("STATE_5\n");
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_ON);
    LED_set(LED3, LED_ON);

		if (check(state_object.player)) {
			if (state_object.player == 'X'){
				state_object.score1 += 1;
			}
			else {
				state_object.score2 += 1;
			}
			lv_label_set_text_fmt(state_object.score_label, "Score: %d - %d", state_object.score1, state_object.score2);
			state_object.win_flag = 1;
		}
		
}

static enum smf_state_result STATE_5_run(void* o) {

	if (state_object.win_flag) {
		if (BTN_check_clear_pressed(BTN2) || BTN_check_clear_pressed(BTN3)) {
            state_object.win_flag = 0;  
			smf_set_state(SMF_CTX(&state_object), &states[STATE_4]); //clear
    	}
    }
	
	else if (state_object.player == 'X') {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_3]); //switch to X
    }

	else if (state_object.player == 'O') {
        smf_set_state(SMF_CTX(&state_object), &states[STATE_2]); //switch to X
    }

    return SMF_EVENT_HANDLED;
}