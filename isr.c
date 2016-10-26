#include "isr.h"

//Define variables  
uint16_t buckVoltage  = 0,boostVoltage = 0, inputVoltage = 0;
uint16_t buckCurrent  = 0,boostCurrent = 0;
uint16_t tempSense    = 0;
int16_t buckControlReference = 0, boostControlReference = 0;
volatile uint8_t messageComplete = 0, CALL_OR_WRITE = 0;


void __attribute__((__interrupt__, no_auto_psv)) _MI2C1Interrupt(void){
    
       
        static int8_t cmd = 0;
        
        Nop();

        switch(cmd)
        {
            case MASTERWRITESLAVEADDR://0
                //Write slave address
                I2C1TRN = (SLAVEADDRESS << 1) | 0;
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

void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void){

    IFS0bits.U1RXIF = 0; //Reset RX Interrupt flag
}
void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt(void){
    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
}



void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void){  
  
}
void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void){
    IFS0bits.T2IF = 0;
    /*
    if(_t2counter < _timeout && _t2counter < TIMEOUT_MAX_VALUE){
        _t2counter++;
    }
    else{
        _t2counter = 0;
        _timeoutFlag = true;            
        T2CONbits.TON = 0;
    }
     */ 
}
void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void){
  IFS0bits.T3IF = 0; //clear T1 interrupt flagRead

}

////////////////////////////////-CODE JUNKYARD-/////////////////////////////////
/**/
//T1
/*
    #if(BUCKDYNAMICLOAD == ENABLED)

    if(dpskFlags.buckSSActive == DISABLED)
    {
        I2CLoadWrite &= 0x38;           // Clear lower 3-bits for Buck write

        //Code to switch on-board load for dynamic load testing
        if(dpskFlags.buckDynLoad == 0)
        {
            I2CLoadWrite |= I2CBUCKLOAD2WRITE;

            LCD_BuckLoad = BUCKLOAD2LCD;
            dpskFlags.buckDynLoad = 1;
        }    
        else
        {
            I2CLoadWrite |= I2CBUCKLOAD1WRITE;

            LCD_BuckLoad = BUCKLOAD1LCD;
            dpskFlags.buckDynLoad = 0;
        }
    }
    #else
    I2CLoadWrite |= I2CBUCKLOAD1WRITE;    //Restores load setting after fault

    #endif

    #if(BOOSTDYNAMICLOAD == ENABLED)

    if(dpskFlags.boostSSActive == DISABLED)
    {
        I2CLoadWrite &= 0x07;            // Clear bits 4-6 for Boost write

        //Code to switch on-board load for dynamic load testing
        if(dpskFlags.boostDynLoad == 0)
        {
            I2CLoadWrite |= I2CBOOSTLOAD2WRITE;

            LCD_BoostLoad = BOOSTLOAD2LCD;
            dpskFlags.boostDynLoad = 1;
        }
        else
        {
            I2CLoadWrite |= I2CBOOSTLOAD1WRITE;

            LCD_BoostLoad = BOOSTLOAD1LCD;
            dpskFlags.boostDynLoad = 0;
        }
    }
    #else
    I2CLoadWrite |= I2CBOOSTLOAD1WRITE;    //Restores load setting after fault

    #endif
    
    I2C1CON1bits.SEN = 1;      // Initiate start sequence

    IFS0bits.T1IF = 0;
    */
//T3
/* 
  if (processFlags.stv1Active == 1){
       T_stv_1++;
       if (T_stv_1 == S_T_stv_1){
         processFlags.stv1Active = 0;
         // call beeper
       }
  }
  if (processFlags.stv2Active == 1){
       T_stv_2++;
       if (T_stv_2 == S_T_stv_2){
         processFlags.stv2Active = 0;
         // call beeper
       }
  }
  if (processFlags.stv3Active == 1){
       T_stv_3++;
       if (T_stv_3 == S_T_stv_3){
         processFlags.stv3Active = 0;
         // call beeper
       }
  }
  if (processFlags.stv4Active == 1){
       T_stv_4++;
       if (T_stv_4 == S_T_stv_4){
         processFlags.stv4Active = 0;
         // call beeper
       }
  }
  */ 
//U1TX
/*
    while(DataRdyUART1())    {
        RXData[RXCnt] = ReadUART1();
        RXCnt++;
        if((RXCnt > 63) || (RXData[RXCnt-1] == 0x0D)){
           
            for(i=0; i < RXCnt; i++)
            {
                while(BusyUART1()); // Wait till TX Buffer has space
                putcUART1(RXData[i]);
            }
            
        memset(RXData,0x00,63);
        RXCnt = 0;
        }
    }
    RXCnt   =0;
    */
