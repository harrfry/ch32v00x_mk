#include "ch32fun.h"
#include <stdio.h>
#include <string.h>
#include "rv003usb.h"
#include <stdbool.h>
#include "keymap.h"

static uint8_t mouse_report[4] = { 0x00, 0x00, 0x00, 0x00 };
static uint8_t keyboard_report[8] = {0x00};

static const uint32_t row_pins[] = MATRIX_ROW_PINS;
static const uint32_t col_pins[] = MATRIX_COL_PINS;

static bool matrix_raw[MATRIX_ROWS][MATRIX_COLS] = {0};
static bool matrix_debounced[MATRIX_ROWS][MATRIX_COLS] = {0};
static bool matrix_previous[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint32_t debounce_timers[MATRIX_ROWS][MATRIX_COLS] = {0};

static uint16_t active_layers = 0;
static uint16_t active_keys[MATRIX_ROWS][MATRIX_COLS];
static event_queue_t queue = {.head = 0, .tail = 0};


void init_gpio(void) {
	
	funGpioInitAll();
	#ifdef CAPS_LOCK_LED
	funPinMode(CAPS_LOCK_LED, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP); // caps lock led
	#endif

	#ifdef COL2ROW
	for(uint8_t r = 0; r < MATRIX_ROWS; r++) {
		funPinMode(row_pins[r], GPIO_Speed_In | GPIO_CNF_IN_PUPD);
		funDigitalWrite(row_pins[r], 0);
	
	}

	for(uint8_t c = 0; c < MATRIX_COLS; c++) {
		funPinMode(col_pins[c], GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
		funDigitalWrite(col_pins[c], 0);
	}
	#endif
	#ifdef ROW2COL
	for(uint8_t r = 0; r < MATRIX_ROWS; r++) {
		funPinMode(row_pins[r], GPIO_Speed_10Mhz | GPIO_CNF_OUT_PP);
		funDigitalWrite(row_pins[r], 0);
	
	}

	for(uint8_t c = 0; c < MATRIX_COLS; c++) {
		funPinMode(col_pins[c], GPIO_Speed_In | GPIO_CNF_IN_PUPD);
		funDigitalWrite(col_pins[c], 0);
	}
	#endif

}

#define CHARGE_DELAY48() __asm__ volatile(".rept 48 \n\t nop \n\t .endr")

void matrix_scan() {
	#ifdef COL2ROW
	for(uint8_t c = 0; c < MATRIX_COLS; c++) {

		funDigitalWrite(col_pins[c], 1);

		CHARGE_DELAY48();

		for(uint8_t r = 0; r < MATRIX_ROWS; r++) {
			matrix_raw[r][c] = (funDigitalRead(row_pins[r]) == 1);
			__asm__ volatile(".rept 9 \n\t nop \n\t .endr");
		}

		CHARGE_DELAY48();
		funDigitalWrite(col_pins[c], 0);
	}
	#endif
	#ifdef ROW2COL
	for(uint8_t r = 0; r < MATRIX_ROWS; r++) {

		funDigitalWrite(row_pins[r], 1);

		CHARGE_DELAY48();

		for(uint8_t c = 0; c < MATRIX_COLS; r++) {
			matrix_raw[r][c] = (funDigitalRead(col_pins[c]) == 1);
			__asm__ volatile(".rept 9 \n\t nop \n\t .endr");
		}

		CHARGE_DELAY48();
		funDigitalWrite(row_pins[r], 0);
	}
	#endif
}

bool matrix_debounce() {
	uint32_t current_time = funSysTick32();
	bool matrix_did_change = false;
	for(uint8_t r = 0; r < MATRIX_ROWS; r++) {

		for(uint8_t c = 0; c < MATRIX_COLS; c++) {
			uint32_t last_debounce_time = debounce_timers[r][c];
			if(matrix_raw[r][c] != matrix_debounced[r][c]) {
				
				if(last_debounce_time == 0) {
					debounce_timers[r][c] = current_time;
				}

				else if(TimeElapsed32u(current_time, last_debounce_time) >= Ticks_from_Ms(DEBOUNCE)) {
					matrix_debounced[r][c] = matrix_raw[r][c];
					debounce_timers[r][c] = 0;
					matrix_did_change = true;
				}

			} else {
				debounce_timers[r][c] = 0;
			}
		}

	}
	return matrix_did_change;
}

void layer_on(uint16_t index) {
	active_layers |= (1U << index);
}

void layer_off(uint16_t index) {
	active_layers &= ~(1U << index);
}

uint16_t get_highest_active_layer(void) {
	if(active_layers == 0) return 0;
	return 31 - __builtin_clz((uint32_t)active_layers);
}

bool queue_is_empty(void) {
	return queue.head == queue.tail;
}

bool queue_is_full(void) {
	return ((queue.tail + 1) & QUEUE_MASK) == queue.head;
}

void queue_push(uint16_t keycode, uint8_t row, uint8_t col, event_type_t type, uint32_t timestamp) {
	if(queue_is_full()) return;

	queue.events[queue.tail].keycode = keycode;
	queue.events[queue.tail].matrix_row = row;
	queue.events[queue.tail].matrix_col = col;
	queue.events[queue.tail].type = type;
	queue.events[queue.tail].timestamp = timestamp;

	queue.tail = (queue.tail + 1) & QUEUE_MASK;
}

bool queue_has_next(void) {
	if(queue_is_empty()) return false;

	uint8_t next_idx = (queue.head + 1) & QUEUE_MASK;

	return next_idx != queue.tail;
}

key_event_t* queue_peek_oldest(void) {
	if(queue_is_empty()) {
		return NULL;
	}
	return &queue.events[queue.head];
}

key_event_t* queue_peek_next(void) {
	if(queue_is_empty()) {
		return NULL;
	}
	uint8_t next_idx = (queue.head + 1) & QUEUE_MASK;
	return &queue.events[next_idx];
}

void queue_pop_oldest(void) {
	if(!queue_is_empty()) {
		queue.head = (queue.head + 1) & QUEUE_MASK;
	}
}


uint16_t get_effective_keycode(uint8_t r, uint8_t c) {
	uint16_t start_layer = get_highest_active_layer();
	for(uint16_t l = start_layer; l >= 0; l--) {
		if(active_layers & (1 << l)) {
			if(keymap[l][r][c] != 0) {
				return keymap[l][r][c];
			}
		}
	}
	return keymap[0][r][c];
}

void process_matrix_changed(void) {
	uint32_t current_time = funSysTick32();

	for(uint8_t r = 0; r < MATRIX_ROWS; r++) {
		for(uint8_t c = 0; c < MATRIX_COLS; c++) {
			bool current = matrix_debounced[r][c];
			bool previous = matrix_previous[r][c];

			if(current == previous) continue;

			if(current && !previous) { // this is a press
				uint16_t key = keymap[0][r][c];
				if(key == 0) continue;

				active_keys[r][c] = key;

				queue_push(key, r, c, EVENT_PRESS, current_time);
			} else if(!current && previous) { // this is a release event
				uint16_t key = active_keys[r][c];
				if(key == 0) continue;

				queue_push(key, r, c, EVENT_RELEASE, current_time);
				active_keys[r][c] = 0;
			}
			matrix_previous[r][c] = current;
		}
	}
}

void add_to_keyboard_report(uint8_t tap_keycode) {
	
	if(tap_keycode == 0) return;
	for(uint8_t i = 0; i < 6; i++) {
		if(keyboard_report[i + 2] == tap_keycode) return;
	}
	for(uint8_t i = 0; i < 6; i++) {
		if(keyboard_report[i + 2] == 0) {
			keyboard_report[i + 2] = tap_keycode;
			return;
		}
	}
}

void remove_from_hid_report(uint8_t tap_keycode) {
	if(tap_keycode == 0) return;
	for(uint8_t i = 0; i < 6; i++) {
		if(keyboard_report[i + 2] == tap_keycode) {
			keyboard_report[i + 2] = 0;
			break;
		}
	}
}

uint8_t get_mod_tap_mask(uint8_t mod_no) {
	uint8_t mod_mask;
	switch(mod_no) {
		case MOD_LGUI:
			mod_mask = LGUI;
			break;
		case MOD_LALT:
			mod_mask = LALT;
			break;
		case MOD_LCTL:
			mod_mask = LCTL;
			break;
		case MOD_LSFT:
			mod_mask = LSFT;
			break;
		case MOD_RGUI:
			mod_mask = RGUI;
			break;
		case MOD_RALT:
			mod_mask = RALT;
			break;
		case MOD_RCTL:
			mod_mask = RCTL;
			break;
		case MOD_RSFT:
			mod_mask = RSFT;
			break;
	}
	return mod_mask;
}

void process_keyboard_queue(void) {
	if(queue_is_empty()) return;
	
	key_event_t *oldest = queue_peek_oldest();
	uint32_t current_time = funSysTick32();

	uint8_t action_type = (uint8_t)(oldest->keycode >> 12); 
	uint8_t tap_keycode = (uint8_t)(oldest->keycode);
	uint8_t action_info = (uint8_t)((oldest->keycode & 0x0F00) >> 8); // target layer or the modifier number
	uint32_t timestamp = oldest->timestamp;
	uint8_t row = oldest->matrix_row;
	uint8_t col = oldest->matrix_col;


	if(action_type == ACTION_NORMAL) {
		if(oldest->type == EVENT_PRESS) add_to_keyboard_report(tap_keycode);
		else							remove_from_hid_report(tap_keycode);
		queue_pop_oldest();
		return;
	}

	if(action_type == ACTION_MODIFIER) {
		if(oldest->type == EVENT_PRESS) {
			keyboard_report[0] |= tap_keycode;
		} else {
			keyboard_report[0] &= ~(tap_keycode);
		}
		queue_pop_oldest();
		return;
	}

	if(action_type == ACTION_LAYER_MO) {
		if(oldest->type == EVENT_PRESS) {
			layer_on(action_info);
		} else {
			layer_off(action_info);
		}
		queue_pop_oldest();
		return;
	}

	if(action_type == ACTION_LAYER_TG) {
		if(oldest->type == EVENT_PRESS) {
			if(get_highest_active_layer() != action_info) {
				layer_on(action_info);
			} else {
				layer_off(action_info);
			}
		}
		queue_pop_oldest();
		return;
	}

	static uint32_t tap_hold_timers[MATRIX_ROWS][MATRIX_COLS] = {0};
	if(action_type == ACTION_MOD_TAP || ACTION_LAYER_TAP) {
		uint8_t mod_mask = 0;
		if(action_type == ACTION_MOD_TAP) mod_mask = get_mod_tap_mask(action_info);
		if(oldest->type == EVENT_RELEASE) {
			if(keyboard_report[0] & mod_mask) {
				keyboard_report[0] &= ~mod_mask;
				queue_pop_oldest();
				return;
			} else {
				if(active_layers & (1 << action_info)) {
					layer_off(action_info);
					queue_pop_oldest();
					return;
				}
			}
			if(TimeElapsed32u(current_time, tap_hold_timers[row][col]) >= Ticks_from_Ms(8)) { // bodge the living daylights out of it so that the press event is not immediately eaten before the packet sends
					remove_from_hid_report(tap_keycode);
					queue_pop_oldest();
					tap_hold_timers[row][col] = 0;
					return;
				}
				return;
		}
		if(oldest->type == EVENT_PRESS) {
			if(TimeElapsed32u(current_time, timestamp) >= Ticks_from_Ms(TAPPING_TERM)) { // if it's passed the tapping term then it's immediately realised as a hold (modifier)
				if(action_type == ACTION_MOD_TAP) 	keyboard_report[0] |= mod_mask;
				else								layer_on(action_info);
				queue_pop_oldest();
				return;
			}
			if(queue_has_next()) {
				uint8_t count = (queue.tail >= queue.head) ? (queue.tail - queue.head) : (QUEUE_SIZE - queue.head + queue.tail);
				for(uint8_t i = 1; i < count; i++) {
					uint8_t lookahead_idx = (queue.head + i) & QUEUE_MASK;
					key_event_t *future_event = &queue.events[lookahead_idx];

					uint8_t future_action_type = (uint8_t)(future_event->keycode >> 12); 
					// uint8_t future_tap_keycode = (uint8_t)(future_event->keycode);
					// uint8_t future_action_info = (uint8_t)((future_event->keycode & 0x0F00) >> 8); // target layer or the modifier number
					// uint32_t future_timestamp = future_event->timestamp;

					// this is a single isolated fast tap
					if((future_event->matrix_row == row) && (future_event->matrix_col == col)) {
						if(future_event->type == EVENT_RELEASE) {
							tap_hold_timers[row][col] = current_time;
							add_to_keyboard_report(tap_keycode);
							queue_pop_oldest();
							return;
						}
						continue;
					}

					if((future_event->matrix_row != row || future_event->matrix_col != col) && (future_event->type != ACTION_MOD_TAP)) {
						add_to_keyboard_report(tap_keycode);
						queue_pop_oldest();
						return;
					} 
					
					// then if the next pressed keys is also a mod tap, but not on the same one, then look ahead and see whether there is a release of both before the timer and then pop both
					if((future_action_type == ACTION_MOD_TAP) && ((future_event->matrix_row != row) || (future_event->matrix_col != col))) {

						for(uint8_t j = i; j < count; j++) {
							uint8_t second_lookahead_idx = (queue.head + j) & QUEUE_MASK;
							key_event_t *second_future_event = &queue.events[second_lookahead_idx];

							uint8_t second_future_action_type = (uint8_t)(second_future_event->keycode >> 12); 
							// uint8_t second_future_tap_keycode = (uint8_t)(second_future_event->keycode);
							// uint8_t second_future_action_info = (uint8_t)((second_future_event->keycode & 0x0F00) >> 8); // target layer or the modifier number
							// uint32_t second_future_timestamp = second_future_event->timestamp; 

							if((second_future_action_type == EVENT_RELEASE) && ((TimeElapsed32u(current_time, future_event->timestamp)) < Ticks_from_Ms(TAPPING_TERM))) {
								add_to_keyboard_report(tap_keycode);
								queue_pop_oldest();
								return;
							}
						}
					}
					

				}
			}
		}
	}
	return;
}

volatile bool burst_active = false;
volatile uint32_t usb_packet_timestamp = 0;

int main()
{
	SystemInit();
	Delay_Ms(1); // Ensures USB re-enumeration after bootloader or reset; Spec demand >2.5µs ( TDDIS )
	
	init_gpio();
	usb_setup();
	while(1)
	{
#if RV003USB_EVENT_DEBUGGING
		uint32_t * ue = GetUEvent();
		if( ue )
		{
			printf( "%lu %lx %lx %lx\n", ue[0], ue[1], ue[2], ue[3] );
		}
#endif
		uint32_t current_time = funSysTick32();		
		if(burst_active) {

		
			matrix_scan();
			bool matrix_changed = false;
			matrix_changed = matrix_debounce();
			if(matrix_changed) {
				process_matrix_changed();
			}
			process_keyboard_queue();
		}
		if(current_time >= (usb_packet_timestamp + (Ticks_from_Ms(1)))) { // fuck ass weird hack that gets it working for some reason
			burst_active = false;
		}
	}
}


void usb_handle_user_data( struct usb_endpoint * e, int current_endpoint, uint8_t * data, int len, struct rv003usb_internal * ist )
{
	#ifdef RV003USB_EVENT_DEBUGGING
	if (len > 0) {
		LogUEvent(1139, data[0], 0, current_endpoint);
	}
	#endif
	#ifdef CAPS_LOCK_LED
	if(current_endpoint == 0 && len > 0) {
		uint8_t kbd_leds = data[0];
		if(kbd_leds & 0x02) {
			funDigitalWrite(PC4, 1);
		} else {
			funDigitalWrite(PC4, 0);
		}
	}
	#endif
}

void usb_handle_user_in_request( struct usb_endpoint * e, uint8_t * scratchpad, int endp, uint32_t sendtok, struct rv003usb_internal * ist )
{
	if( endp == 1 )
	{
		__asm__ volatile ( // fuck ass delay needed apparently lmao
			"nop\n\t"
		);
		
		
		// Move the mouse right, down, left and up in a square.
		usb_send_data( mouse_report, 4, 0, sendtok );
		__asm__ volatile (
			"nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
			"nop\n\t" "nop\n\t"
		);

		
	}
	else if( endp == 2 )
	{
		// Keyboard (8 bytes)
		__asm__ volatile ("nop; nop; nop; nop;");

		usb_send_data(keyboard_report, 8, 0, sendtok);
		

		

		
		
		__asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
		
	}
	else
	{
		// If it's a control transfer, empty it.
		usb_send_empty( sendtok );
	}
	burst_active = true;
	usb_packet_timestamp = funSysTick32();
}


