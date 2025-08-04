
#include "drv_LCD_ST7565_SPI.h"
#include "common.h"
#include "ufo_battle.h"
#include "math.h"


void setdatetime(void)
{
  uint8 selectpos = 0;
  RTCgetdata(rtcbcd.rtcdata);
  rtcbcdtoraw();
  uint8 displaingSaveFlag = 0;
  uint8 displaingSaveLatch = 0;
  uint32 tim = 0;
  while(1)
  {  
    checkjoydir();
    TestBtn(&B1); TestBtn(&B2); TestBtn(&B3); TestBtn(&B4);
    
    if(B2.BtnON){
      B2.BtnON = 0;
      RTCsenddata(rtcbcd.rtcdata);
      displaingSaveFlag = 1;
    }
    
    if(B1.BtnON){
      B1.BtnON = 0;
      return;
    }
    
    if(joystick.right == 1)
    {
      joystick.right = 0;
      selectpos++;
      if(selectpos == 7) selectpos = 0;
    }
    if(joystick.left == 1)
    {
      joystick.left = 0;
      selectpos--;
      if(selectpos == 255) selectpos = 6;
    }
        
    uint8 day[4] = {dig_to_smb((rtcbcd.day & 0x30) >> 4), dig_to_smb(rtcbcd.day & 0x0F), '.', '\0'};
    //uint8 temp[6];    
    //u16_to_str(temp, ((((rtcbcd.month & 0x10) >> 4) * 10) + (rtcbcd.month & 0x0F)), DISABLE);
    //uint8 month[4] = {temp[3], temp[4], '.', '\0'}; 
    uint8 month[4] = {dig_to_smb(((rtcbcd.month & 0x10) >> 4) * 10), dig_to_smb(rtcbcd.month & 0x0F), '.', '\0'};
    uint8 year[5] = {'2', '0', dig_to_smb((rtcbcd.year & 0xF0) >> 4), dig_to_smb(rtcbcd.year & 0x0F), '\0'};
    LCD_printstr8x5(day, 0, 2);
    LCD_printstr8x5(month, 0, 18);
    LCD_printstr8x5(year, 0, 34);
    uint8 hour[4] = {dig_to_smb((rtcbcd.hour & 0x30) >> 4), dig_to_smb(rtcbcd.hour & 0x0F), ':', '\0'};
    uint8 min[4] = {dig_to_smb((rtcbcd.min & 0x70) >> 4), dig_to_smb(rtcbcd.min & 0x0F), ':', '\0'};
    uint8 sec[3] = {dig_to_smb((rtcbcd.sec & 0x70) >> 4), dig_to_smb(rtcbcd.sec & 0x0F), '\0'};
    LCD_printstr8x5(hour, 0, 62);
    LCD_printstr8x5(min, 0, 80);
    LCD_printstr8x5(sec, 0, 98);
    LCD_printweekday(rtcraw.weekday, 0, 115);
    
    switch(selectpos)
    {
      case 0: 
        LCD_erasestring(128, 1, 0); 
        LCD_printhorline(12, 8, 2); 
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.day--; if(rtcraw.day > 31) rtcraw.day = 31;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.day++; if(rtcraw.day > 31) rtcraw.day = 0;}
      break;
      case 1: 
        LCD_erasestring(128, 1, 0); 
        LCD_printhorline(12, 8, 18);
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.month--; if(rtcraw.month > 12) rtcraw.month = 12;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.month++; if(rtcraw.month > 12) rtcraw.month = 0;}
      break;
      case 2: 
        LCD_erasestring(128, 1, 0); 
        LCD_printhorline(24, 8, 34); 
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.year--; if(rtcraw.year > 99) rtcraw.year = 99;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.year++; if(rtcraw.year > 99) rtcraw.year = 0;}
      break;
      case 3: 
        LCD_erasestring(128, 1, 0); 
        LCD_printhorline(12, 8, 62);
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.hour--; if(rtcraw.hour > 23) rtcraw.hour = 23;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.hour++; if(rtcraw.hour > 23) rtcraw.hour = 0;}
      break;
      case 4: 
        LCD_erasestring(128, 1, 0); 
        LCD_printhorline(12, 8, 80); 
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.min--; if(rtcraw.min > 59) rtcraw.min = 59;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.min++; if(rtcraw.min > 59) rtcraw.min = 0;}
      break;
      case 5: 
        LCD_erasestring(128, 1, 0);
        LCD_printhorline(12, 8, 98);
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.sec--; if(rtcraw.sec > 59) rtcraw.sec = 59;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.sec++; if(rtcraw.sec > 59) rtcraw.sec = 0;}
      break;
      case 6: 
        LCD_erasestring(128, 1, 0);
        LCD_printhorline(12, 8, 115);
        if(B3.BtnON || B3.HoldON || B3.StuckON){B3.BtnON = 0; rtcraw.weekday--; if(rtcraw.weekday > 6) rtcraw.weekday = 6;}
        if(B4.BtnON || B4.HoldON || B4.StuckON){B4.BtnON = 0; rtcraw.weekday++; if(rtcraw.weekday > 6) rtcraw.weekday = 0;}
      break;
    }
    
    LCD_printbutselhint(7, 3, 40);
    LCD_printbutselhint(6, 3, 89);
    LCD_printstr8x5((uint8*)"‚˚ı", 7, 85);
    LCD_printstr8x5((uint8*)"ÓÍ", 7, 110);
    LCD_printstr8x5((uint8*)"-  +", 2, 93); 
    
    if(displaingSaveFlag) {
      tim = timestamp;
      displaingSaveFlag = 0;
      displaingSaveLatch = 1;
    }
    if(displaingSaveLatch) LCD_printstr8x5((uint8*)"—Œ’–¿Õ≈ÕŒ", 7, 2);
    if(((timestamp - tim) > 2000) && displaingSaveLatch) {
      LCD_erasestring(128, 7, 0);
      displaingSaveLatch = 0;
    }
    
    rtcrawtobcd();
    LCD_bufupload_buferase();
  }
}