//Add this code later
/*
#if(__XC16_VERSION__ == 1024)           // for XC16 v1.24 compiler
void __attribute__((__interrupt__, no_auto_psv)) _ADCMP1Interrupt(void)
#else                                   
void __attribute__((__interrupt__, no_auto_psv)) _ADCMP0Interrupt(void)
#endif
{
    if(ADCMP0HI == WARNINGTEMPADC)
    {
        // Change digital cmp settings
        ADCMP0CONbits.LOLO = 1;
        ADCMP0LO = TEMPREINITADC;
        ADCMP0HI = TEMPMAXADC;

        if(dpskFaultFlags.overTemp == 0)
        {
            // set bit, indicates hi temp warning fault
            dpskFaultFlags.tempWarning = 1;
        }
    }
    else if (ADCBUF7 >= TEMPMAXADC)
    {
       // Change digital cmp settings
        ADCMP0CONbits.HIHI         = 0;

        dpskFaultFlags.overTemp    = 1;
        dpskFaultFlags.tempWarning = 0;

        I2CLoadWrite     = 0x09;   // If over-temp condition, change loads to 0.5W (minimum load)

        I2C1CON1bits.SEN = 1;      // Initiate start sequence

        #if((BUCKDYNAMICLOAD == ENABLED) || (BOOSTDYNAMICLOAD == ENABLED))
        IEC0bits.T1IE = 0;
        #endif
    }
    else if (ADCBUF7 <= TEMPREINITADC)
    {
        // Change digital cmp settings
        ADCMP0CONbits.LOLO = 0;
        ADCMP0CONbits.HIHI = 1;
        ADCMP0HI = WARNINGTEMPADC;

        if(dpskFaultFlags.tempWarning == 1)
        {
            // Clear bit to disable 'HI TEMP WARNING' on display
            dpskFaultFlags.tempWarning = 0;
        }
        else if (dpskFaultFlags.overTemp == 1)
        {
            // Clear bit to disable 'OVER TEMP FAULT' on display
            dpskFaultFlags.overTemp = 0;

           // Re-enable loads
           I2CLoadWrite = (I2CBUCKLOAD1WRITE | I2CBOOSTLOAD1WRITE);
           I2C1CON1bits.SEN = 1;            // Initiate start sequence

           #if((BUCKDYNAMICLOAD == ENABLED) || (BOOSTDYNAMICLOAD == ENABLED))
           IEC0bits.T1IE = 1;
           #endif
        }
    }

    IFS11bits.ADCMP0IF = 0;
}


#if(__XC16_VERSION__ == 1024)                 // for XC16 v1.24 compiler
void __attribute__((__interrupt__, no_auto_psv)) _ADCMP2Interrupt(void)
#else
void __attribute__((__interrupt__, no_auto_psv)) _ADCMP1Interrupt(void)
#endif
{
    if(ADCMP1HI == INPUTOVERVOLTAGEADC)
    {
        // Change digital comparator settings
        ADCMP1CONbits.HIHI = 0;
        ADCMP1CONbits.LOLO = 0;

        ADCMP1HI = INPUTVOLTMAXHYST;
        ADCMP1LO = INPUTVOLTMINHYST;

        if(inputVoltage >= INPUTOVERVOLTAGEADC)
        {
            ADCMP1CONbits.HILO = 1;      // Interrupt when ADCBUF is less than DCMP HI
        }
        else
        {
          ADCMP1CONbits.LOHI = 1;        // Interrupt when ADCBUF is >= than DCMP LO
        }

        IOCON1bits.OVRENH = 1;           // Override the PWM1H/L - Buck FETs to inactive state
        IOCON1bits.OVRENL = 1;
        IOCON2bits.OVRENL = 1;           // Override the PWMHL - Boost FET to inactive state

        IEC6bits.ADCAN1IE = 0;           // Disable interrupts
        IEC7bits.ADCAN3IE = 0;

        CMP1DAC = 0;
        CMP2DAC = 0;

        dpskFaultFlags.inputVoltage = 1;

        I2CLoadWrite = NOLOAD;     // If fault condition exists, disable ALL loads
        I2CLoadWrite = 0x09;
        I2C1CON1bits.SEN = 1;      // Initiate start sequence
    }
    else
    {
        // Change digital comparator settings
        ADCMP1CONbits.HILO = 0;
        ADCMP1CONbits.LOHI = 0;

        ADCMP1HI = INPUTOVERVOLTAGEADC;
        ADCMP1LO = INPUTUNDERVOLTAGEADC;

        ADCMP1CONbits.HIHI = 1;
        ADCMP1CONbits.LOLO = 1;

        dpskFaultFlags.inputVoltage = 0;

        #if(BUCK == ENABLED)

            #if(OPENLOOP == DISABLED)

                MACRO_CLR_BUCKHISTORY()
                buckControlReference = 0;

                IEC6bits.ADCAN1IE    = 1;

                IOCON1bits.OVRENH    = 0;       // Remove Override
                IOCON1bits.OVRENL    = 0;
            #endif

            dpskFlags.buckSSActive = ENABLED;   // Reinitiate softstart

        #endif

        #if(BOOST == ENABLED)

            #if(OPENLOOP == DISABLED)

                MACRO_CLR_BOOSTHISTORY()  // Clear error history

                // SW Trigger AN3 for initial Boost output voltage Measurement
                ADTRIG0Hbits.TRGSRC3 = 1;
                ADCON3Lbits.SWCTRG   = 1;
                while(ADSTATLbits.AN3RDY != 1);
                boostControlReference = ADCBUF3;
                ADTRIG0Hbits.TRGSRC3 = 6;       // Change trigger back to PWM2
                IEC7bits.ADCAN3IE    = 1;
                IOCON2bits.OVRENL    = 0;       // Remove Override

            #endif

            dpskFlags.boostSSActive = ENABLED;   // Reinitiate softstart

         #endif

       // Re-enable loads
       I2CLoadWrite = (I2CBUCKLOAD1WRITE | I2CBOOSTLOAD1WRITE);
       I2C1CON1bits.SEN = 1;            // Initiate start sequence

       // Fire up soft start timer
       TMR2            = 0;
       T2CONbits.TON   = 1;

    }


    IFS11bits.ADCMP1IF = 0;


}





void __attribute__((__interrupt__, no_auto_psv)) _ADCAN7Interrupt(void)
{
    refreshLCD++;

    // Read Vin and Temperature for LCD display, fault management performed
    // in digital comparator ISRs
 
    inputVoltage = ADCBUF4;
    tempSense    = ADCBUF7;

    IFS7bits.ADCAN7IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void)
{
    #if(BUCK == ENABLED)
    static uint8_t buckErrorCount = 0;

    #if(OPENLOOP == DISABLED)  // If in closed-loop, perform soft-start routine
    {
       if(dpskFlags.buckSSActive == ENABLED)
       {
          if(buckControlReference < BUCKVOLTAGEREFERENCEADC)
          {
             buckControlReference += BUCKINCREMENT;

             if((buckErrorControlHistory[0]) > ( BUCK_FB_LOOP_CHECK)) // Used to check feedback loop (is feedback loop open?)
             {                                                        // and ensure that output follows Vreference
                ++buckErrorCount;

                if(buckErrorCount > 2)
                {
                   // Disable Buck converter PWMs
                   IOCON1bits.OVRENH = 1;                   // Over ride the PWM1H to inactive state
                   IOCON1bits.OVRENL = 1;                   // Over ride the PWM1L to inactive state
                   I2CLoadWrite = I2CLoadWrite & ~0x07;     // Configure Buck loads to be off
                   I2C1CONLbits.SEN = 1;                    // Initiate start sequence

                   dpskFaultFlags.buckSoftStart = ENABLED;  // Display: Bck Output Fault
                   IFS6bits.ADCAN1IF = 0;
                 }
              }
           }
           else
           {
              buckErrorCount         = 0;
              buckControlReference   = BUCKVOLTAGEREFERENCEADC;
              dpskFlags.buckSSActive = DISABLED;    // Soft-start is complete
           }
        }
    }

    #else

    if(PDC1 < BUCKOPENLOOPDC)
    {
      PDC1 += BUCKINCREMENT;
    }
    else
    {
      PDC1 = BUCKOPENLOOPDC;
      dpskFlags.buckSSActive = DISABLED;
    }
    #endif

#endif

    #if(BOOST == ENABLED)
    static uint8_t boostErrorCount = 0;

    #if(OPENLOOP == DISABLED)  // If in closed-loop, perform soft-start routine
    {
      if(dpskFlags.boostSSActive == ENABLED)
      {

        if(boostControlReference < BOOSTVOLTAGEREFERENCEADC)
        {
           boostControlReference += BOOSTINCREMENT;

           if ((boostErrorControlHistory[0]) > (BOOST_FB_LOOP_CHECK))  // Used to check feedback loop (is feedback loop open?)
           {                                                          // and ensure that output follows Vreference
             ++boostErrorCount;

             if(boostErrorCount > 2)
             {
               //Disable Boost converter PWMs
               IOCON2bits.OVRENL = 1;                       // Override PWM2H to inactive state

               I2CLoadWrite = I2CLoadWrite & 0x07;          // Configure boost loads off
               I2C1CONLbits.SEN = 1;                        // Initiate start sequence

               dpskFaultFlags.boostSoftStart = ENABLED;     // Display: Bst Output Fault
               IFS7bits.ADCAN3IF = 0;
             }
           }
         }
         else
         {
            boostErrorCount         = 0;
            boostControlReference   = BOOSTVOLTAGEREFERENCEADC;
            dpskFlags.boostSSActive = DISABLED;    // Soft-start is complete
         }
      }
    }

    #else

    if(PDC2 < BOOSTOPENLOOPDC)
    {
      PDC2 += BOOSTINCREMENT;
    }
    else
    {
      PDC2 = BOOSTOPENLOOPDC;
      dpskFlags.boostSSActive = DISABLED;
    }
    #endif

#endif

   // Disable Timer once both softstart flags are disabled
   if((dpskFlags.buckSSActive == DISABLED) && (dpskFlags.boostSSActive == DISABLED))
   {
       T2CONbits.TON = 0;
   }

    IFS0bits.T2IF = 0;
}

*/



 