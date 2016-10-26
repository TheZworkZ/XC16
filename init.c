
#include "init.h"
#include "lcd_driver/LiquidCrystal.h"
#include <string.h>
#include <stdlib.h>
//initialize variables used in I2C configuration
uint8_t I2CLoadWrite = 0, I2CRegAddr = 0, SET_L = 0;
char *TXData;
unsigned int RXData[64]; //To store Received data
unsigned int RXData2[16];
volatile unsigned int RXCnt = 0;


void InitClock(void){
   // Configure Oscillator to operate the device at 68 MHz
   // Fosc = Fin*M/(N1*N2), Fcy = Fosc/2
   // Fosc = 7.37*(76)/(2*2)= ~140Mhz for Fosc, Fcy = 70 MHz

   // Configure PLL prescaler, PLL postscaler, PLL divisor
   PLLFBD             = 74;   // M = PLLFBD + 2
   CLKDIVbits.PLLPOST =  0;   // N1 = 2
   CLKDIVbits.PLLPRE  =  0;   // N2 = 2

   __builtin_write_OSCCONH(0x01);     // New Oscillator selection FRC w/ PLL
   __builtin_write_OSCCONL(0x01);     // Enable Switch

   while(OSCCONbits.COSC != 0b001);    // Wait for Osc. to switch to FRC w/ PLL
   while(OSCCONbits.LOCK != 1);        // Wait for PLL to Lock

   // Setup the ADC and PWM clock for 120MHz
   // ((FRC * 16) / APSTSCLR ) = (7.37MHz * 16) / 1 = 117.9MHz

   ACLKCONbits.FRCSEL   = 1;	  // FRC provides input for Auxiliary PLL (x16)
   ACLKCONbits.SELACLK  = 1;	  // Aux Osc. provides clock source for PWM & ADC
   ACLKCONbits.APSTSCLR = 7;	  // Divide Auxiliary clock by 1
   ACLKCONbits.ENAPLL   = 1;	  // Enable Auxiliary PLL

   while(ACLKCONbits.APLLCK != 1);  // Wait for Auxiliary PLL to Lock
   __delay_us(50);                  // Errata#1 (reference:DS80000656A)
}  // finish -- tested
void SetupIOPins(){
   
    //--------------- Pin Assignment -----------------
    
    PPSUnLock;
    
    PPSOutput(OUT_FN_PPS_U1TX, OUT_PIN_PPS_RP40);
    PPSInput(IN_FN_PPS_U1RX, IN_PIN_PPS_RP47);
    
    PPSLock; 
    
        
    //-------------------------------------------------
}   // finish -- tested

