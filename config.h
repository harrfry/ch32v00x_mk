#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBOUNCE 10 // ideally 10 since polling is 10 and reporting gets choppy otherwise
#define COL2ROW

#define TAPPING_TERM 185

#define MATRIX_ROWS 2
#define MATRIX_COLS 2
#define MATRIX_ROW_PINS {PC5, PC6}
#define MATRIX_COL_PINS {PC7, PC0}

#endif
