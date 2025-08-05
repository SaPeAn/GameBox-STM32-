
#include "common.h"
#include "drv_LCD_ST7565_SPI.h"
#include <stdlib.h>

/*----------------------------------GLOBVARS--------------------------------*/
tFlags CFlags = {1, 0, 0, 0, 0, 0};
tButton B1;
tButton B2;
tButton B3;
tButton B4;
tJoystick joystick = {0};

uint32 timestamp = 0;    // System timer (ms), starts counting from power on or last restart
tRTC rtcbcd;          // structure for clock/date from RTC module (BCD format))
tRTC rtcraw;          // structure for system clock/date (uint8)
uint8 Ubat;              // ADC data from battery level measurement
uint8 batlvl;            // battery level for display (0...5)
uint8 brightlvl;         // brightness level for display (0...7)
uint8 brightPWM = 220;   // PWM duty cycle value for regulate display brightness

/*----------------------------------UTILITIES-------------------------------*/
uint8 clamp(uint8 val, uint8 min, uint8 max)
{
  if(val > max) val = max;
  if(val < min) val = min;
  return val;
}

void randinit(void)
{
  srand((uint8)timestamp);
}

uint8 getrand(uint8 N)
{
  return (rand() % (N + 1));
}

uint8 dig_to_smb(uint8 dig)
{
  switch (dig)
  {
    case 0: return '0';
    case 1: return '1'; 
    case 2: return '2';
    case 3: return '3'; 
    case 4: return '4';
    case 5: return '5'; 
    case 6: return '6'; 
    case 7: return '7'; 
    case 8: return '8';
    case 9: return '9';
  }
  return 0;
}

void u16_to_str(uint8* str, uint16 num, uint8 N)
{ 
  //sprintf((char*)str, "%u", num);
  
  str[0] = dig_to_smb((uint8)(num/10000));
  num %= 10000;
  str[1] = dig_to_smb((uint8)(num/1000));
  num %= 1000;
  str[2] = dig_to_smb((uint8)(num/100));
  num %= 100;
  str[3] = dig_to_smb((uint8)(num/10));
  str[4] = dig_to_smb((uint8)(num%10));
  str[5] = '\0';
  
  if(N == 10)
  {
    uint8 chars = 0;
    for(int i = 0; i < 6; i++)
    {
      if((str[i] == '0') && !chars) continue;
      str[chars] = str[i];
      chars++;
    }
    if(chars == 1) 
    {
      str[0] = '0';
      str[1] = '\0';
    }
  }
      
  if((N < 5) && (N > 0))
  {
    for(uint8 i = 0; i <= N; i++) 
      str[i] = str[5 - N + i];
  }
}
/*----------------------------------------------------------------------------*/

/*-------------------------------INITIALIZATION-------------------------------*/

void commoninit(void)
{
  HAL_GPIO_WritePin(SHUTDOWN_GPIO_Port, SHUTDOWN_Pin, SET);
  HAL_GPIO_WritePin(SOUND_OUT_GPIO_Port, SOUND_OUT_Pin, RESET);
  BrightPWMgen(220);
  //brightPWM = EEPROM_readbyte(PWM_MEMADR);
}

void initbuttons(void)
{
  B1 = CreateBtn(BTN_1_GPIO_Port, BTN_1_Pin, &timestamp);
  B2 = CreateBtn(BTN_2_GPIO_Port, BTN_2_Pin, &timestamp);
  B3 = CreateBtn(BTN_3_GPIO_Port, BTN_3_Pin, &timestamp);
  B4 = CreateBtn(BTN_4_GPIO_Port, BTN_4_Pin, &timestamp);
}
/*----------------------------------------------------------------------------*/

/*------------------------------SYSTEM FUNCTIONS------------------------------*/
uint8 getbatlvl(uint8 Ub)
{
  uint8 lvl = (240 - Ub) / 14;
  static uint8 Umax = 241;
  static uint8 Umin = 220;
  static uint8 reslvl = 0;
  if(Ub < 157) return 100; // bat to low, immediately shotdown code - 100
  Ub = clamp(Ub, 157, 241);
  if((Ub <= Umax) && (Ub >= Umin)) return reslvl;
  else 
  {
    switch(lvl)
    {
      case 0: Umin = 220; Umax = 241; reslvl = 0; break;
      case 1: Umin = 206; Umax = 227; reslvl = 1; break;
      case 2: Umin = 192; Umax = 213; reslvl = 2; break;
      case 3: Umin = 178; Umax = 199; reslvl = 3; break;
      case 4: Umin = 164; Umax = 185; reslvl = 4; break;
      case 5: Umin = 157; Umax = 171; reslvl = 5; break;
    }
  }
  return reslvl;
}