void testscreen(void)
{
  while(1)
  {
    RTCgetdata(rtcbcd.rtcdata);
    rtcbcdtoraw();    
    uint8 day[4] = {dig_to_smb((rtcbcd.day & 0x30) >> 4), dig_to_smb(rtcbcd.day & 0x0F), '.', '\0'};
    uint8 temp[6];    
    //u16_to_str(temp, ((((rtcbcd.month & 0x10) >> 4) * 10) + (rtcbcd.month & 0x0F)), DISABLE);
    //uint8 month[4] = {temp[3], temp[4], '.', '\0'};
    uint8 month[4] = {dig_to_smb(((rtcbcd.month & 0x10) >> 4) * 10), dig_to_smb(rtcbcd.month & 0x0F), '.', '\0'};
    uint8 year[5] = {'2', '0', dig_to_smb((rtcbcd.year & 0xF0) >> 4), dig_to_smb(rtcbcd.year & 0x0F), '\0'};
    LCD_printstr8x5(day, 0, 2);
    LCD_printstr8x5(month, 0, 18);
    LCD_printstr8x5(year, 0, 34);
    uint8 hour[4] = {dig_to_smb((rtcbcd.hour & 0x30) >> 4), dig_to_smb(rtcbcd.hour & 0x0F), ':', '\0'};
    uint8 min[4] = {dig_to_smb((rtcbcd.min & 0x70) >> 4), dig_to_smb(rtcbcd.min & 0x0F), ':', '\0'};
    uint8 sec[3] = {dig_to_smb((rtcbcd.sec & 0x70) >> 4), dig_to_smb(rtcbcd.sec & 0x0F), '\0'};
    LCD_printstr8x5(hour, 0, 62);
    LCD_printstr8x5(min, 0, 80);
    LCD_printstr8x5(sec, 0, 98);
    LCD_printweekday(rtcraw.weekday, 0, 115);
    
    batcheck();
    LCD_printstr8x5((uint8*)"¡‡Ú. = ", 2, 2);
    u16_to_str(temp, Ubat, DISABLE);
    LCD_printstr8x5(temp, 2, 44);
    LCD_printstr8x5((uint8*)"ﬂÍ. = ", 3, 2);
    u16_to_str(temp, brightPWM, DISABLE);
    LCD_printstr8x5(temp, 3, 44);
    LCD_printstr8x5((uint8*)"ƒÊ.X = ", 5, 2);
    u16_to_str(temp, joystick.ox, DISABLE);
    LCD_printstr8x5(temp, 5, 44);
    LCD_printstr8x5((uint8*)"ƒÊ.Y = ", 6, 2);
    u16_to_str(temp, joystick.oy, DISABLE);
    LCD_printstr8x5(temp, 6, 44);
  
    LCD_printbutselhint(1, 2, 89);
    LCD_printstr8x5((uint8*)"‚˚ıÓ‰", 6, 86);
    TestBtn(&B1);
    if(B1.BtnON) {B1.BtnON = 0; return;}
    LCD_bufupload_buferase();
  }
}
  
