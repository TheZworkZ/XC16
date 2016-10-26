#include "p33Exxxx.h"
#include "define.h"

#include "libpic30.h"
#include "smps_control.h"
#include "load_config.h"

#include "pps.h"

#ifndef INIT_H
#define INIT_H

void InitClock(void);
void InitI2C(void);
void InitBuckPWM(void);
void InitBoostPWM(void);
void InitADC(void);
void InitCMP(void);
void InitBuckComp(void);
void InitBoostComp(void);
void InitTimer(void);
void InitCalibrateADC(void);
void InitPWM3(void);
void InitADCMP(void);

void SetupIOPins();
void Init_UART(void);
void InitPWM1(void);
void InitPWM2(void);

// Added for Alt_W register context switching
void InitAltRegContext1Setup(void);
void InitAltRegContext2Setup(void);

//Declare variable used in init.c
volatile extern uint8_t messageComplete, CALL_OR_WRITE;
extern char *TXData;
extern unsigned int RXData[64]; //To store Received data
extern unsigned int RXData2[16]; //To store Received data
volatile extern unsigned int RXCnt;


#endif	/* INIT_H */