void batcheck(void)
{
  batlvl = getbatlvl(Ubat);
  if(batlvl == 100) ShutDownLB();
}
        
void ShutDownLB(void)
{
  LCD_bufupload_buferase();
  HAL_GPIO_WritePin(SHUTDOWN_GPIO_Port, SHUTDOWN_Pin, RESET);
  LCD_printstr8x5((uint8*)"Низкий заряд батареи!", 1, 0);
  LCD_printstr8x5((uint8*)"Устройство", 3, 0);
  LCD_printstr8x5((uint8*)"сейчас выключится!", 5, 0);
  LCD_bufupload_buferase();
  while(1);
}

void ShutDown(void)
{
  LCD_bufupload_buferase();
  HAL_GPIO_WritePin(SHUTDOWN_GPIO_Port, SHUTDOWN_Pin, RESET);
  LCD_printstr8x5((uint8*)"Выключение...", 3, 0);
  LCD_bufupload_buferase();
  while(1);
}

void getbrightlvl(void)
{
  brightlvl = ((brightPWM - 10) / 30) - 1;
}

void decbright(void)
{
  if(brightPWM <= 40) brightPWM = 70; 
  brightPWM -=30;
  //EEPROM_writebyte(PWM_MEMADR, brightPWM);
}

void incbright(void)
{
  if(brightPWM >= 250) brightPWM = 220; 
  brightPWM +=30;
  //EEPROM_writebyte(PWM_MEMADR, brightPWM);
}

extern TIM_HandleTypeDef htim1;
void BrightPWMgen(uint8 duty_cycle)
{
	uint16 dutyCycle = duty_cycle * 255;
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dutyCycle);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void Sounds(uint16 delay)
{
  uint32 j;
  for(uint16 i = 0; i < (15000/delay); i++)
  {  
  	HAL_GPIO_WritePin(SOUND_OUT_GPIO_Port, SOUND_OUT_Pin, SET);
    j = delay * 6;
    while(j--);
    HAL_GPIO_WritePin(SOUND_OUT_GPIO_Port, SOUND_OUT_Pin, RESET);
    j = delay * 6;
    while(j--);
  }
}

void RTCgetdata(uint8* RTCarr)
{

}

void RTCsenddata(uint8* RTCarr)
{

}

void rtcrawtobcd(void)
{
  rtcbcd.year = (uint8)((rtcraw.year / 10) << 4) | (rtcraw.year % 10);
  rtcbcd.month = (0x80 | (uint8)((rtcraw.month / 10) << 4)) | rtcraw.month % 10;
  rtcbcd.day = (uint8)((rtcraw.day / 10) << 4) | (rtcraw.day % 10);
  rtcbcd.weekday = rtcraw.weekday;
  rtcbcd.hour = (uint8)((rtcraw.hour / 10) << 4) | (rtcraw.hour % 10);
  rtcbcd.min = (uint8)((rtcraw.min / 10) << 4) | (rtcraw.min % 10);
  rtcbcd.sec = (uint8)((rtcraw.sec / 10) << 4) | (rtcraw.sec % 10);
}

void rtcbcdtoraw(void)
{
  rtcraw.year = (rtcbcd.year >> 4) * 10 + (rtcbcd.year & 0x0F);
  rtcraw.month = ((rtcbcd.month & 0x1F) >> 4) * 10 + (rtcbcd.month & 0x0F);
  rtcraw.day = ((rtcbcd.day & 0x3F) >> 4) * 10 + (rtcbcd.day & 0x0F);
  rtcraw.weekday = rtcbcd.weekday & 0x07;
  rtcraw.hour = ((rtcbcd.hour & 0x3F) >> 4) * 10 + (rtcbcd.hour & 0x0F);
  rtcraw.min = ((rtcbcd.min & 0x7F) >> 4) * 10 + (rtcbcd.min & 0x0F);
  rtcraw.sec = ((rtcbcd.sec & 0x7F) >> 4) * 10 + (rtcbcd.sec & 0x0F);
}
/*----------------------------------------------------------------------------*/

