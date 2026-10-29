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
#define LAYER_MO_MASK       (5U << 12)
#define LAYER_TG_MASK       (6U << 12)
#define TAP_MOD_MASK        (7U << 12)

#define SAFE_RANGE          (15U << 12)

#define ACTION_LAYER_TAP    1U
#define ACTION_MOD_TAP      2U
#define ACTION_MODIFIER     3U
#define ACTION_MOUSE_KEY    4U
#define ACTION_LAYER_MO     5U
#define ACTION_LAYER_TG     6U
#define ACTION_TAP_MOD      7U
#define ACTION_USER         15U
#define ACTION_NORMAL       0

typedef struct{
    const uint16_t *keys;
    const uint16_t keycode;
} combo_t;

#define COMBO(combo_keys, combo_action) {.keys = &(combo_keys)[0], .keycode = combo_action}

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

typedef enum {
    LEFT = 1,
    RIGHT,
    UP,
    DOWN,
    BTN1,
    BTN2,
    BTN3
} mouse_action_t;

#define LGUI            KEYBOARD_MODIFIER_LEFTGUI
#define LALT            KEYBOARD_MODIFIER_LEFTALT
#define LCTL            KEYBOARD_MODIFIER_LEFTCTRL
#define LSFT            KEYBOARD_MODIFIER_LEFTSHIFT
#define RGUI            KEYBOARD_MODIFIER_RIGHTGUI
#define RALT            KEYBOARD_MODIFIER_RIGHTALT
#define RCTL            KEYBOARD_MODIFIER_RIGHTCTRL
#define RSFT            KEYBOARD_MODIFIER_RIGHTSHIFT

#define MT(mod, key)            (uint16_t)(MOD_TAP_MASK | (mod << 8) | key)
#define LT(layer, key)          (uint16_t)(LAYER_TAP_MASK | (layer << 8) | key)
#define MOD(modifier)           (uint16_t)(MODIFIER_MASK | modifier)
#define TAP_MOD(mod_no, key)    (uint16_t)(ACTION_TAP_MOD | (mod_no << 8) | key)

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

#define KC_A            HID_KEY_A
#define KC_B            HID_KEY_B
#define KC_C            HID_KEY_C
#define KC_D            HID_KEY_D
#define KC_E            HID_KEY_E
#define KC_F            HID_KEY_F
#define KC_G            HID_KEY_G
#define KC_H            HID_KEY_H
#define KC_I            HID_KEY_I
#define KC_J            HID_KEY_J
#define KC_L            HID_KEY_L
#define KC_M            HID_KEY_M
#define KC_N            HID_KEY_N
#define KC_O            HID_KEY_O
#define KC_P            HID_KEY_P
#define KC_Q            HID_KEY_Q
#define KC_R            HID_KEY_R
#define KC_S            HID_KEY_S
#define KC_T            HID_KEY_T
#define KC_U            HID_KEY_U
#define KC_V            HID_KEY_V
#define KC_W            HID_KEY_W
#define KC_X            HID_KEY_X
#define KC_Y            HID_KEY_Y
#define KC_Z            HID_KEY_Z

#define KC_0            HID_KEY_0
#define KC_1            HID_KEY_1
#define KC_2            HID_KEY_2
#define KC_3            HID_KEY_3
#define KC_4            HID_KEY_4
#define KC_5            HID_KEY_5
#define KC_6            HID_KEY_6
#define KC_7            HID_KEY_7
#define KC_8            HID_KEY_8
#define KC_9            HID_KEY_9

#define KC_LGUI         MOD(LGUI)
#define KC_LALT         MOD(LALT)
#define KC_LCTL         MOD(LCTL)
#define KC_LSFT         MOD(LSFT)
#define KC_RGUI         MOD(RGUI)
#define KC_RALT         MOD(RALT)
#define KC_RCTL         MOD(RCTL)
#define KC_RSFT         MOD(RSFT)

#define KC_CAPS         HID_KEY_CAPS_LOCK
#define KC_SPC          HID_KEY_SPACE
#define KC_ESC          HID_KEY_ESCAPE
#define KC_TAB          HID_KEY_TAB
#define KC_NONE         (uint16_t)0



#endif
