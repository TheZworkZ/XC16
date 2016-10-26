// As 
#include "my_i2c.h"
// this is used to write to I2C LCD adapter
//#include "lcd_driver/LiquidCrystal.h"

// declare (all function prototypes && variables) as extern in my_i2c.h need them for ISR write
// functions & variables

//initialize variables used in I2C configuration
uint8_t I2CLoadWrite = 0, I2CRegAddr = 0, SET_L = 0;

void InitI2C(void){
    
    I2C1CON1bits.I2CEN = 1;      // Enable I2C Module
  
    //Configure I2C as MASTER to communicate with Load IC
    I2C1CON1bits.A10M   = 0;     // 7-bit slave address/data
    I2C1CON1bits.DISSLW = 1;     // Slew rate control disabled

    I2C1BRG = 690;       // 100kHz -->// (((1/100Khz) - 110n) * 70M) - 2 

    IFS1bits.MI2C1IF = 0;
    IEC1bits.MI2C1IE = 1;        // Enable Master I2C interrupt
    IPC4bits.MI2C1IP = 3;  
}  // finish -- tested
unsigned char I2C_W_Byte(char DATA){
           
    I2CLoadWrite = DATA;
    CALL_OR_WRITE = WRITE;
    I2C1CON1bits.SEN = 1; 
    while(messageComplete == 0);
    messageComplete = 0;
    return 0;
}// WRITE BYTE CAN SECUENTIAL
unsigned char I2C_Call(unsigned char ADRESS){
    
    I2CRegAddr = ADRESS;
    CALL_OR_WRITE = CALL;
    I2C1CON1bits.SEN = 1; 
    
    while(messageComplete == 0);
    messageComplete = 0;
    return 0;
} // Prepare for sequential writes
void IDLE_I2C(void){
    I2C1CON1bits.PEN = 1;
} // RESETS I2C to IDLE
