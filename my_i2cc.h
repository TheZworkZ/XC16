// adjust this according to your needs - Tested for dspic33Ep64gs502
// write functions prototypes are declared as extern elseware (my lcd_i2c library)
// write functions prototypes are declared as extern elseware (my lcd_i2c library)
// write functions prototypes are declared as extern elseware (my lcd_i2c library)

#include "p33Exxxx.h"
#include "libpic30.h"


#ifndef MY_I2C_H
#define MY_I2C_H

void InitI2C(void);

//Declare variable used in init.c
volatile extern uint8_t messageComplete, CALL_OR_WRITE;
// write functions prototypes are declared as extern elseware (my lcd_i2c library)

#endif	/* MY_I2C_H */
