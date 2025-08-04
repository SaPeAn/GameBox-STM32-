 /******************************************************************************/
//      
//    LCD display
//    Controller - ST7565
//    
//    sofrware  hardware SPI
//    
/******************************************************************************/

#include "drv_LCD_ST7565_SPI.h"
#include "display_data.h"
#include <stdlib.h>

#define    BUF_EN   // BUF_EN - working through the buffer, BUF_DIS - direct 


#define   DISP_INIT_SEND_COM      HAL_GPIO_WritePin(DISP_RS_GPIO_Port, DISP_RS_Pin, RESET)   // 0-cmd
#define   DISP_INIT_SEND_DAT      HAL_GPIO_WritePin(DISP_RS_GPIO_Port, DISP_RS_Pin, SET)     // 1-data
#define   DISP_RESET_ON           HAL_GPIO_WritePin(DISP_RSE_GPIO_Port, DISP_RSE_Pin, RESET)
#define   DISP_RESET_OFF          HAL_GPIO_WritePin(DISP_RSE_GPIO_Port, DISP_RSE_Pin, SET)

extern SPI_HandleTypeDef hspi2;

#define  HW_SPI  // SPI type software(SW_SPI)/hardware(HW_SPI) 

uint8 dispbuffer[8][128] = {0};
uint8 bufpg = 0;
uint8 bufcl = 0;

/*----------------------------------------------------------------------------*/

/*------------------------LCD FUNCTIONS---------------------------------------*/
void LCD_init(void)
{
	DISP_RESET_OFF;
	DISP_INIT_SEND_DAT;
	HAL_Delay(2);
	DISP_RESET_ON;
	HAL_Delay(2);
	DISP_RESET_OFF;
	DISP_INIT_SEND_COM;
	uint8_t init_data_array[13] = {
	(0xA2 | 0),       // LCD Bias Set -- x=0 : 1/9 (default); x=1 :  1/7
	(0xA0 | 0),       // Segment Driver Direction Select 0->131(0);  Segment Driver Direction Select 131->0(1))
	(0xC0 | 8),       // Common Output Mode Select 0->63(0)); Common Output Mode Select 63->0(8)
	(0x20 | 0x6),     // V5 Voltage Regulator Internal Resistor Ratio Set 0:3.0; 1:3.5; 2:4; 3:4.5; 4:5.0(default); 5:5.5; 6:6; 7:6.4;
	(0x28 | 0b111),   // Power Controller Set a=1 :  Booster circuit on; b=1 :  Voltage regulator circuit on; c=1 : Voltage follower circuit on; default: 000, must be 111
	(0x81),           // The Electronic Volume Mode Set (contrast) (default 0010 0000) - first byte (command id)
	(0x20),           // The Electronic Volume Mode Set (contrast) (default 0010 0000) - second byte (value)
	(0x40 | 0),       // Display Start Line Set (0-63)
	(0xB0 | 0),       // Page Address Set (0-7))
	(0x10 | 0),       // Column Address Set - first byte (0-15)
	(0x00 | 0),       // Column Address Set - second byte (0-15)
	(0xAE | 1),       // Display on(1) / Display off (0)
	(0xA6 | 0),       // Display Normal(0) / Display Reverse(1)
	};
	HAL_SPI_Transmit(&hspi2, init_data_array, 13, 20);
	LCD_bufupload_buferase();
	DISP_INIT_SEND_DAT;
}

void lcd_sendcommands(uint8_t* data, uint8_t N)
{
  DISP_INIT_SEND_COM;
  HAL_SPI_Transmit(&hspi2, data, N, 10);
  DISP_INIT_SEND_DAT;
}

void LCD_setpagecolumn(uint8 pg, uint8 cl) {
  bufpg = pg;
  bufcl = cl;
}