/*-----------------------------BUTTONS FUNCTIONS------------------------------*/
#define  BTN_HOLD_ON_DELAY    75
#define  BTN_STUCK_ON_DELAY   2000

tButton CreateBtn(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, const uint32* timecounter)
{
  tButton BTN;
  BTN.Port = GPIOx;
  BTN.Pin = GPIO_Pin;
  BTN.timecounter = timecounter;
  BTN.BtnFl = 0;
  BTN.BtnON = 0;
  BTN.Toggle = 0;
  BTN.HoldON = 0;
  BTN.StuckON = 0;
  BTN.btnTimer = 0;
  return BTN;
}

void check_btn_jstk(void) //Test buttons and joystick
{
  TestBtn(&B1);
  TestBtn(&B2);
  TestBtn(&B3);
  TestBtn(&B4);
  checkjoydir();
}

void TestBtn(tButton* btn)
{
  if (!HAL_GPIO_ReadPin(btn->Port, btn->Pin) && !btn->BtnFl && ((*(btn->timecounter) - btn->btnTimer) > 30)) {
    if(btn->Toggle) btn->Toggle = 0; 
    else btn->Toggle = 1;
    btn->BtnFl = 1;
    btn->btnTimer = *(btn->timecounter); 
    btn->BtnON = 1;
  }
  if (HAL_GPIO_ReadPin(btn->Port, btn->Pin) && ((*(btn->timecounter) - btn->btnTimer) > 30)) {
    btn->BtnFl = 0;
    btn->HoldON = 0;
    btn->StuckON = 0;
    btn->btnTimer = *(btn->timecounter);
  }
  if (!HAL_GPIO_ReadPin(btn->Port, btn->Pin) && btn->BtnFl && ((*(btn->timecounter) - btn->btnTimer) > BTN_HOLD_ON_DELAY) && ((*(btn->timecounter) - btn->btnTimer) <= BTN_STUCK_ON_DELAY)) {
    btn->HoldON = 1;
    if(btn->Toggle) btn->Toggle = 0; 
    else btn->Toggle = 1;
  }
  if (!HAL_GPIO_ReadPin(btn->Port, btn->Pin) && btn->BtnFl && ((*(btn->timecounter) - btn->btnTimer) > BTN_STUCK_ON_DELAY)) {
    btn->HoldON = 0;
    btn->StuckON = 1;
    if(btn->Toggle) btn->Toggle = 0; 
    else btn->Toggle = 1;
  }
}

void checkjoydir(void)
{
    if(joystick.oy > 150 && joystick.joyFl == 0) {
        joystick.up = 1; 
        joystick.joyFl = 1;
    }
    if(joystick.oy < 100 && joystick.joyFl == 0) {
        joystick.down = 1; 
        joystick.joyFl = 1;
    }
    if(joystick.ox > 150 && joystick.joyFl == 0) {
        joystick.right = 1; 
        joystick.joyFl = 1;
    }
    if(joystick.ox < 100 && joystick.joyFl == 0) {
        joystick.left = 1; 
        joystick.joyFl = 1;
    }
    if(joystick.oy < 150 && joystick.oy > 100 && joystick.ox < 150 && joystick.ox > 100) joystick.joyFl = 0;
}
/*----------------------------------------------------------------------------*/

/*---------------------------SAVE/LOAD FUNCTIONS------------------------------*/
/*
uint8 EEPROM_writebyte(uint8 adr, uint8 byte)
{
  EEADR = adr;
  EEDATA = byte;
  EECON1bits.EEPGD = 0;
  EECON1bits.CFGS = 0;
  EECON1bits.WREN = 1;
  INTCONbits.GIE = 0;
  EECON2 = 0x55;
  EECON2 = 0xAA;
  EECON1bits.WR = 1;
  INTCONbits.GIE = 1;
  EECON1bits.WREN = 0;
  return EECON1bits.WRERR;
}

uint8 EEPROM_readbyte(uint8 adr)
{
  EEADR = adr;
  EECON1bits.EEPGD = 0;
  EECON1bits.CFGS = 0;
  EECON1bits.RD = 1;
  return EEDATA;
}
*/
