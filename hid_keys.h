#ifndef __HID_KEYS_H
#define __HID_KEYS_H

#include <stdint.h>

#define QUEUE_SIZE 8
#define QUEUE_MASK (QUEUE_SIZE - 1)

// 0b00000000

#define LAYER_TAP_MASK      (1U << 12)
#define MOD_TAP_MASK        (2U << 12)
#define MODIFIER_MASK       (3U << 12)
#define MOUSE_KEY_MASK      (4U << 12)
#define LAYER_MO            (5U << 12)
#define LAYER_TG            (6U << 12)

#define SAFE_RANGE          (15U << 12)

#define ACTION_LAYER_TAP    1U
#define ACTION_MOD_TAP      2U
#define ACTION_MODIFIER     3U
#define ACTION_MOUSE_KEY    4U
#define ACTION_LAYER_MO     5U
#define ACTION_LAYER_TG     6U
#define ACTION_USER         15U
#define ACTION_NORMAL       0



typedef enum {
    MOD_LGUI = 1,
    MOD_LALT,
    MOD_LCTL,
    MOD_LSFT,
    MOD_RGUI,
    MOD_RALT,
    MOD_RCTL,
    MOD_RSFT
} mod_tap_codes;

#define LGUI            KEYBOARD_MODIFIER_LEFTGUI
#define LALT            KEYBOARD_MODIFIER_LEFTALT
#define LCTL            KEYBOARD_MODIFIER_LEFTCTRL
#define LSFT            KEYBOARD_MODIFIER_LEFTSHIFT
#define RGUI            KEYBOARD_MODIFIER_RIGHTGUI
#define RALT            KEYBOARD_MODIFIER_RIGHTALT
#define RCTL            KEYBOARD_MODIFIER_RIGHTCTRL
#define RSFT            KEYBOARD_MODIFIER_RIGHTSHIFT

#define MT(mod, key)        (uint16_t)(MOD_TAP_MASK | (mod << 8) | key)
#define LT(layer, key)      (uint16_t)(LAYER_TAP_MASK | (layer << 8) | key)
#define KC(key)             (uint16_t)(key)
#define MOD(modifier)       (uint16_t)(MODIFIER_MASK | modifier)

typedef enum {
    EVENT_PRESS = 0,
    EVENT_RELEASE
} event_type_t;

typedef struct {
    uint16_t keycode;
    uint8_t matrix_row;
    uint8_t matrix_col;
    event_type_t type;
    uint32_t timestamp;
} key_event_t;

typedef struct {
    uint8_t buttons_mask;
    int8_t x;
    int8_t y;
    int8_t vert_scroll;
    int8_t hori_scroll;
} mouse_report_t;

typedef struct {
    key_event_t events[QUEUE_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} event_queue_t;



#endif