void LCD_printsmb8x5(const unsigned char ch, uint8 pg, uint8 cl) {
  LCD_setpagecolumn(pg, cl);
  LCD_senddata(char_8x5[ch], 5);
  LCD_setpagecolumn(pg, cl + 5);
  LCD_writebyte(0x00);
}

uint8 LCD_printstr8x5(const uint8 *str, uint8 pg, uint8 cl) {
  uint8 str_null[] = "NULL";
  if (str == NULL) str = str_null;
  uint8 i = 0;
  while (str[i]) {
    if (cl > 123 && str[i] != '\n') {
      pg++;
      cl = 0;
    }
    if(str[i] == '\n') {cl = 0; pg++; i++; continue;}
    LCD_printsmb8x5(str[i], pg, cl);
    cl += 6;
    if (pg > 7) return 0;
    i++;
  }
  return i;
}

void LCD_erasestring(uint8 length, uint8 pg, uint8 cl) {
  LCD_setpagecolumn(pg, cl);
  for (uint8 i = 0; i < length; i++) LCD_writebyte(0x00);
}

void LCD_bufupload_buferase(void) {
  LCDbuf_upload();
  LCDbuf_erase();
}

void LCD_senddata(const uint8* byte, uint8 N) {
  for (uint8 i = 0; i < N; i++) {
    dispbuffer[bufpg][bufcl + i] |= byte[i];
    if (bufcl > 127) bufpg++;
    if (bufpg > 7) return;
  }
}
/*----------------------------------------------------------------------------*/

/*------------------------WORKING THROUGH THE BUFFER--------------------------*/
void LCDbuf_upload(void) {
	for(uint8_t j = 0; j < 8; j++)
	  {
		uint8_t tmp_arr[] = {(0xB0 + j), 0x10, 0x00};
	    lcd_sendcommands(tmp_arr, 3);
	    HAL_SPI_Transmit(&hspi2, dispbuffer[j], 128, 10);
	  }
}

void LCDbuf_erase(void) {
  for (uint8 j = 0; j < 8; j++)
    for (uint8 i = 0; i < 128; i++) dispbuffer[j][i] = 0;
}
/*----------------------------------------------------------------------------*/

/*-----------------------------SYSTEM MENU ELEMENTS---------------------------*/

void LCD_printweekday(uint8 wdaynum, uint8 pg, uint8 cl) {
  LCD_printstr8x5(weekday[wdaynum], pg, cl);
}

void LCD_printmonth(uint8 mon, uint8 pg, uint8 cl) {
  LCD_printstr8x5(month[mon - 1], pg, cl);
}

void LCD_printclockanddate(uint8 pg, uint8 cl) {
  uint8 wday[3] = {0};
  uint8 month[4] = {0};
  uint8 day[3] = {dig_to_smb((rtcbcd.day & 0x30) >> 4), dig_to_smb(rtcbcd.day & 0x0F), '\0'};
  uint8 hours[3] = {dig_to_smb(rtcraw.hour / 10), dig_to_smb(rtcraw.hour % 10), '\0'};
  uint8 colon[4] = {0x00, 0x12, 0x00}; // ':' colon
  uint8 minutes[3] = {dig_to_smb(rtcraw.min / 10), dig_to_smb(rtcraw.min % 10), '\0'};
  LCD_printstr8x5(day, pg, cl);
  LCD_printmonth(rtcraw.month, pg, cl + 14);
  LCD_printweekday(rtcraw.weekday, pg, cl + 35);
  LCD_printstr8x5(hours, pg, cl + 49);
  LCD_setpagecolumn(pg, cl + 61);
  LCD_senddata(colon, 3);
  LCD_printstr8x5(minutes, pg, cl + 64);
}

void LCD_printbrightnes(uint8 pg, uint8 cl) //  size 26 column
{
  LCD_setpagecolumn(pg, cl);
  LCD_senddata(bright_icon, 8);
  LCD_setpagecolumn(pg, cl + 9);
  LCD_senddata(bright_lvl[brightlvl], 15);
}

