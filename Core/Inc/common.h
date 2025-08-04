#ifndef COMMON_H
#define	COMMON_H

#include "main.h"

#define     ENABLE     1
#define     DISABLE    0    
    
typedef unsigned char    uint8;
typedef signed char      int8; 
typedef unsigned int     uint16;
typedef signed int       int16;
typedef unsigned long    uint32;
typedef signed long      int32;  

/*-------------------------SYSTEM VARIABLES-----------------------------------*/
typedef union{
  struct{
    uint8 sec;     // BCD  0...59            |x |SH|SH|SH|SL|SL|SL|SL|
    uint8 min;     // BCD  0...59            |x |MH|MH|MH|ML|ML|ML|ML|
    uint8 hour;    // BCD  0...23            |x |x |HH|HH|HL|HL|HL|HL|
    uint8 day;     // BCD  0...              |x |x |DH|DH|DL|DL|DL|DL|
    uint8 weekday; // BCD  from SUN to SAT   |x |x |x |x |x |W |W |W |
    uint8 month;   // century bit & BCD      |C |x |x |MH|ML|ML|ML|ML|
    uint8 year;    // BCD                    |YH|YH|YH|YH|YL|YL|YL|YL|
  };
  uint8 rtcdata[7];
}tRTC;

typedef struct{
    unsigned MenuFl             :1;
    unsigned RunGameFl          :1;
    unsigned MenuSettingsFl     :1;  
    unsigned TestScreen         :1;  
    unsigned MMPartbit_2        :1;  
    unsigned coursorpos         :3;
} tFlags;                               // System functions launch flags

tFlags CFlags = {1, 0, 0, 0, 0, 1};

uint32 timestamp = 0;    // System timer (ms), starts counting from power on or last restart
tRTC rtcbcd;          // structure for clock/date from RTC module (BCD format))
tRTC rtcraw;          // structure for system clock/date (uint8)
uint8 Ubat;              // ADC data from battery level measurement
uint8 batlvl;            // battery level for display (0...5)
uint8 brightlvl;         // brightness level for display (0...7)
uint8 brightPWM = 220;   // PWM duty cycle value for regulate display brightness
/*----------------------------------------------------------------------------*/

typedef enum {
LINES_FIRST,
COLUMNS_FIRST
}tDirect;

typedef struct{
uint8 pages;
uint8 columns;
const uint8* sprite;
tDirect direct;
}tSprite;


/*-------------------------SYSTEM FUNCTIONS-----------------------------------*/
void commoninit(void);
void gettime(void);
uint8 getbatlvl(uint8);
uint8 getrand(uint8);
void randinit(void);
uint8 dig_to_smb(uint8);
void u16_to_str(uint8*, uint16, uint8);
void BrightPWMgen(uint8);
void decbright(void);
void incbright(void);
void getbrightlvl(void);
void Sounds(uint16);
void ShutDown(void);
void ShutDownLB(void);
void batcheck(void);
void rtcbcdtoraw(void);
void rtcrawtobcd(void);
void RTCgetdata(uint8*);
void RTCsenddata(uint8*);


#define    PWM_MEMADR    0
uint8 EEPROM_writebyte(uint8, uint8);
uint8 EEPROM_readbyte(uint8);
/*----------------------------------------------------------------------------*/

/*---------------------BUTTONS & JOYSTICK VARIABLES---------------------------*/
struct buttonstruct{
	GPIO_TypeDef*    Port;
  uint16           Pin;
  const uint32*    timecounter;
  uint32           btnTimer;
  uint8            BtnFl;
  uint8            BtnON;
  uint8            Toggle;
  uint8            HoldON;
  uint8            StuckON;
};

typedef struct buttonstruct tButton;
//buttons

typedef struct{
  uint8 up      :1;
  uint8 down    :1;
  uint8 left    :1;
  uint8 right   :1;
  uint8 joyFl   :1;
  uint8 ox;          //joystick x-axis position(0...255, center - ~130)
  uint8 oy;          //joystick y-axis position(0...255, center - ~130)
}tJoystick;

tButton B1;
tButton B2;
tButton B3;
tButton B4;
tJoystick joystick = {0};
/*----------------------------------------------------------------------------*/

/*---------------------BUTTONS & JOYSTICK FUNCTIONS---------------------------*/
tButton CreateBtn(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, const uint32* timecounter);
void TestBtn(tButton*);
void initbuttons(void);
uint8 adc_getval_an0(void);
uint8 adc_getval_an1(void);
uint8 adc_getval_an2(void);
void getjoypos(void);
void checkjoydir(void);
void check_btn_jstk(void);
/*----------------------------------------------------------------------------*/

#endif	/* COMMON_H */

