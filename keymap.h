#ifndef __KEYMAP_H
#define __KEYMAP_H

#include "hid_keys.h"
#include "config.h"

static const uint16_t keymap[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
        {MT(MOD_LGUI, HID_KEY_A), HID_KEY_1},
        {HID_KEY_2, HID_KEY_3}
    }
};

#endif