void LCD_printbatlevel(uint8 lvl, uint8 pg, uint8 cl) {
  if (lvl == 100) lvl = 5; // 100 -shutdown code
  LCD_setpagecolumn(pg, cl);
  LCD_senddata(battary_2[lvl], 20);
}

void LCD_printmenucoursor(uint8 pg, uint8 cl) {
  LCD_setpagecolumn(pg, cl);
  LCD_senddata(menu_pointer, 8);
}

void LCD_printbutselhint(uint8 hintnum, uint8 pg, uint8 cl) {
  for (uint8 j = 0; j < 4; j++) {
    LCD_setpagecolumn(pg + j, cl);
    for (uint8 i = 0; i < 32; i++) LCD_writebyte(but_sel_hint[hintnum][(i * 4 + j)]);
  }
}

void LCD_printvertline(uint8 linelength, uint8 startline, uint8 cl) {
  if ((linelength - startline) > 63) return;
  uint8 startpg = startline / 8;
  uint8 bitshifting = (startline % 8);
  uint8 lengthinpages = (linelength - (8 - bitshifting)) / 8;
  uint8 temp = (uint8) (0xFF << bitshifting);
  LCD_setpagecolumn(startpg, cl);
  LCD_senddata(&temp, 1);
  temp = 0xFF;
  for (uint8 i = 0; i < lengthinpages; i++) {
    LCD_setpagecolumn(startpg + 1 + i, cl);
    LCD_senddata(&temp, 1);
  }
  temp = 0xFF >> (8 - (linelength - (8 - bitshifting)) % 8);
  LCD_setpagecolumn(startpg + 1 + lengthinpages, cl);
  LCD_senddata(&temp, 1);
}

void LCD_printhorline(uint8 linelength, uint8 startstring, uint8 cl) {
  if ((linelength - cl) > 127) return;
  uint8 startpg = startstring / 8;
  uint8 bitshifting = (startstring % 8);
  uint8 temp = (uint8) (0x1 << bitshifting);
  LCD_setpagecolumn(startpg, cl);
  for (uint8 i = 0; i < linelength; i++) {
    LCD_setpagecolumn(startpg, cl + i);
    LCD_senddata(&temp, 1);
  }
}
/*----------------------------------------------------------------------------*/

