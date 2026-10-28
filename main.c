#include "ch32fun.h"
#include <stdio.h>
#include <string.h>
#include "rv003usb.h"
#include <stdbool.h>
#include "keymap.h"

typedef struct {
	uint8_t packet[8];
} usb_packet_t;

typedef struct {
    usb_packet_t key_packet[QUEUE_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} usb_packet_queue_t;

static uint8_t mouse_report[4] = { 0x00, 0x00, 0x00, 0x00 };
static uint8_t keyboard_report[8] = {0x00};

const uint32_t row_pins[] = MATRIX_ROW_PINS;
const uint32_t col_pins[] = MATRIX_COL_PINS;

static bool matrix_raw[MATRIX_ROWS][MATRIX_COLS] = {0};
static bool matrix_debounced[MATRIX_ROWS][MATRIX_COLS] = {0};
static bool matrix_previous[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint32_t debounce_timers[MATRIX_ROWS][MATRIX_COLS] = {0};

static uint16_t active_layers = 0;
uint16_t active_keys[MATRIX_ROWS][MATRIX_COLS];
static event_queue_t queue = {.head = 0, .tail = 0};


void init_gpio(void) {
	
	funGpioInitAll();
	// funPinMode(PC4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP); // caps lock led


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
			if(matrix_raw[r][c] != matrix_debounced[r][c]) {
				if(TimeElapsed32u(current_time, debounce_timers[r][c]) >= Ticks_from_Ms(DEBOUNCE)) {
					matrix_debounced[r][c] = matrix_raw[r][c];
					debounce_timers[r][c] = current_time;
					matrix_did_change = true;
				}

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

void remove_from_hid_report(uint8_t tap_keybode) {
	if(tap_keycode == 0) return;
	for()
}

void process_keyboard_queue(void) {
	if(queue_is_empty()) return;
	
	key_event_t *oldest = queue_peek_oldest();
	uint32_t current_time = funSysTick32();

	uint8_t action_type = (uint8_t)(oldest->keycode >> 12); 
	uint8_t tap_keycode = (uint8_t)(oldest->keycode);
	uint8_t action_info = (uint8_t)((oldest->keycode & 0x0F00) >> 8); // target layer or the modifier number

	if(action_type = ACTION_NORMAL) {
		if(oldest->type == EVENT_PRESS) add_to_keyboard_report(tap_keycode);
		else							remove_from_hid_report(tap_keycode);
		queue_pop_oldest();
		return;
	}

}

int main()
{
	SystemInit();
	Delay_Ms(1); // Ensures USB re-enumeration after bootloader or reset; Spec demand >2.5µs ( TDDIS )
	
	init_gpio();
	usb_setup();
	uint32_t previous = 0;
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
		bool matrix_changed = false;
		if((current_time - previous) >= Ticks_from_Ms(1)) {
			matrix_scan();
			matrix_changed = matrix_debounce();
			if(matrix_changed) {
				process_matrix_changed();
			}
			previous = current_time;
		}
		// if((current_time - previous) >= Ticks_from_Ms(1000)) {
		// 	for(uint8_t r = 0; r < MATRIX_ROWS; r++) {
		// 		printf("Row %d: ", r);
		// 		for(uint8_t c = 0; c < MATRIX_COLS; c++) {
		// 			printf("%d ", matrix_debounced[r][c]);
		// 		}
		// 		printf("\n\r");
		// 	}
		// 	previous = current_time;
		// }
	}
}

void usb_handle_user_data( struct usb_endpoint * e, int current_endpoint, uint8_t * data, int len, struct rv003usb_internal * ist )
{
	#ifdef RV003USB_EVENT_DEBUGGING
	if (len > 0) {
		LogUEvent(1139, data[0], 0, current_endpoint);
	}
	#endif

	// if(current_endpoint == 0 && len > 0) {
	// 	uint8_t kbd_leds = data[0];
	// 	if(kbd_leds & 0x02) {
	// 		funDigitalWrite(PC4, 1);
	// 	} else {
	// 		funDigitalWrite(PC4, 0);
	// 	}
	// }
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

		usb_send_data( oldest->packet, 8, 0, sendtok );
		
		__asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
		
	}
	else
	{
		// If it's a control transfer, empty it.
		usb_send_empty( sendtok );
	}
}