void InitPWM1(void){
    // IOCONx: PWMx I/O CONTROL REGISTER
    IOCON1bits.PENH = 1;                // PWM2H is controlled by I/O module
                                        //1 = PWMx module controls the PWMxH pin
                                        //0 = GPIO module controls the PWMxH pin
    IOCON1bits.PENL = 1;                // PWM2L is controlled by I/O module
                                        //1 = PWMx module controls the PWMxL pin
                                        //0 = GPIO module controls the PWMxL pin
    IOCON1bits.PMOD = 3;                // Redundant Mode w/ PWMH not used
                                        //11 = PWMx I/O pin pair is in the True Independent Output mode
                                        //10 = PWMx I/O pin pair is in the Push-Pull Output mode
                                        //01 = PWMx I/O pin pair is in the Redundant Output mode
                                        //00 = PWMx I/O pin pair is in the Complementary Output mode
    IOCON1bits.POLH = 0;                // PWMxH Output Pin Polarity bit
                                        //1 = PWMxH pin is active-low
                                        //0 = PWMxH pin is active-high
    IOCON1bits.POLL = 0;                // PWMxL Output Pin Polarity bit
                                        //1 = PWMxL pin is active-low
                                        //0 = PWMxL pin is active-high
    IOCON1bits.OVRENL = 0;              // Override Enable for PWMxL Pin bit
                                        //1 = OVRDAT0 provides data for output on the PWMxL pin
                                        //0 = PWMx generator provides data for the PWMxL pin
    IOCON1bits.OVRDAT = 0b00;            // Data for PWMxH, PWMxL Pins if Override is Enabled bits
                                        //If OVERENH = 1, OVRDAT1 provides data for the PWMxH pin
                                        //If OVERENL = 1, OVRDAT0 provides data for the PWMxL pin
    /// NOT SUPPORTED IN INDEPENDENT MODE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    PWMCON1bits.DTC = 2;                // Dead-Time Control bits
                                        //11 = Reserved
                                        //10 = Dead-time function is disabled
                                        //01 = Negative dead time is actively applied for Complementary Output mode
                                        //00 = Positive dead time is actively applied for all Output modes
                        DTR1    = 2000;   // Unsigned 14-Bit Dead-Time Value for PWMx Dead-Time Unit bits
                        ALTDTR1 = 80;  // Unsigned 14-Bit Dead-Time Value for PWMx Dead-Time Unit bits
    PWMCON1bits.IUE = 1;                // Immediate Update Enable bit
                                        //1 = Updates to the active Duty Cycle, Phase Offset, Dead-Time and local Time Base Period registers are immediate
                                        //0 = Updates to the active Duty Cycle, Phase Offset, Dead-Time and local Time Base Period registers are synchronized to the local PWMx time base
    PWMCON1bits.ITB = 0;                // Independent Time Base Mode bit(3)
                                        //1 = PHASEx/SPHASEx registers provide the time base period for this PWMx generator
                                        //0 = PTPER register provides timing for this PWMx generator
    
    // FCLCONx: PWMx FAULT CURRENT-LIMIT CONTROL REGISTER
    FCLCON1bits.FLTSRC = 0;       // Fault Control Signal Source Select for PWMx Generator # bits
                                        //11111 = Reserved
                                        //10001 = Reserved
                                        //10000 = Analog Comparator 4
                                        //01111 = Analog Comparator 3
                                        //01110 = Analog Comparator 2
                                        //01101 = Analog Comparator 1
                                        //01100 = Fault 12
                                        //01011 = Fault 11
                                        //01010 = Fault 10
                                        //01001 = Fault 9
                                        //01000 = Fault 8
                                        //00111 = Fault 7
                                        //00110 = Fault 6
                                        //00101 = Fault 5
                                        //00100 = Fault 4
                                        //00011 = Fault 3
                                        //00010 = Fault 2
                                        //00001 = Fault 1
                                        //00000 = Reserved
    FCLCON1bits.FLTPOL = 0;             // Fault Signal is active-high
    FCLCON1bits.FLTMOD = 3;             // Fault Mode for PWMx Generator # bits
                                        //11 = Fault input is disabled
                                        //10 = Reserved
                                        //01 = The selected Fault source forces the PWMxH, PWMxL pins to FLTDATx values (cycle)
                                        //00 = The selected Fault source forces the PWMxH, PWMxL pins to FLTDATx values (latched condition)
    
    //PWMx LEADING-EDGE BLANKING (LEB) CONTROL REGISTER
    LEBCON1bits.PLR      =  1;          // PWMxL Rising Edge Trigger Enable bit
                                        //1 = Rising edge of PWMxL will trigger the Leading-Edge Blanking counter
                                        //0 = Leading-Edge Blanking ignores the rising edge of PWMxL
    LEBCON1bits.FLTLEBEN =  1;          // Fault Input Leading-Edge Blanking Enable bit
                                        //1 = Leading-Edge Blanking is applied to the selected Fault input
                                        //0 = Leading-Edge Blanking is not applied to the selected Fault input
    LEBDLY1bits.LEB      = 20;          // 8.32n Steps x 20 = 160ns
                                        //Leading-Edge Blanking Delay for Current-Limit and Fault Inputs bits (The value is in 8.32 ns increments)

    //TRGCONx: PWMx TRIGGER CONTROL REGISTER (x = 1 to 5)
    TRGCON1bits.TRGDIV  = 1;            // Trigger # Output Divider bits
                                        //1111 = Trigger output for every 16th trigger event
                                        //1110 = Trigger output for every 15th trigger event
                                        //1101 = Trigger output for every 14th trigger event
                                        //1100 = Trigger output for every 13th trigger event
                                        //1011 = Trigger output for every 12th trigger event
                                        //1010 = Trigger output for every 11th trigger event
                                        //1001 = Trigger output for every 10th trigger event
                                        //1000 = Trigger output for every 9th trigger event
                                        //0111 = Trigger output for every 8th trigger event
                                        //0110 = Trigger output for every 7th trigger event
                                        //0101 = Trigger output for every 6th trigger event
                                        //0100 = Trigger output for every 5th trigger event
                                        //0011 = Trigger output for every 4th trigger event
                                        //0010 = Trigger output for every 3rd trigger event
                                        //0001 = Trigger output for every 2nd trigger event
                                        //0000 = Trigger output for every trigger event
    TRGCON1bits.TRGSTRT = 1;            // Trigger Postscaler Start Enable Select bits
                                        //111111 = Wait 63 PWM cycles before generating the first trigger event after the module is enabled
                                        //?
                                        //?
                                        //?
                                        //000010 = Wait 2 PWM cycles before generating the first trigger event after the module is enabled
                                        //000001 = Wait 1 PWM cycle before generating the first trigger event after the module is enabled
                                        //000000 = Wait 0 PWM cycles before generating the first trigger event after the module is enabled
   
    
    // PTPER: PWMx PRIMARY MASTER TIME BASE PERIOD REGISTER
    /*
        Note 1: The PWMx time base has a minimum value of 0x0010 and a maximum value of 0xFFF8.
        Note 2: Any period value that is less than 0x0028 must have the Least Significant 3 bits set to ?0?, thus yielding a
        period resolution at 8.32 ns (at fastest auxiliary clock rate).
    */
    
    // PHASEx: PWMx PRIMARY PHASE-SHIFT REGISTER (x = 1 to 5)
    /*
    Note 1: If PWMCONx<9> = 0, the following applies based on the mode of operation:
       ? Complementary, Redundant and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
       PHASEx<15:0> = Phase-shift value for PWMxH and PWMxL outputs
       ? True Independent Output mode (IOCONx<11:10> = 11); PHASEx<15:0> = Phase-shift value for
       PWMxH only
       ? When the PHASEx/SPHASEx registers provide the phase shift with respect to the master time base;
       therefore, the valid range is 0x0000 through period
    Note 2: If PWMCONx<9> = 1, the following applies based on the mode of operation:
       ? Complementary, Redundant, and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
       PHASEx<15:0> = Independent time base period value for PWMxH and PWMxL
       ? True Independent Output mode (IOCONx<11:10> = 11); PHASEx<15:0> = Independent time base
       period value for PWMxH only
       ? When the PHASEx/SPHASEx registers provide the local period, the valid range is 0x0000 through
       0xFFF8
     */ 
    
    // SPHASEx: PWMx SECONDARY PHASE-SHIFT REGISTER (x = 1 to 5)
    /*
    Note 1: If PWMCONx<9> = 0, the following applies based on the mode of operation:
        ? Complementary, Redundant and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
        SPHASEx<15:0> = Not used
        ? True Independent Output mode (IOCONx<11:10> = 11), PHASEx<15:0> = Phase-shift value for
        PWMxL only
    Note 2: If PWMCONx<9> = 1, the following applies based on the mode of operation:
        ? Complementary, Redundant and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
        SPHASEx<15:0> = Not used
        ? True Independent Output mode (IOCONx<11:10> = 11); PHASEx<15:0> = Independent time base
        period value for PWMxL only
        ? When the PHASEx/SPHASEx registers provide the local period, the valid range of values is
        0x0010-0xFFF8  
    */ 
   
    PHASE1 = PTPER>>1;			// Introduce phase shift from Buck converter to reduce Vin distortion
                                        // 180 Deg phase-shift from Buck converter
    
    // PDCx: PWMx GENERATOR DUTY CYCLE REGISTER (x = 1 to 5)
    /*
        Note 1: In Independent PWM mode, the PDCx register controls the PWMxH duty cycle only. In the
        Complementary, Redundant and Push-Pull PWM modes, the PDCx register controls the duty cycle of both
        the PWMxH and PWMxL.
        Note 2: The smallest pulse width that can be generated on the PWMx output corresponds to a value of 0x0008,
        while the maximum pulse width generated corresponds to a value of Period ? 0x0008.
        Note 3: As the duty cycle gets closer to 0% or 100% of the PWMx period (0 to 40 ns, depending on the mode of
        operation), PWMx duty cycle resolution will increase from 1 to 3 LSBs.
    */
    
    PDC1 = PTPER/4;
    PHASE1 = PDC1 +60;
    
    SPHASE1 = 0;
    SDC1 = PDC1;
            
    // TRIGx: PWMx PRIMARY TRIGGER COMPARE VALUE REGISTER
    TRIG1 = 0;                     // Trigger Compare Value bits
                                    //When the primary PWMx functions in the local time base, this register contains the compare values
                                    //that can trigger the ADC module.
}
void InitPWM2(void){
    // IOCONx: PWMx I/O CONTROL REGISTER
    IOCON2bits.PENH = 1;                // 
                                        //1 = PWMx module controls the PWMxH pin
                                        //0 = GPIO module controls the PWMxH pin
    IOCON2bits.PENL = 1;                // 
                                        //1 = PWMx module controls the PWMxL pin
                                        //0 = GPIO module controls the PWMxL pin
    IOCON2bits.PMOD = 3;                // PWM MODE 
                                        //11 = PWMx I/O pin pair is in the True Independent Output mode
                                        //10 = PWMx I/O pin pair is in the Push-Pull Output mode
                                        //01 = PWMx I/O pin pair is in the Redundant Output mode
                                        //00 = PWMx I/O pin pair is in the Complementary Output mode
    IOCON2bits.POLH = 1;                // PWMxH Output Pin Polarity bit
                                        //1 = PWMxH pin is active-low
                                        //0 = PWMxH pin is active-high
    IOCON2bits.POLL = 1;                // PWMxL Output Pin Polarity bit
                                        //1 = PWMxL pin is active-low
                                        //0 = PWMxL pin is active-high
    IOCON2bits.OVRENL = 0;              // Override Enable for PWMxL Pin bit
                                        //1 = OVRDAT0 provides data for output on the PWMxL pin
                                        //0 = PWMx generator provides data for the PWMxL pin
    IOCON2bits.OVRDAT = 0b00;            // Data for PWMxH, PWMxL Pins if Override is Enabled bits
                                        //If OVERENH = 1, OVRDAT1 provides data for the PWMxH pin
                                        //If OVERENL = 1, OVRDAT0 provides data for the PWMxL pin
    /// NOT SUPPORTED IN INDEPENDENT MODE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    PWMCON2bits.DTC = 2;                // Dead-Time Control bits
                                        //11 = Reserved
                                        //10 = Dead-time function is disabled
                                        //01 = Negative dead time is actively applied for Complementary Output mode
                                        //00 = Positive dead time is actively applied for all Output modes
                        DTR2    = 2000;   // Unsigned 14-Bit Dead-Time Value for PWMx Dead-Time Unit bits
                        ALTDTR2 = 80;  // Unsigned 14-Bit Dead-Time Value for PWMx Dead-Time Unit bits
    PWMCON2bits.IUE = 1;                // Immediate Update Enable bit
                                        //1 = Updates to the active Duty Cycle, Phase Offset, Dead-Time and local Time Base Period registers are immediate
                                        //0 = Updates to the active Duty Cycle, Phase Offset, Dead-Time and local Time Base Period registers are synchronized to the local PWMx time base
    PWMCON2bits.ITB = 1;                // Independent Time Base Mode bit(3)
                                        //1 = PHASEx/SPHASEx registers provide the time base period for this PWMx generator
                                        //0 = PTPER register provides timing for this PWMx generator
    
    // FCLCONx: PWMx FAULT CURRENT-LIMIT CONTROL REGISTER
    FCLCON2bits.FLTSRC = 0;       // Fault Control Signal Source Select for PWMx Generator # bits
                                        //11111 = Reserved
                                        //10001 = Reserved
                                        //10000 = Analog Comparator 4
                                        //01111 = Analog Comparator 3
                                        //01110 = Analog Comparator 2
                                        //01101 = Analog Comparator 1
                                        //01100 = Fault 12
                                        //01011 = Fault 11
                                        //01010 = Fault 10
                                        //01001 = Fault 9
                                        //01000 = Fault 8
                                        //00111 = Fault 7
                                        //00110 = Fault 6
                                        //00101 = Fault 5
                                        //00100 = Fault 4
                                        //00011 = Fault 3
                                        //00010 = Fault 2
                                        //00001 = Fault 1
                                        //00000 = Reserved
    FCLCON2bits.FLTPOL = 0;             // Fault Signal is active-high
    FCLCON2bits.FLTMOD = 3;             // Fault Mode for PWMx Generator # bits
                                        //11 = Fault input is disabled
                                        //10 = Reserved
                                        //01 = The selected Fault source forces the PWMxH, PWMxL pins to FLTDATx values (cycle)
                                        //00 = The selected Fault source forces the PWMxH, PWMxL pins to FLTDATx values (latched condition)
    
    //PWMx LEADING-EDGE BLANKING (LEB) CONTROL REGISTER
    LEBCON2bits.PLR      =  1;          // PWMxL Rising Edge Trigger Enable bit
                                        //1 = Rising edge of PWMxL will trigger the Leading-Edge Blanking counter
                                        //0 = Leading-Edge Blanking ignores the rising edge of PWMxL
    LEBCON2bits.FLTLEBEN =  1;          // Fault Input Leading-Edge Blanking Enable bit
                                        //1 = Leading-Edge Blanking is applied to the selected Fault input
                                        //0 = Leading-Edge Blanking is not applied to the selected Fault input
    LEBDLY2bits.LEB      = 20;          // 8.32n Steps x 20 = 160ns
                                        //Leading-Edge Blanking Delay for Current-Limit and Fault Inputs bits (The value is in 8.32 ns increments)

    //TRGCONx: PWMx TRIGGER CONTROL REGISTER (x = 1 to 5)
    TRGCON2bits.TRGDIV  = 1;            // Trigger # Output Divider bits
                                        //1111 = Trigger output for every 16th trigger event
                                        //1110 = Trigger output for every 15th trigger event
                                        //1101 = Trigger output for every 14th trigger event
                                        //1100 = Trigger output for every 13th trigger event
                                        //1011 = Trigger output for every 12th trigger event
                                        //1010 = Trigger output for every 11th trigger event
                                        //1001 = Trigger output for every 10th trigger event
                                        //1000 = Trigger output for every 9th trigger event
                                        //0111 = Trigger output for every 8th trigger event
                                        //0110 = Trigger output for every 7th trigger event
                                        //0101 = Trigger output for every 6th trigger event
                                        //0100 = Trigger output for every 5th trigger event
                                        //0011 = Trigger output for every 4th trigger event
                                        //0010 = Trigger output for every 3rd trigger event
                                        //0001 = Trigger output for every 2nd trigger event
                                        //0000 = Trigger output for every trigger event
    TRGCON2bits.TRGSTRT = 1;            // Trigger Postscaler Start Enable Select bits
                                        //111111 = Wait 63 PWM cycles before generating the first trigger event after the module is enabled
                                        //?
                                        //?
                                        //?
                                        //000010 = Wait 2 PWM cycles before generating the first trigger event after the module is enabled
                                        //000001 = Wait 1 PWM cycle before generating the first trigger event after the module is enabled
                                        //000000 = Wait 0 PWM cycles before generating the first trigger event after the module is enabled
   
    
    // PTPER: PWMx PRIMARY MASTER TIME BASE PERIOD REGISTER
    /*
        Note 1: The PWMx time base has a minimum value of 0x0010 and a maximum value of 0xFFF8.
        Note 2: Any period value that is less than 0x0028 must have the Least Significant 3 bits set to ?0?, thus yielding a
        period resolution at 8.32 ns (at fastest auxiliary clock rate).
    */
    
    // PHASEx: PWMx PRIMARY PHASE-SHIFT REGISTER (x = 1 to 5)
    /*
    Note 1: If PWMCONx<9> = 0, the following applies based on the mode of operation:
       ? Complementary, Redundant and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
       PHASEx<15:0> = Phase-shift value for PWMxH and PWMxL outputs
       ? True Independent Output mode (IOCONx<11:10> = 11); PHASEx<15:0> = Phase-shift value for
       PWMxH only
       ? When the PHASEx/SPHASEx registers provide the phase shift with respect to the master time base;
       therefore, the valid range is 0x0000 through period
    Note 2: If PWMCONx<9> = 1, the following applies based on the mode of operation:
       ? Complementary, Redundant, and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
       PHASEx<15:0> = Independent time base period value for PWMxH and PWMxL
       ? True Independent Output mode (IOCONx<11:10> = 11); PHASEx<15:0> = Independent time base
       period value for PWMxH only
       ? When the PHASEx/SPHASEx registers provide the local period, the valid range is 0x0000 through
       0xFFF8
     */ 
    
    // SPHASEx: PWMx SECONDARY PHASE-SHIFT REGISTER (x = 1 to 5)
    /*
    Note 1: If PWMCONx<9> = 0, the following applies based on the mode of operation:
        ? Complementary, Redundant and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
        SPHASEx<15:0> = Not used
        ? True Independent Output mode (IOCONx<11:10> = 11), PHASEx<15:0> = Phase-shift value for
        PWMxL only
    Note 2: If PWMCONx<9> = 1, the following applies based on the mode of operation:
        ? Complementary, Redundant and Push-Pull Output mode (IOCONx<11:10> = 00, 01 or 10);
        SPHASEx<15:0> = Not used
        ? True Independent Output mode (IOCONx<11:10> = 11); PHASEx<15:0> = Independent time base
        period value for PWMxL only
        ? When the PHASEx/SPHASEx registers provide the local period, the valid range of values is
        0x0010-0xFFF8  
    */ 
   
   // PHASE2 = PTPER>>1;			// Introduce phase shift from Buck converter to reduce Vin distortion
                                        // 180 Deg phase-shift from Buck converter
    
    // PDCx: PWMx GENERATOR DUTY CYCLE REGISTER (x = 1 to 5)
    /*
        Note 1: In Independent PWM mode, the PDCx register controls the PWMxH duty cycle only. In the
        Complementary, Redundant and Push-Pull PWM modes, the PDCx register controls the duty cycle of both
        the PWMxH and PWMxL.
        Note 2: The smallest pulse width that can be generated on the PWMx output corresponds to a value of 0x0008,
        while the maximum pulse width generated corresponds to a value of Period ? 0x0008.
        Note 3: As the duty cycle gets closer to 0% or 100% of the PWMx period (0 to 40 ns, depending on the mode of
        operation), PWMx duty cycle resolution will increase from 1 to 3 LSBs.
    */
    
    PDC2 = PTPER/4;
    PHASE2 = PDC2 +60;
    
    SPHASE2 = 0;
    SDC2 = PDC2;
            
    // TRIGx: PWMx PRIMARY TRIGGER COMPARE VALUE REGISTER
    TRIG2 = 0;                     // Trigger Compare Value bits
                                    //When the primary PWMx functions in the local time base, this register contains the compare values
                                    //that can trigger the ADC module.
}
void InitPWM3(void){
    // PWM3 only used as clock source for triggering AN4/AN7

    #if(BOOST == ENABLED)
    PHASE3 = PTPER >> 1;
    TRGCON3bits.TRGSTRT = 2;    // Trigger generated after waiting 3 PWM cycles

    #else
    TRGCON3bits.TRGSTRT = 0;    // Trigger generated after waiting 0 PWM cycles

    #endif

    TRIG3 = 500;
    TRGCON3bits.TRGDIV = 5;    // Trigger interrupt generated once every 6 PWM cycle
    FCLCON3bits.FLTMOD = 3;
    IOCON3bits.PMOD    = 1;

    IOCON3bits.PENL = 0;        // Give Ownership of PWM3 to device pin
    IOCON3bits.PENH = 0;
} // get source for analogs here
void InitTimer(void){
    //Load Dynamics applied after the softstart

    #if((BUCKDYNAMICLOAD == ENABLED) || (BOOSTDYNAMICLOAD == ENABLED))
    PR1 = TMRPERIOD;
    T1CONbits.TCKPS = TMRSCALER;
    T1CONbits.TCS = 0;              // Clock source is internal Fcy

    IFS0bits.T1IF = 0;
    IPC0bits.T1IP = 4;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;
    #endif

    // Configure Timer 2 for soft start
    PR2 = TMR2PERIOD;
    T2CONbits.TCKPS = TMR2SCALER;
    T2CONbits.TCS = 0;              // Clock source is internal Fcy

    IFS0bits.T2IF = 0;
    IPC1bits.T2IP = 4;
    IEC0bits.T2IE = 0;   
} // wip
void InitBuckPWM(void){
    IOCON1bits.PENH = 0;            // GPIO controls I/O port
    IOCON1bits.PENL = 0;           

    IOCON1bits.PMOD = 0;            // Complementary Mode
    
    IOCON1bits.POLH = 0;            // Drive signals are active-high
    IOCON1bits.POLL = 0;            // Drive signals are active-high

    IOCON1bits.OVRENH = 0;          // Override disabled
    IOCON1bits.OVRENL = 0;	  
    IOCON1bits.OVRDAT = 0b00;       // Override data PWMH and PWML
    IOCON1bits.FLTDAT = 0b01;       // If fault occurs:
                                    // PWMH = 0 & PWML = 1

    PWMCON1bits.DTC   = 0;          // Positive Deadtime enabled
    DTR1    = 80;
    ALTDTR1 = 110;
				  
    PWMCON1bits.IUE = 0;            // Disable Immediate duty cycle updates
    PWMCON1bits.ITB = 0;            // Select Primary Timebase mode

    FCLCON1bits.FLTSRC = 0b01101;   // Fault Control Signal assigned to CMP1
    FCLCON1bits.FLTPOL = 0;         // Fault Signal is active-high
    FCLCON1bits.FLTMOD = 1;         // Cycle-by-Cycle current limiting

    // In order to block the sensed current spike at the
    // turn on edge of the HS FET, use Leading Edge blanking.
    LEBCON1bits.PHR      = 1;       // Enable LEB bit for HS MOSFET rising edge
    LEBCON1bits.FLTLEBEN = 1;       // Fault Input LEB Enabled
    LEBDLY1bits.LEB      = 20;      // 8.32n Steps x 20 = 160ns

    TRGCON1bits.TRGDIV  = 1;        // Trigger interrupt generated every 2 PWM cycles

    TRGCON1bits.TRGSTRT = 0;        // Trigger generated after waiting 0 PWM cycles

    // Initialize PDC1 depending on configuration

    #if(OPENLOOP == ENABLED)
    PDC1 = BUCKOPENLOOPDC;
    #else
    PDC1 = BUCKMAXDUTYCYCLE;
    #endif

    TRIG1 = 30;                     // Set Initial Trigger location

} // get triggers from here 
void InitBoostPWM(void){
    IOCON2bits.PENH = 0;                // PWM2H is controlled by I/O module
    IOCON2bits.PENL = 0;                // PWM2L is controlled by I/O module

    IOCON2bits.PMOD = 1;                // Redundant Mode w/ PWMH not used
    IOCON2bits.POLL = 0;                // Drive signal- active-high

    IOCON2bits.OVRENL = 0;		// Override disabled
    IOCON2bits.OVRDAT = 0b00;           // Override data:
                                        // PWMH = 0 & PWML = 0
    PWMCON2bits.DTC = 2;                // Deadtime disabled

    PWMCON2bits.IUE = 0;                // Disable Immediate duty cycle updates
    PWMCON2bits.ITB = 0;                // Select Primary Timebase mode
    
    FCLCON2bits.FLTSRC = 0b01110;       // Fault Control Signal assigned to CMP2
    FCLCON2bits.FLTPOL = 0;             // Fault Signal is active-high
    FCLCON2bits.FLTMOD = 1;             // Cycle-by-Cycle Fault Mode

    //Enable LEB to blank (mask) any turn-on spike
    LEBCON2bits.PLR      =  1;          // Enable LEB bit for PWML
    LEBCON2bits.FLTLEBEN =  1;          // Fault Input LEB Enabled
    LEBDLY2bits.LEB      = 20;          // 8.32n Steps x 20 = 160ns

    TRGCON2bits.TRGDIV  = 1;            // Trigger interrupt generated every 2 PWM cycles

    TRGCON2bits.TRGSTRT = 1;            // Trigger generated after waiting 1 PWM cycles

    PHASE2 = PTPER>>1;			// Introduce phase shift from Buck converter to reduce Vin distortion
                                        // 180 Deg phase-shift from Buck converter

    // Initialize PDC1 depending on configuration

    #if(OPENLOOP == ENABLED)
    PDC2 = BOOSTOPENLOOPDC;
    #else
    PDC2  = BOOSTMAXDUTYCYCLE;
    #endif

    TRIG2 = 30;                        // Trigger generated at beginning of PWM period
} // get triggers from here 
void InitCMP(void)   // Buck & Boost overcurrent protection
{
    #if(BUCK == ENABLED)
    CMP1CONbits.INSEL  = 0;          // Input: CMP1A
    CMP1CONbits.HYSSEL = 1;          // 5mV of Hysteresis

    CMP1CONbits.RANGE = 1;           // AVdd is the max DACx output voltage
    
    CMP1DAC = BUCKMAXCURRENTDAC;     // Buck DAC voltage for overcurrent
                                     // 2964 counts (2.39V)
    CMP1CONbits.CMPON = 1;
    #endif

    #if(BOOST == ENABLED)
    CMP2CONbits.INSEL  = 0;          // Input: CMP2A
    CMP2CONbits.HYSSEL = 1;          // 5mV of Hysteresis

    CMP2CONbits.RANGE = 1;           // AVdd is the max DACx output voltage

    CMP2DAC = BOOSTMAXCURRENTDAC;    // Boost DAC voltage for overcurrent
                                     // 2904 counts (2.34V)
    CMP2CONbits.CMPON = 1;
    
    #endif
}
void InitADC(void)
{
    // Setup ADC Clock Input Max speed of 70 MHz --> Fosc = 140 MHz
    ADCON3Hbits.CLKSEL  = 1;    // 0-Fsys, 1-Fosc, 2-FRC, 3-APLL
    ADCON3Hbits.CLKDIV  = 0;    // Global Clock divider (1:1)
    ADCORE0Hbits.ADCS   = 0;    // Core 0 clock divider (1:2)
    ADCORE1Hbits.ADCS   = 0;    // Core 1 clock divider (1:2)
    ADCORE2Hbits.ADCS   = 0;    // Core 2 clock divider (1:2)
    ADCORE3Hbits.ADCS   = 0;    // Core 3 clock divider (1:2)
    ADCON2Lbits.SHRADCS = 0;    // 1/2 clock divider

    ADCON1Hbits.FORM    = 0;    // Integer format
    ADCON3Lbits.REFSEL  = 0;    // AVdd as voltage reference

     // ADC Cores in 12-bit resolution mode
    ADCON1Hbits.SHRRES  = 3;    // Shared ADC Core in 12-bit resolution mode
    ADCORE0Hbits.RES    = 3;    // Core 0 ADC Core in 12-bit resolution mode
    ADCORE1Hbits.RES    = 3;    // Core 1 ADC Core in 12-bit resolution mode
    ADCORE2Hbits.RES    = 3;    // Core 2 ADC Core in 12-bit resolution mode
    ADCORE3Hbits.RES    = 3;    // Core 3 ADC Core in 12-bit resolution mode
    ADCON2Hbits.SHRSAMC = 2;    // Shared ADC Core sample time 4Tad

    // Configure ANx for unsigned format and single ended (0,0)
    ADMOD0L = 0x0000;
    
    /*
      ADIELbits.IE0 = 1; // enable interrupt for AN0
    ANSELAbits.ANSA0 = 1; TRISAbits.TRISA0 = 1; // AN0/RA0 connected the dedicated core 0
    ANSELAbits.ANSA1 = 1; TRISAbits.TRISA1 = 1; // AN1/RA1 connected the dedicated core 1   
        // Configure the common ADC clock.
    ADCON3Hbits.CLKSEL = 2; // clock from FRC oscillator
    ADCON3Hbits.CLKDIV = 0; // no clock divider (1:1)
    // Configure the cores? ADC clock.
    ADCORE0Hbits.ADCS = 0; // clock divider (1:2)
    ADCORE1Hbits.ADCS = 0; // clock divider (1:2)
    // Configure the ADC reference sources.
    ADCON3Lbits.REFSEL = 0; // AVdd as voltage reference
    // Configure the integer of fractional output format.
    ADCON1Hbits.FORM = 0; // integer format
    // Select single-ended input configuration and unsigned output format.
    ADMOD0Lbits.SIGN0 = 0; // AN0/RA0
    ADMOD0Lbits.DIFF0 = 0; // AN0/RA0
    ADMOD0Lbits.SIGN1 = 0; // AN1/RA1
    ADMOD0Lbits.DIFF1 = 0; // AN1/RA1
     */

    InitCalibrateADC();

    
#if(BUCK == ENABLED)
    ADTRIG0Lbits.TRGSRC0 = 5;    // ADC AN0 triggered by PWM1
    ADTRIG0Lbits.TRGSRC1 = 5;    // ADC AN1 triggered by PWM1

    IFS6bits.ADCAN1IF  = 0;      // Clear ADC interrupt flag
    IPC27bits.ADCAN1IP = 7;      // Set Buck ADC interrupt priority to 7

#endif

#if (BOOST == ENABLED)
    ADTRIG0Hbits.TRGSRC2 = 6;    // ADC AN2/AN3 triggered by PWM2
    ADTRIG0Hbits.TRGSRC3 = 1;    // Initial trigger by SW for softstart

    IFS7bits.ADCAN3IF    = 0;    // Clear ADC interrupt flag
    IPC28bits.ADCAN3IP   = 6;    // Set Boost ADC interrupt priority

#endif
    ADTRIG1Lbits.TRGSRC4 = 7;    // ADC AN4/AN7 triggered by PWM3
    ADTRIG1Hbits.TRGSRC7 = 7;    // for synchronizing trigger event

    IFS7bits.ADCAN7IF    = 0;    // Clear ADC interrupt flag
    IPC29bits.ADCAN7IP   = 5;	 // Set ADC interrupt priority
    IEC7bits.ADCAN7IE    = 1;	 // Enable the ADC AN7 interrupt
    ADIELbits.IE7        = 1;    // Enable ADC Common Interrupt
}
void InitCalibrateADC(void)
{
    // Power Up delay: 2048 Core Clock Source Periods (TCORESRC) for all ADC cores
    // (~14.6 us)
    _WARMTIME = 11;

    // Turn on ADC module
    ADCON1Lbits.ADON  = 1;

    // Turn on analog power for dedicated core 0
    ADCON5Lbits.C0PWR = 1;
    while(ADCON5Lbits.C0RDY == 0);
    ADCON3Hbits.C0EN  = 1;   // Enable ADC core 0

    // Turn on analog power for dedicated core 1
    ADCON5Lbits.C1PWR = 1;
    while(ADCON5Lbits.C1RDY == 0);
    ADCON3Hbits.C1EN  = 1;   // Enable ADC core 1

    // Turn on analog power for dedicated core 2
    ADCON5Lbits.C2PWR = 1;
    while(ADCON5Lbits.C2RDY == 0);
    ADCON3Hbits.C2EN  = 1;   // Enable ADC core 2

    // Turn on analog power for dedicated core 3
    ADCON5Lbits.C3PWR = 1;
    while(ADCON5Lbits.C3RDY == 0);
    ADCON3Hbits.C3EN  = 1;   // Enable ADC core 3

    // Turn on analog power for shared core
    ADCON5Lbits.SHRPWR = 1;
    while(ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN  = 1;  // Enable shared ADC core

    // Enable calibration for the dedicated core 0
    ADCAL0Lbits.CAL0EN   = 1;
    ADCAL0Lbits.CAL0DIFF = 0;         // Single-ended input calibration
    ADCAL0Lbits.CAL0RUN  = 1;         // Start Cal
    while(ADCAL0Lbits.CAL0RDY == 0);
    ADCAL0Lbits.CAL0EN   = 0;         // Cal complete

    // Enable calibration for the dedicated core 1
    ADCAL0Lbits.CAL1EN   = 1;
    ADCAL0Lbits.CAL1DIFF = 0;         // Single-ended input calibration
    ADCAL0Lbits.CAL1RUN  = 1;         // Start Cal
    while(ADCAL0Lbits.CAL1RDY == 0);
    ADCAL0Lbits.CAL1EN   = 0;         // Cal complete

    // Enable calibration for the dedicated core 2
    ADCAL0Hbits.CAL2EN   = 1;
    ADCAL0Hbits.CAL2DIFF = 0;         // Single-ended input calibration
    ADCAL0Hbits.CAL2RUN  = 1;         // Start Cal
    while(ADCAL0Hbits.CAL2RDY == 0);
    ADCAL0Hbits.CAL2EN   = 0;         // Cal complete

    // Enable calibration for the dedicated core 3
    ADCAL0Hbits.CAL3EN   = 1;
    ADCAL0Hbits.CAL3DIFF = 0;         // Single-ended input calibration
    ADCAL0Hbits.CAL3RUN  = 1;         // Start Cal
    while(ADCAL0Hbits.CAL3RDY == 0);
    ADCAL0Hbits.CAL3EN   = 0;         // Cal complete

    // Enable calibration for the shared core
    ADCAL1Hbits.CSHREN   = 1;
    ADCAL1Hbits.CSHRDIFF = 0;        // Single-ended input calibration
    ADCAL1Hbits.CSHRRUN  = 1;        // Start calibration cycle
    while(ADCAL1Hbits.CSHRRDY == 0); // while calibration is still in progress

    ADCAL1Hbits.CSHREN   = 0;        // Calibration is complete
        /*
         // Power Up delay: 2048 Core Clock Source Periods (TCORESRC) for all ADC cores
    // (~14.6 us)
    _WARMTIME = 15;

    // Turn on ADC module
    ADCON1Lbits.ADON  = 1;

    // Turn on analog power for dedicated core 0
    ADCON5Lbits.C0PWR = 1;
    while(ADCON5Lbits.C0RDY == 0);
    ADCON3Hbits.C0EN  = 1;   // Enable ADC core 0

    // Turn on analog power for dedicated core 1
    ADCON5Lbits.C1PWR = 1;
    while(ADCON5Lbits.C1RDY == 0);
    ADCON3Hbits.C1EN  = 1;   // Enable ADC core 1
     
    // Turn on analog power for dedicated core 2
    ADCON5Lbits.C2PWR = 1;
    while(ADCON5Lbits.C2RDY == 0);
    ADCON3Hbits.C2EN  = 1;   // Enable ADC core 2

    // Turn on analog power for dedicated core 3
    ADCON5Lbits.C3PWR = 1;
    while(ADCON5Lbits.C3RDY == 0);
    ADCON3Hbits.C3EN  = 1;   // Enable ADC core 3

    // Turn on analog power for shared core
    ADCON5Lbits.SHRPWR = 1;
    while(ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN  = 1;  // Enable shared ADC core
    */
    // Enable calibration for the dedicated core 0
    ADCAL0Lbits.CAL0EN   = 1;
    ADCAL0Lbits.CAL0DIFF = 0;         // Single-ended input calibration
    ADCAL0Lbits.CAL0RUN  = 1;         // Start Cal
    while(ADCAL0Lbits.CAL0RDY == 0);
    ADCAL0Lbits.CAL0EN   = 0;         // Cal complete
    // Enable calibration for the dedicated core 1
    ADCAL0Lbits.CAL1EN   = 1;
    ADCAL0Lbits.CAL1DIFF = 0;         // Single-ended input calibration
    ADCAL0Lbits.CAL1RUN  = 1;         // Start Cal
    while(ADCAL0Lbits.CAL1RDY == 0);
    ADCAL0Lbits.CAL1EN   = 0;         // Cal complete
    
    /*
    // Enable calibration for the dedicated core 2
    ADCAL0Hbits.CAL2EN   = 1;
    ADCAL0Hbits.CAL2DIFF = 0;         // Single-ended input calibration
    ADCAL0Hbits.CAL2RUN  = 1;         // Start Cal
    while(ADCAL0Hbits.CAL2RDY == 0);
    ADCAL0Hbits.CAL2EN   = 0;         // Cal complete

    // Enable calibration for the dedicated core 3
    ADCAL0Hbits.CAL3EN   = 1;
    ADCAL0Hbits.CAL3DIFF = 0;         // Single-ended input calibration
    ADCAL0Hbits.CAL3RUN  = 1;         // Start Cal
    while(ADCAL0Hbits.CAL3RDY == 0);
    ADCAL0Hbits.CAL3EN   = 0;         // Cal complete

    // Enable calibration for the shared core
    ADCAL1Hbits.CSHREN   = 1;
    ADCAL1Hbits.CSHRDIFF = 0;        // Single-ended input calibration
    ADCAL1Hbits.CSHRRUN  = 1;        // Start calibration cycle
    while(ADCAL1Hbits.CSHRRDY == 0); // while calibration is still in progress

    ADCAL1Hbits.CSHREN   = 0;        // Calibration is complete
     */
         
         
       
}
void init_ADC_INTS(void){
    /* 
     // Configure and enable ADC interrupts.
    ADIELbits.IE0 = 1; // enable interrupt for AN0
    ADIELbits.IE1 = 1; // enable interrupt for AN1
    _ADCAN0IF = 0; // clear interrupt flag for AN0
    _ADCAN0IE = 1; // enable interrupt for AN0
    _ADCAN1IF = 0; // clear interrupt flag for AN1
    _ADCAN1IE = 1; // enable interrupt for AN1
    // Set same trigger source for all inputs to sample signals simultaneously.
    ADTRIG0Lbits.TRGSRC0 = 13; // timer 2 for AN0
    ADTRIG0Lbits.TRGSRC1 = 13; // timer 2 for AN1
    // TIMER 2 INITIALIZATION (TIMER IS USED AS A TRIGGER SOURCE FOR ALL CHANNELS).
    T2CONbits.TCS = 0; // clock from peripheral clock
    T2CONbits.TCKPS = 0; // 1:1 prescale
    PR2 = 0x8000; // rollover every 0x8000 clocks
    T2CONbits.TON = 1; // start timer to generate ADC triggers
     
     */
    
} //change interrupt sources to PWM OUTPUT
void InitADCMP(void)
{
    // Digital Comparator 0 for temperature
    ADCMP0CONbits.HIHI   = 1;       // Interrupt when ADCBUF is greater than DCMP HI
    ADCMP0ENLbits.CMPEN7 = 1;       // ADCBUF7 processed by digital comparator

    ADCMP0HI = WARNINGTEMPADC;

    ADCMP0CONbits.CMPEN = 1;        // Enable digital comparator

    ADCMP0CONbits.IE    = 1;        // Enable interrupt for CMP
   

    IPC44bits.ADCMP0IP = 4;         // Set ADC Digital Comparator 0 Interrupt Priority Level to 4
    IFS11bits.ADCMP0IF = 0;         // Clear ADC Digital Comparator 0 Flag
    IEC11bits.ADCMP0IE = 1;         // Enable ADC Digital Comparator 0 Interrupt

   

    // Digital Comparator 1 for input voltage
    ADCMP1CONbits.HIHI   = 1;       // Interrupt when ADCBUF is greater than DCMP HI
    ADCMP1CONbits.LOLO   = 1;       // Interrupt when ADCBUF is less than DCMP LO
    ADCMP1ENLbits.CMPEN4 = 1;       // ADCBUF4 processed by digital comparator

    ADCMP1HI = INPUTOVERVOLTAGEADC;
    ADCMP1LO = INPUTUNDERVOLTAGEADC;

    ADCMP1CONbits.CMPEN = 1;        // Enable digital comparator

    ADCMP1CONbits.IE    = 1;        // Enable interrupt for CMP

    IPC44bits.ADCMP1IP = 4;         // Set ADC Digital Comparator 1 Interrupt Priority Level to 4
    IFS11bits.ADCMP1IF = 0;         // Clear ADC Digital Comparator 1 Flag
    IEC11bits.ADCMP1IE = 1;         // Enable ADC Digital Comparator 1 Interrupt
    
}
//////////////////////////////////////-I2C-/////////////////////////////////////
void InitI2C(void){
    
    I2C1CON1bits.I2CEN = 1;      // Enable I2C Module
  
    //Configure I2C as MASTER to communicate with Load IC
    I2C1CON1bits.A10M   = 0;     // 7-bit slave address/data
    I2C1CON1bits.DISSLW = 1;     // Slew rate control disabled

    I2C1BRG = I2CBAUDRATE;       // 100kHz

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
////////////////////////////////////-UART-//////////////////////////////////////
void Init_UART(){
    
    U1MODE = 0;
    U1STA = 0;
    U1BRG = BRGVAL;
    U1MODEbits.UARTEN = 1; 
    U1STAbits.UTXEN = 1;  
    IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;
    IEC0bits.U1TXIE = 0; 
    IEC0bits.U1RXIE = 0; 
    
}
////////////////////////////////-CODE JUNKYARD-/////////////////////////////////

/*
void ESP_W_C(unsigned char c){
}
void ESP_W(unsigned char *s){
    
   
}
unsigned char ESP_R_C(void){
    
}
unsigned char ESP_R(void){
    
}
*/
/*
     while (!U1STAbits.TRMT); // wait until transmit shift register is empty
    U1TXREG = c;               // write character to TXREG and start transmission

 */
/*
  while (*s)
    {
        ESP_W_C(*s);     // send character pointed to by s
        s++;                // increase pointer location to the next character
    }
 
 */
/* 
    U1STAbits.URXDA = 0;
    if(U1STAbits.URXDA == 1){ 
        RXData2[0] = U1RXREG;
        RXData2[1] = U1RXREG;
        RXData2[2] = U1RXREG;
        RXData2[3] = U1RXREG;
        RXData2[4] = U1RXREG;
        RXData2[5] = U1RXREG;
        RXData2[6] = U1RXREG;
        RXData2[7] = U1RXREG;
        RXData2[8] = U1RXREG;
        RXData2[9] = U1RXREG;
        RXData2[10] = U1RXREG;
        RXData2[11] = U1RXREG;
        RXData2[12] = U1RXREG;
        RXData2[13] = U1RXREG;
        RXData2[14] = U1RXREG;
        RXData2[15] = U1RXREG;
        return 1;
    }
    return 0;
     */
/*
    U1STAbits.URXDA = 0;
    RXCnt = 0;
    while(U1STAbits.URXDA == 1){ 
        RXData[RXCnt] = U1RXREG;
        RXCnt++;
        if((RXCnt > 63) || (RXData[RXCnt-1] == 0x0D)){
            memset(RXData,0x00,63);
            RXCnt = 0;
            return 0;
        }
        U1STAbits.URXDA = 0;
        return 1;
    }
    return 0; 
     */
/*
    int c = 0;
    unsigned char *data = NULL;
    U1STAbits.URXDA = 0;
    while(U1STAbits.URXDA == 1 && data[c] != '0'){
        data[c] =  U1RXREG; 
        c++;    
        
    }
    c = 0;
    */
/*   
    int i=0,l = 0;
    char buf[64];
    itoa(buf,(int)RXData,10);
    l = strlen(buf);
    for(i = 0 ; i < l ; i++){
        return buf[i];
    }
    return '0';
     * */


//void Init_UART()
/*
    U1MODEbits.UARTEN = 0; // Bit15 TX, RX DISABLED, ENABLE at end of func

    U1MODEbits.USIDL = 0; // Bit13 Continue in Idle
    U1MODEbits.IREN = 0; // Bit12 No IR translation
    U1MODEbits.RTSMD = 0; // Bit11 Simplex Mode

    U1MODEbits.UEN = 0; // Bits8,9 TX,RX enabled, CTS,RTS not
    U1MODEbits.WAKE = 0; // Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0; // Bit6 No Loop Back
    U1MODEbits.ABAUD = 0; // Bit5 No Autobaud (would require sending '55')
    U1MODEbits.BRGH = 0; // Bit3 16 clocks per bit period
    U1MODEbits.PDSEL = 0; // Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0; // Bit0 One Stop Bit

    U1BRG = BRGVAL; // 60Mhz osc, 9600 Baud

    U1STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    U1STAbits.UTXINV = 0; //Bit14 N/A, IRDA config
    U1STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15

    U1STAbits.UTXBRK = 0; //Bit11 Disabled
    U1STAbits.UTXEN = 0; //Bit10 TX pins controlled by periph

    U1STAbits.URXISEL = 0; //Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0; //Bit5 Address Detect Disabled

    U1MODEbits.UARTEN = 1; // And turn the peripheral on
    U1STAbits.UTXEN = 1;
    */
/*
    unsigned int UartMode = 0;
    unsigned int UartCtrl = 0;
    
    UartMode =  UART_EN &       //Enable UART
                UART_IDLE_CON &     //Continue working when Idle
                UART_IrDA_DISABLE & //Not using IrDA
                UART_MODE_SIMPLEX & //Not using Flow control
                UART_UEN_00 &       //Not using CTS/RTS pins by not setting it up
                UART_DIS_WAKE &     //Disable wakeup option
                UART_DIS_LOOPBACK & //Not using in loopback mode
                UART_DIS_ABAUD &    //Disable ABAUD
                UART_NO_PAR_8BIT &  //8bit data with none for parity
                UART_BRGH_SIXTEEN & //Clock pulse per bit
                UART_UXRX_IDLE_ONE& // UART Idle state is 1
                UART_1STOPBIT;      // 1 stop bit
    
    UartCtrl =  UART_TX_ENABLE &     //Enable UART transmit
                UART_IrDA_POL_INV_ZERO &//sets the pin's idle state
                UART_SYNC_BREAK_DISABLED &
                UART_INT_RX_CHAR &      //Interrupt at every character received 
                UART_ADR_DETECT_DIS &  //Disabled since not using addressed mode
                UART_RX_OVERRUN_CLEAR;  //UART Overrun Bit Clear
    
ConfigIntUART1( UART_RX_INT_EN & //Enable RX Interrupt
                UART_RX_INT_PR2 &        //Set priority level
                UART_TX_INT_DIS &        //Disable TX Interrupt
                UART_TX_INT_PR0);        //Priority as none 
    
    IFS0bits.U1RXIF = 0;    
    
    OpenUART1(UartMode,UartCtrl, BRGVAL);
    */
