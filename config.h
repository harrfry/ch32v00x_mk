#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBOUNCE 10 // ideally 10 since polling is 10 and reporting gets choppy otherwise
#define COL2ROW

#define TAPPING_TERM 185
#define COMBO_TERM

#define CAPS_LOCK_LED PC4

#define MATRIX_ROWS 5
#define MATRIX_COLS 5
#define MATRIX_ROW_PINS {PC5, PC6, PD6, PD2, PC2}
#define MATRIX_COL_PINS {PC7, PC0, PC1, PC3, PD7}

#endif
