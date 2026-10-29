#ifndef __KEYMAP_H
#define __KEYMAP_H

#include "hid_keys.h"
#include "config.h"

#define HRM_A   MT(MOD_LGUI, KC_A)
#define HRM_R   MT(MOD_LALT, KC_R)
#define HRM_S   MT(MOD_LCTL, KC_S)
#define HRM_T   MT(MOD_LSFT, KC_T)


static const uint16_t keymap[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
        {KC_1, KC_2, KC_3, KC_4, KC_5},
        {KC_Q, KC_W, KC_F, KC_P, KC_B},
        {HRM_A, HRM_R, HRM_S, HRM_T, KC_G},
        {KC_Z, KC_X, KC_C, KC_D, KC_V},
        {KC_NONE, KC_NONE, KC_ESC, KC_SPC, KC_TAB},
    }
};

#endif