void MainMenu(void)
{
  //--------------------START MENU-------------------
  if(CFlags.MenuFl)
  {
    checkjoydir();
    TestBtn(&B2);
    if(B2.BtnON) 
    {
      B2.BtnON = 0;
      switch(CFlags.coursorpos) { 
        case 1: CFlags.MenuFl = 0; CFlags.RunGameFl = 1; CFlags.coursorpos = 1; break;
        case 2: CFlags.MenuFl = 0; CFlags.MenuSettingsFl = 1; CFlags.coursorpos = 1; break;
        case 3: ShutDown(); break;
      }
    }
    if(joystick.down == 1)
    {
      joystick.down = 0;
      CFlags.coursorpos++;
      if(CFlags.coursorpos == 4) CFlags.coursorpos = 1;
    }
    if(joystick.up == 1)
    {
      joystick.up = 0;
      CFlags.coursorpos--;
      if(CFlags.coursorpos == 0) CFlags.coursorpos = 3;
    }
    
    if(CFlags.coursorpos == 1) LCD_printmenucoursor(2, 4);
    if(CFlags.coursorpos == 2) LCD_printmenucoursor(4, 4);
    if(CFlags.coursorpos == 3) LCD_printmenucoursor(6, 4);
    
    LCD_printstr8x5((uint8*)"»√–¿", 2, 19);
    LCD_printstr8x5((uint8*)"Õ¿—“–Œ… ¿", 4, 19);
    LCD_printstr8x5((uint8*)"¬€ Àﬁ◊»“‹", 6, 19);
    
    LCD_printbutselhint(0, 2, 89);
    LCD_printstr8x5((uint8*)"‚ıÓ‰", 6, 93);
  }  
  //-------------------------------------------------
  
  //--------------------SETTINGS---------------------
  if(CFlags.MenuSettingsFl)
  {
    checkjoydir();
    TestBtn(&B1); TestBtn(&B2);
    if(B2.BtnON) 
    {
      B2.BtnON = 0;
      switch(CFlags.coursorpos) { 
        case 1: break;
        case 2: setdatetime(); break;
        case 3: testscreen(); break;
      }
    }
    if(B1.BtnON) 
    {
      B1.BtnON = 0;
      CFlags.MenuSettingsFl = 0;
      CFlags.MenuFl = 1;
      CFlags.coursorpos = 2;
    }
    if(joystick.down == 1)
    {
      joystick.down = 0;
      CFlags.coursorpos++;
      if(CFlags.coursorpos == 4) CFlags.coursorpos = 1;
    }
    if(joystick.up == 1)
    {
      joystick.up = 0;
      CFlags.coursorpos--;
      if(CFlags.coursorpos == 0) CFlags.coursorpos = 3;
    }
    
    if(CFlags.coursorpos == 1) {
      TestBtn(&B3); TestBtn(&B4);
      if(B3.BtnON || B3.HoldON || B3.StuckON) {B3.BtnON = 0; decbright();}
      if(B4.BtnON || B4.HoldON || B4.StuckON) {B4.BtnON = 0; incbright();}      
      LCD_printmenucoursor(2, 4);
      LCD_printbutselhint(4, 2, 89);
      LCD_printstr8x5((uint8*)"‚˚ı.", 6, 86);
      LCD_printstr8x5((uint8*)"-  +", 1, 93);
    }
    if(CFlags.coursorpos == 2) {
      LCD_printmenucoursor(4, 4);
      LCD_printbutselhint(5, 2, 89);
      LCD_printstr8x5((uint8*)"‚˚ı.‚ı.", 6, 86);
    }
    if(CFlags.coursorpos == 3) {
      LCD_printmenucoursor(6, 4);
      LCD_printbutselhint(5, 2, 89);
      LCD_printstr8x5((uint8*)"‚˚ı.‚ı.", 6, 86);
    }
    LCD_printstr8x5((uint8*)"ﬂ– Œ—“‹", 2, 19);
    LCD_printstr8x5((uint8*)"ƒ¿“¿/¬–≈Ãﬂ", 4, 19);
    LCD_printstr8x5((uint8*)"“≈—“", 6, 19);
  }
  //-------------------------------------------------
  
  batcheck();
  BrightPWMgen(brightPWM);
  getbrightlvl();
  RTCgetdata(rtcbcd.rtcdata);
  rtcbcdtoraw();
  
  LCD_printclockanddate(0, 26);
  LCD_printbatlevel(batlvl, 0, 105);
  LCD_printbrightnes(0, 0);
  LCD_printvertline(55, 8, 80);
  LCD_printvertline(55, 8, 0);
  LCD_printvertline(55, 8, 126);

  LCD_bufupload_buferase();
  HAL_Delay(50);
  
  //----------------------GAME-----------------------
  if(CFlags.RunGameFl)
  {
    ufobattle();
  }
  //-------------------------------------------------
  
}