/*------------------------------GAME OBJECTS----------------------------------*/
void LCD_printsprite(int8 startline, int8 startcolumn, const tSprite * const Sprite) {
  int16 columns_max = 0;
  uint16 m = 0, mprev = 0;
  uint8 column_shift = 0;
  uint8 line_shift = (uint8)startline % 8;
  int8 pages_max = (int8)Sprite->pages;
  uint8 page_shift = 0;
  bufpg = (uint8)startline / 8;
  if (startcolumn < 0) {
    if(startline >= 0) {
      bufcl = 0;
      columns_max = (int16)Sprite->columns + (int16)startcolumn;
      if (columns_max < 0) columns_max = 0;
      m = (uint16)(-startcolumn) * Sprite->pages;
      column_shift = (uint8)(-startcolumn);
    }
    else{
      bufcl = 0;
      columns_max = (int16)Sprite->columns + (int16)startcolumn;
      if (columns_max < 0) columns_max = 0;
      //m = (uint16)(-startcolumn) * Sprite->pages + ;
      column_shift = (uint8)(-startcolumn);
      
      line_shift = 8 - (uint8)-startline % 8;
      bufpg = 0;
      pages_max = (int8)Sprite->pages + startline/8 - 1;
      page_shift = (uint8)-startline/8 + 1;
      
      m = (uint16)(-startcolumn) * Sprite->pages + page_shift;
    }
  }
  else {
    if(startline >= 0){
      bufcl = (uint8)startcolumn;
      columns_max = (((int16)startcolumn + (int16)Sprite->columns) > 127) ? (127 - startcolumn) : Sprite->columns;
    }
    else{
      bufcl = (uint8)startcolumn;
      columns_max = (((int16)startcolumn + (int16)Sprite->columns) > 127) ? (127 - startcolumn) : Sprite->columns;
      
      line_shift = 8 - (uint8)-startline % 8;
      bufpg = 0;
      pages_max = (int8)Sprite->pages + startline/8 - 1;
      page_shift = (uint8)-startline/8 + 1;
      m = page_shift;
    }
  }
  
  switch (Sprite->direct) {
    case COLUMNS_FIRST:
      for (uint8 j = 0; j < columns_max; j++) {
        for (uint8 i = 0; i < (line_shift ? (pages_max + 1) : pages_max); i++) {
          if (i == 0) {
            if(startline >= 0){ 
              if((bufpg + i) < 8 && (bufcl + j) <= 127)  dispbuffer[bufpg + i][bufcl + j] |= Sprite->sprite[m] << line_shift;
              mprev = m;
              m++;
            }
            if(startline < 0){
              mprev = m - 1;
              if((bufpg + i) < 8 && (bufcl + j) <= 127)  dispbuffer[bufpg + i][bufcl + j] |= (Sprite->sprite[mprev + page_shift * j] >> (8 - line_shift))|
                                                                                             (Sprite->sprite[m + page_shift * j] << line_shift);
              mprev = m;
              m++;
            }
          }
          if ((i > 0) && (i < pages_max)) {
            if((bufpg + i) < 8 && (bufcl + j) <= 127) dispbuffer[bufpg + i][bufcl + j] |= (Sprite->sprite[mprev + page_shift * j] >> (8 - line_shift))| 
                                                                                          (Sprite->sprite[m + page_shift * j] << line_shift);
            mprev = m;
            m++;
          }
          if (i == pages_max) {
            
            if((bufpg + i) < 8 && (bufcl + j) <= 127) dispbuffer[bufpg + i][bufcl + j] |= Sprite->sprite[mprev + page_shift * j] >> (8 - line_shift);
          }
        }
      }
      break;
    case LINES_FIRST:
      for (uint8 i = 0; i < (line_shift ? (pages_max + 1) : pages_max); i++) {
        for (uint8 j = 0; j < columns_max; j++) {
          if (i == 0) {
            dispbuffer[bufpg + i][bufcl + j] |= Sprite->sprite[j + column_shift] << line_shift;
          }
          if ((i > 0) && (i < pages_max)) {
            dispbuffer[bufpg + i][bufcl + j] |= (Sprite->sprite[(i - 1) * Sprite->columns + j + column_shift] >> (8 - line_shift)) | 
                                                (Sprite->sprite[i * Sprite->columns + j + column_shift] << line_shift);
          }
          if (i == pages_max) {
            dispbuffer[bufpg + i][bufcl + j] |= Sprite->sprite[(i - 1) * Sprite->columns + j + column_shift] >> (8 - line_shift);
          }
        }
      }
      break;
  }
}

void LCD_printgamestatbar(tGamer* gamer) {
  for (uint8 i = 0; i < 128; i++) dispbuffer[0][i] |= GameStatusBar[i];
  for (uint8 i = 9; i < (9 + gamer->health); i++) dispbuffer[0][i] |= 0b00111100; // helth bar
  for (uint8 i = 102; i < (102 + gamer->energy); i++) dispbuffer[0][i] |= 0b00111100; // energy bar
  dispbuffer[0][102 + gamer->energymax] |= 0b01111110;
  uint8 money[5];
  uint16 displaymoney = gamer->money;
  if(displaymoney > 999) displaymoney = 999;
  u16_to_str(money, displaymoney, 10);
  uint8 bombs[2];
  u16_to_str(bombs, gamer->bombs, 10);
  LCD_printstr8x5(money, 0, 50);
  LCD_printstr8x5(bombs, 0, 81);
}
/*----------------------------------------------------------------------------*/

