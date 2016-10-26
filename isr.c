// this code needs to be copied to isr.c if you choose or it can be pasted to main or i2c function

#include "isr.h"

//
volatile uint8_t messageComplete = 0, CALL_OR_WRITE = 0;

// STEP 1 -> send slave i2c adress shifted 1 bit left then or w/ 0 (write mode)
// STEP 2 -> send slave i2c reg adress adress (depends on module)
// STEP 3 -> Send slave i2c data (depends on module)
// Call or write is for enabling secuentian writing 
// call first enables secuential writing 

void __attribute__((__interrupt__, no_auto_psv)) _MI2C1Interrupt(void){
    
       
        static int8_t cmd = 0;
        
        Nop();

        switch(cmd)
        {
            case MASTERWRITESLAVEADDR://0
                //Write slave address
                I2C1TRN = (SLAVEADDRESS << 1) | 0;// this can be defined if fixed  or declare it as you need
                 break;
            
            if (CALL_OR_WRITE == CALL){
                cmd = 4;
            }
            if (CALL_OR_WRITE == WRITE){
                cmd = 2;
            }
            case MASTERWRITESREGADDR://1
                if(I2C1STATbits.ACKSTAT == 0)
                {
                     I2C1TRN =  I2CRegAddr;
                }

            break;
            case MASTERWRITESREGDATA://2
                if(I2C1STATbits.ACKSTAT == 0)
                {
                I2C1TRN = I2CLoadWrite;
                }
            break;
            case STOPCONDITION: //3
                 if(I2C1STATbits.ACKSTAT == 0)
                {
                    I2C1CON1bits.PEN = 1;
                 }
            break;
            case RESET:
                messageComplete = 1;
                cmd = -1;
                break;
        }
    cmd++;
    IFS1bits.MI2C1IF = 0;
}



 
