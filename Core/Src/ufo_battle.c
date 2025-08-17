#include "ufo_battle.h"
#include "drv_LCD_ST7565_SPI.h"
#include "display_data.h"
#include "scheduler.h"
#include "common.h"
#include <stdio.h>
#define  JOY_DIRECT_RESET       if (joystick.down || joystick.up || joystick.left ||joystick.right) {\
                                  joystick.down = 0; joystick.up = 0; joystick.left = 0; joystick.right = 0;}
#define  IF_ANY_BTN_PRESS(a)    if(B1.BtnON || B2.BtnON || B3.BtnON || B4.BtnON)\
                                {B1.BtnON = 0; B2.BtnON = 0; B3.BtnON = 0; B4.BtnON = 0; a}
#define  IF_BTN_B2_PRESS(a)     if(B2.BtnON || B2.HoldON || B2.StuckON)\
                                 {B2.BtnON = 0; a}
#define  IF_BTN_B1_PRESS(a)     if(B1.BtnON || B1.HoldON || B1.StuckON)\
                                 {B1.BtnON = 0; a}


//------------------------------Game const & vars-------------------------------

#define EVILSTAR_MAX                    15
#define EVILSTAR_DEATHANIMATION_PERIOD  2
#define EVILSTAR_DAMAGE                 2

#define BULLET_MAX                      25
#define BULLET_ENERGY_COST              2
#define BULLET_DAMAGE                   2
#define BULLET_GENERATE_PERIOD_MS       130
#define BULLET_MOVE_PERIOD_MS           13

#define BOMB_MAX                        10
#define BOMB_DAMAGE                     10
#define BOMBSHARD_DAMAGE                5
#define BOMB_ANIMATION_PERIOD_CALLS     4
#define BOMB_GENERATE_PERIOD_MS         300
#define BOMB_MOVE_SPEED                 7
#define BOMB_MONEY_COST                 5

#define SMALLSTAR_MAX                   20
#define SMALLSTAR_MOVE_PERIOD_CALLS     2
#define SMALLSTAR_CREATE_PERIOD_CALLS   5

#define COIN_MAX                        15
#define COIN_ANIMATION_PERIOD           10

#define MAGAZIN_INTROANIMATION_PERIOD   16
#define MAGAZIN_FIRSTENTERINFO_PERIOD   10
#define MINMAGAZ_MOVE_PERIOD_CALLS      10


#define GASMASK_MONEY_COST              15

#define BATTERY_ENERGY_BUST             4
#define BATTERY_MONEY_COST              50

#define HEALTH_REGEN                    4
#define HEALTH_MONEY_COST               5


#define GASCLOUD_DAMAGE                 1

#define GAMER_HEALTH_MAX                24
#define GAMER_ENERGY_MAX                24
#define GAMERDEATH_ANIMATION_PERIOD     20
#define GAME_PROGRESS_PERIOD_CALLS      50

#define PRD_EVILSTAR_CREATE_INITVAL     1400
#define PRD_ENEMY_MOVE_INITVAL          24

tGameProcess Game;
uint32 runtimecounter = 0;
tGamer Gamer;
tDispObj Bullet[BULLET_MAX] = {0};
tBomb    Bomb[BOMB_MAX] = {0};
tDispObj MinMagaz = {0, 8, 127};
tEvilStar EvilStar[EVILSTAR_MAX] = {0};
tCoin Coin[COIN_MAX] = {0};
tSmallStar SmallStar[SMALLSTAR_MAX] = {0};

uint16  PRD_EVILSTAR_CREATE = PRD_EVILSTAR_CREATE_INITVAL;
uint8   PRD_ENEMY_MOVE = PRD_ENEMY_MOVE_INITVAL;
uint16  PRD_GAMER_ENERGYREGEN = 250;
uint8   GAME_STORY_STRING_NUM = 0;

uint16 TotalStars = 0;
uint16 TotalCoins = 0;
uint16 CoinReceived = 0;
uint16 StarsKilled = 0;

tGAMEPROCESFLAGS GameFlags;

typedef enum {
  STATE_MAINMENU,
  STATE_LOADMENU,
  STATE_LOADGAME,
  STATE_PAUSEMENU,
  STATE_SAVEMENU,
  STATE_SAVEGAME,
  STATE_STARTGAME,
  STATE_RUNGAME,
  STATE_MAGAZIN,
  STATE_MAX,
} tGAME_STATE;

typedef enum {
  COURS_POS_1,
  COURS_POS_2,
  COURS_POS_3,
  COURS_POS_4,
  COURS_POS_5,
} tCOURSOR_POS;

typedef enum {
  EVENT_NONE,
  EVENT_SELPOS_1,
  EVENT_SELPOS_2,
  EVENT_SELPOS_3,
  EVENT_SELPOS_4,
  EVENT_PAUSE,
  EVENT_EXIT,
  EVENT_ENTERMAGAZ,
  EVENT_GAMERDEATH,
  EVENT_YOU_ARE_WINNER,
  EVENT_MAX,
} tGAME_EVENT;

tGAME_STATE gamestate = STATE_MAINMENU;
tGAME_STATE gamestate_prev = STATE_MAINMENU;
tCOURSOR_POS coursorpos = COURS_POS_1;
tGAME_EVENT gameevent = EVENT_NONE;
uint8 MENU_ENABLE = 1;

void statehandler_menumain(void);
void statehandler_gameinitnew(void);
void statehandler_menuload(void);
void stateinit_gameexit(void);
void statehandler_gameload(void);
void statehandler_menupause(void);
void statehandler_menusave(void);
void stateinit_gamestop(void);
void statehandler_gamesave(void);
void statehandler_magazin(void);
void statehandler_gamerun(void);
void magaz_buybomb(void);
void magaz_buygasmask(void);
void magaz_buyenergy(void);
void magaz_buyhealth(void);
void stateinit_exitmagazin(void);
void gameprogress(void);

void (*const gamestate_transition_table[STATE_MAX][EVENT_MAX])(void) = {
  [STATE_MAINMENU] [EVENT_NONE] = statehandler_menumain,
  [STATE_MAINMENU] [EVENT_SELPOS_1] = statehandler_gameinitnew,
  [STATE_MAINMENU] [EVENT_SELPOS_2] = statehandler_menuload,
  [STATE_MAINMENU] [EVENT_SELPOS_3] = stateinit_gameexit,

  [STATE_LOADMENU] [EVENT_NONE] = statehandler_menuload,
  [STATE_LOADMENU] [EVENT_SELPOS_1] = statehandler_gameload,
  [STATE_LOADMENU] [EVENT_SELPOS_2] = statehandler_gameload,
  [STATE_LOADMENU] [EVENT_SELPOS_3] = statehandler_menumain,

  [STATE_LOADGAME] [EVENT_NONE] = statehandler_gameload,
  [STATE_LOADGAME] [EVENT_EXIT] = statehandler_gamerun,

  [STATE_PAUSEMENU] [EVENT_NONE] = statehandler_menupause,
  [STATE_PAUSEMENU] [EVENT_SELPOS_1] = statehandler_menusave,
  [STATE_PAUSEMENU] [EVENT_SELPOS_2] = statehandler_gamerun,
  [STATE_PAUSEMENU] [EVENT_SELPOS_3] = stateinit_gamestop,

  [STATE_SAVEMENU] [EVENT_NONE] = statehandler_menusave,
  [STATE_SAVEMENU] [EVENT_SELPOS_1] = statehandler_gamesave,
  [STATE_SAVEMENU] [EVENT_SELPOS_2] = statehandler_gamesave,
  [STATE_SAVEMENU] [EVENT_SELPOS_3] = statehandler_menupause,

  [STATE_SAVEGAME] [EVENT_NONE] = statehandler_gamesave,
  [STATE_SAVEGAME] [EVENT_EXIT] = statehandler_menusave,

  [STATE_STARTGAME] [EVENT_NONE] = statehandler_gameinitnew,
  [STATE_STARTGAME] [EVENT_EXIT] = statehandler_gamerun,

  [STATE_RUNGAME] [EVENT_NONE] = statehandler_gamerun,
  [STATE_RUNGAME] [EVENT_PAUSE] = statehandler_menupause,
  [STATE_RUNGAME] [EVENT_EXIT] = stateinit_gamestop,
  [STATE_RUNGAME] [EVENT_YOU_ARE_WINNER] = statehandler_gamerun,
  [STATE_RUNGAME] [EVENT_ENTERMAGAZ] = statehandler_magazin,

  [STATE_MAGAZIN] [EVENT_NONE] = statehandler_magazin,
  [STATE_MAGAZIN] [EVENT_ENTERMAGAZ] = statehandler_magazin,
  [STATE_MAGAZIN] [EVENT_SELPOS_1] = magaz_buybomb,
  [STATE_MAGAZIN] [EVENT_SELPOS_2] = magaz_buygasmask,
  [STATE_MAGAZIN] [EVENT_SELPOS_3] = magaz_buyenergy,
  [STATE_MAGAZIN] [EVENT_SELPOS_4] = magaz_buyhealth,
  [STATE_MAGAZIN] [EVENT_EXIT] = stateinit_exitmagazin,
};

uint8 gameslot1[25] = "-œ”—“Œ-";
uint8 gameslot2[25] = "-œ”—“Œ-";

void gamestatesprocess(void);

/*******************************************************************************
 *                        SCHEDULER GAME EVENT HANDLERS
 * *****************************************************************************
 */
void createevilstar(void) {
  for(int i = 0; i < EVILSTAR_MAX; i++) {
    if (!EvilStar[i].state) {
      EvilStar[i].state = 1;
      TotalStars++;
      EvilStar[i].cl = 127;
      EvilStar[i].ln = (int8)getrand(40) + 8;
      return;
    }
  }
}

void createcoin(tEvilStar* Evil_Star) {
  for(int i = 0; i < COIN_MAX; i++) {
    if (!Coin[i].state) {
      Coin[i].state = 1;
      TotalCoins++;
      Coin[i].cl = Evil_Star->cl;
      Coin[i].ln = Evil_Star->ln;
      Coin[i].animation_count = COIN_ANIMATION_PERIOD;
      return;
    }
  }
}

void createbullet(void) {
  IF_BTN_B2_PRESS(
    if((Gamer.health > 0)  && (Gamer.energy >= BULLET_ENERGY_COST)) {
      for(int i = 0; i < BULLET_MAX; i ++) {
        if (!Bullet[i].state) {
          Gamer.energy -= BULLET_ENERGY_COST;
          Bullet[i].state = 1;
          Bullet[i].ln = Gamer.ln;
          Bullet[i].cl = Gamer.cl + 24;
          Sounds(400);
          return;
        }
      }
    }
  )
}

void createbomb(void) {
  IF_BTN_B1_PRESS(
    if((Gamer.health > 0)  && Gamer.bombs) {
      for(int i = 0; i < BOMB_MAX; i ++) {
        if (!Bomb[i].state) {
          Gamer.bombs --;
          Bomb[i].state = 1;
          Bomb[i].ln = Gamer.ln;
          Bomb[i].cl = Gamer.cl + 24;
          Sounds(1000);
          return;
        }
      }
    }
  )
}

void energyregen(void) {
  if ((Gamer.energy < Gamer.energymax) && (Gamer.health > 0)) Gamer.energy++;
}

void movgamer(void) 
{
  if(Gamer.health > 0)
  {
    if (joystick.oy > 140 && joystick.oy <= 210){Gamer.ln -= 1; if(Gamer.ln < 8) Gamer.ln = 8;}
    if (joystick.oy > 210 && joystick.oy <= 250){Gamer.ln -= 2;if(Gamer.ln < 8) Gamer.ln = 8;}
    if (joystick.oy > 250){Gamer.ln -= 5; if(Gamer.ln < 8)Gamer.ln = 8;}
    if (joystick.oy < 120 && joystick.oy >= 50){Gamer.ln += 1; if(Gamer.ln > 48)Gamer.ln = 48;}
    if (joystick.oy < 50 && joystick.oy >= 10){Gamer.ln += 2; if(Gamer.ln > 48)Gamer.ln = 48;}
    if (joystick.oy < 10){Gamer.ln += 5; if(Gamer.ln > 48)Gamer.ln = 48;}
    if (joystick.ox > 140 && joystick.ox <= 210){Gamer.cl += 1; if(Gamer.cl > 94)Gamer.cl = 94;}
    if (joystick.ox > 210 && joystick.ox <= 250){Gamer.cl += 2; if(Gamer.cl > 94)Gamer.cl = 94;}
    if (joystick.ox > 250){Gamer.cl += 5; if(Gamer.cl > 94)Gamer.cl = 94;}
    if (joystick.ox < 120 && joystick.ox >= 50){Gamer.cl -= 1; if(Gamer.cl < -8)Gamer.cl = -8;}
    if (joystick.ox < 50 && joystick.ox >= 10){Gamer.cl -= 2; if(Gamer.cl < -8) Gamer.cl = -8;}
    if (joystick.ox < 10){Gamer.cl -= 5; if(Gamer.cl < -8)Gamer.cl = -8;}
  }
}

void movevilstar(void) {
  for (uint8 i = 0; i < EVILSTAR_MAX; i++) {
    if (EvilStar[i].state == 1) {
      EvilStar[i].cl--;
      if ((EvilStar[i].cl < -29)) EvilStar[i].state = 0;
      if(GameFlags.WinTheGame) EvilStar[i].state = 2; 
    }
  }
}

void movbullet(void) {
  for (uint8 i = 0; i < BULLET_MAX; i++) {
    if (Bullet[i].state) {
      Bullet[i].cl += 2;
      if (Bullet[i].cl > 126 || Bullet[i].cl <= 0) {
        Bullet[i].state = 0;
        Bullet[i].cl = 0;
      }
      if(GameFlags.WinTheGame) Bullet[i].state = 0; 
    }
  }
}

void movbomb(void) {
  for(int i = 0; i < BOMB_MAX; i++) {
    if (Bomb[i].state == 1){
      Bomb[i].cl += BOMB_MOVE_SPEED;
      if (Bomb[i].cl > 105) {
        Bomb[i].state = 2;
        Bomb[i].cl -= 16;
        Bomb[i].ln -= 16;
        Sounds(1200);
      }
    }
  }
}

void movcoin(void) {
  for (uint8 i = 0; i < COIN_MAX; i++) {
    if (Coin[i].state == 1) {
      Coin[i].cl -= 1;
      if (Coin[i].cl < -9) Coin[i].state = 0;
    }
  }
}

void movminmagaz(uint8 period){
  static uint8 prd = 0;
  if(MinMagaz.state){
    if(++prd == period)
    {
      prd = 0;
      MinMagaz.cl -= 6;
      if(MinMagaz.cl < 4) {
        MinMagaz.state = 0;
        SchedResumeEvent(createevilstar);
        SchedResumeEvent(gameprogress);
      }
    }
  }
}

void bullet_evilstar_collision(void) {
  for (uint8 j = 0; j < BULLET_MAX; j++) {
    for (uint8 i = 0; i < EVILSTAR_MAX; i++) {
      if ((EvilStar[i].cl <= Bullet[j].cl) && (EvilStar[i].ln < (Bullet[j].ln + 6)) &&
              ((EvilStar[i].ln + 10) > Bullet[j].ln) && EvilStar[i].state == 1 &&
              Bullet[j].state) {
        EvilStar[i].state = 2;
        StarsKilled++;
        Bullet[j].state = 0;
        Sounds(500);
      }
    }
  }
}

void bomb_evilstar_collision(void) {
  for(int j = 0; j < BOMB_MAX; j++) {
    if(Bomb[j].state == 1){
      for (uint8 i = 0; i < EVILSTAR_MAX; i++) {
        if(((EvilStar[i].cl + 28) >= Bomb[j].cl) && (EvilStar[i].cl <= (Bomb[j].cl + 8)) && (EvilStar[i].ln <= (Bomb[j].ln + 8)) &&
              ((EvilStar[i].ln + 16) > Bomb[j].ln) && EvilStar[i].state == 1){
          Bomb[j].state = 2;
          Bomb[j].cl -= 16;
          Bomb[j].ln -= 16;
          Sounds(1200);
        }
      }
    }
    if(Bomb[j].state > 1){
      for (uint8 i = 0; i < EVILSTAR_MAX; i++) {
        if (((EvilStar[i].cl + 30) >= Bomb[j].cl) && (EvilStar[i].cl <= (Bomb[j].cl + 50)) && (EvilStar[i].ln <= (Bomb[j].ln + 39)) &&
                ((EvilStar[i].ln + 16) > Bomb[j].ln) && EvilStar[i].state == 1) {
          EvilStar[i].state = 2;
          StarsKilled++;
          Sounds(500);
        }
      }
    }
  }
}

void gamer_evilstar_collision(void) {
  for (uint8 i = 0; i < EVILSTAR_MAX; i++) {
    if ((EvilStar[i].cl <= (Gamer.cl + 29)) && ((EvilStar[i].cl + 25) >= Gamer.cl) &&
            ((EvilStar[i].ln + 15) > Gamer.ln) && (EvilStar[i].ln < (Gamer.ln + 15)) &&
            (EvilStar[i].state == 1) && (Gamer.health > 0)) {
      EvilStar[i].state = 2;
      StarsKilled++;
      if(Gamer.gasmask_health > 0){
        Gamer.health -= 2;
        Gamer.gasmask_health -= 2;
      }
      else{
        Gamer.health -= 4;
      }
      Sounds(600);
    }
  }
}

void gamer_coin_collision(void) {
  for (uint8 i = 0; i < COIN_MAX; i++) {
    if ((Coin[i].cl <= (Gamer.cl + 29)) && ((Coin[i].cl + 8) >= Gamer.cl) &&
            ((Coin[i].ln + 8) > Gamer.ln) && (Coin[i].ln < (Gamer.ln + 15)) &&
            (Coin[i].state == 1) && (Gamer.health > 0)) {
      Coin[i].state = 0;
      Gamer.money += 1;
      CoinReceived++;
      Sounds(300);
    }
  }
}

void gamer_minmagaz_collision(void) {
  if(MinMagaz.state){
    if ((MinMagaz.cl <= (Gamer.cl + 29)) && ((MinMagaz.cl + 30) >= Gamer.cl) &&
            ((MinMagaz.ln + 28) > Gamer.ln) && (MinMagaz.ln < (Gamer.ln + 15)) &&
            (Gamer.health > 0)) {
      MinMagaz.state = 0;
      gameevent = EVENT_ENTERMAGAZ;
    }
  }
}

void drawevilstar(void) {
  for (uint8 i = 0; i < EVILSTAR_MAX; i++) {
    if (EvilStar[i].state == 1) {
      LCD_printsprite(EvilStar[i].ln, EvilStar[i].cl, &evilstar_sprite);
    }
    if (EvilStar[i].state == 2) {
      LCD_printsprite(EvilStar[i].ln, EvilStar[i].cl, &distr_evilstar_sprite);
      if (EvilStar[i].distr_ttl_count++ >= EVILSTAR_DEATHANIMATION_PERIOD) {
        EvilStar[i].distr_ttl_count = 0;
        EvilStar[i].state = 0;
        createcoin(&EvilStar[i]);
      }
    }
  }
}

void drawcoin(void) {
  for (uint8 i = 0; i < COIN_MAX; i++) {
    if (Coin[i].state == 1) {
      Coin[i].animation_count--;
      if (Coin[i].animation_count >= (COIN_ANIMATION_PERIOD / 2)) LCD_printsprite(Coin[i].ln + 1, Coin[i].cl, &coin_sprite);
      if (Coin[i].animation_count < (COIN_ANIMATION_PERIOD / 2)) LCD_printsprite(Coin[i].ln - 1, Coin[i].cl, &coin_sprite);
      if (Coin[i].animation_count == 0) Coin[i].animation_count = COIN_ANIMATION_PERIOD;
    }
  }
}

void drawminmagaz(void) {
    if (MinMagaz.state) {
      LCD_printsprite(MinMagaz.ln, MinMagaz.cl, &minimag_sprite);
    }
}

void drawgamer(void) {
  if ((Gamer.health > 0)) {
    if (Gamer.gasmask_health > 0) LCD_printsprite(Gamer.ln, Gamer.cl, &gamer_gas_sprite);
    else LCD_printsprite(Gamer.ln, Gamer.cl, &gamer_sprite);
  }
}

void drawbullet(void) {
  for (uint8 i = 0; i < BULLET_MAX; i++) {
    if (Bullet[i].state) {
      LCD_printsprite(Bullet[i].ln, Bullet[i].cl, &bullet_sprite);
    }
  }
}

void drawbomb(uint16 period) {
  for(int i = 0; i < BOMB_MAX; i++) {
    if(Bomb[i].state == 1){
      if (Bomb[i].animation_count >= (3 * period / 4)) LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bomb_sprite[0]);
      if ((Bomb[i].animation_count < (3 * period / 4)) &&
         (Bomb[i].animation_count >= (2 * period / 4))) LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bomb_sprite[1]);
      if ((Bomb[i].animation_count < (2 * period / 4)) &&
         (Bomb[i].animation_count >= (period / 4))) LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bomb_sprite[2]);
      if (Bomb[i].animation_count < (period / 4)) LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bomb_sprite[3]);
      if (Bomb[i].animation_count == 0) Bomb[i].animation_count = period;
      Bomb[i].animation_count--;
    }
    if(Bomb[i].state > 1){

      switch(Bomb[i].state){
        case 2:
          LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bombshards_sprite[0]);
          break;
        case 3:
          LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bombshards_sprite[1]);
          break;
        case 4: 
          LCD_printsprite(Bomb[i].ln, Bomb[i].cl, &bombshards_sprite[2]);
          break;
      }
      if(Bomb[i].animation_count > (period / 2)){
  	    Bomb[i].animation_count--;
    	return;
      }
      else {
	    Bomb[i].state++;
	    Bomb[i].animation_count = period;
	    if(Bomb[i].state > 4) {
	      Bomb[i].state = 0;
	    }
      }
    }
  }
}

void drawinfo(void) {
  LCD_printgamestatbar(&Gamer);
}

void screenupdate(void) {
  LCD_bufupload_buferase();
}

//-------------------Background smallstars animation----------

void createsmallstar(uint8 create_period)
{
  static uint8 k = 0;
  if(k == 0) k = create_period;
  if(k == create_period)
  {
    for(int i = 0; i < SMALLSTAR_MAX; i++)
    {
      if(SmallStar[i].state == 0){
        SmallStar[i].state = getrand(1) + 1;
        SmallStar[i].ln = (int8)getrand(60);
        SmallStar[i].cl = 127;
        break;
      }
    }
  }
  k--;
}

void movesmallstar(uint8 move_period)
{
  static uint8 l = 0;
  if(l == 0) l = move_period;
  if(l == move_period) {
    for(uint8 i = 0; i < SMALLSTAR_MAX; i++)
    {
      if(SmallStar[i].state != 0) {
      SmallStar[i].cl -= 1;
      if(SmallStar[i].cl < -2) SmallStar[i].state = 0;
      }
    }
  }
  l--;
}

void drawsmallstar(void)
{
  for(uint8 i = 0; i < SMALLSTAR_MAX; i++)
  {
    if(SmallStar[i].state) {
      LCD_printsprite(SmallStar[i].ln, SmallStar[i].cl, &smallstar_sprite[SmallStar[i].state - 1]);
    }
  }
}

//-------------------RUNGAME PROGRESS PROCESS----------
void gameprogress(void)
{
  static uint8 counter = 0;
  if(Gamer.health > 0)
  {
	PRD_EVILSTAR_CREATE = PRD_EVILSTAR_CREATE_INITVAL - 100 * (Game.level_progress / 15);
	PRD_ENEMY_MOVE = PRD_ENEMY_MOVE_INITVAL - (Game.level_progress / 15);

    if((Game.level_progress % 15 == 0) && counter == 0 && Game.level_progress){
	  if(PRD_EVILSTAR_CREATE < 500) GameFlags.WinTheGame = 1;
	  else {
		MinMagaz.state = 1;
		MinMagaz.cl = 127;
		SchedPauseEvent(createevilstar);
		SchedPauseEvent(gameprogress);
	  }
    }


    counter++;
    if(counter == GAME_PROGRESS_PERIOD_CALLS)
    {
      Game.level_progress++;
      counter = 0;
    }
  }
}
//--------------------------COMBINED SCHEDULER EVENTS---------------------------
void move_enemy_objects(void) {
  movevilstar();
  movcoin();
}

/*******************************************************************************
 *                              GAME HANDLERS                            
 *******************************************************************************
 */
void SchedGamerunEventsAdd(void)
{
  SchedAddEvent(createevilstar, PRD_EVILSTAR_CREATE);
  SchedAddEvent(createbullet, BULLET_GENERATE_PERIOD_MS);
  SchedAddEvent(createbomb, BOMB_GENERATE_PERIOD_MS);
  SchedAddEvent(movbullet, BULLET_MOVE_PERIOD_MS);
  SchedAddEvent(move_enemy_objects, PRD_ENEMY_MOVE);
  SchedAddEvent(gameprogress, 75);
}

void SchedGamerunEventsPause(void)
{
  SchedPauseEvent(createevilstar);
  SchedPauseEvent(createbullet);
  SchedPauseEvent(createbomb);
  SchedPauseEvent(movbullet);
  SchedPauseEvent(move_enemy_objects);
  SchedPauseEvent(gameprogress);
}

void SchedGamerunEventsResume(void)
{
  SchedResumeEvent(createevilstar);
  SchedResumeEvent(createbullet);
  SchedResumeEvent(createbomb);
  SchedResumeEvent(movbullet);
  SchedResumeEvent(move_enemy_objects);
  SchedResumeEvent(gameprogress);
}

void SchedGamerunEventsRemove(void)
{
  SchedRemoveEvent(createevilstar);
  SchedRemoveEvent(createbullet);
  SchedRemoveEvent(createbomb);
  SchedRemoveEvent(movbullet);
  SchedRemoveEvent(move_enemy_objects);
  SchedRemoveEvent(gameprogress);
}

void statehandler_gameinitnew(void)
{
  gamestate = STATE_STARTGAME;
  gameevent = EVENT_NONE;
 
  if(GAME_STORY_STRING_NUM < 4)
  {
    LCD_printstr8x5(gamestory_string[GAME_STORY_STRING_NUM], 0, 0);
    LCD_printstr8x5((uint8*)"‰‡ÎÂÂ...", 7, 78);
    if(B1.BtnON || B2.BtnON || B3.BtnON || B4.BtnON)
    {
      B1.BtnON = 0;
      B2.BtnON = 0;
      B3.BtnON = 0;
      B4.BtnON = 0;
      GAME_STORY_STRING_NUM++;
    }
  }
  else
  {
    Gamer.health = 24;
    Gamer.energy = 4;
    Gamer.energymax = 4;
    Gamer.gasmask_health = 0;
    Gamer.bombs = 10;
    Gamer.money = 50;
    Gamer.ln = 16;
    Gamer.cl = 0;
    
    Game.level_progress = 0;
    GameFlags.gameflagsreg = 0b00001101;

    BTN_HOLD_ON_DELAY = 50;
    SchedGamerunEventsAdd();
    gameevent = EVENT_EXIT; 
  }
}

void statehandler_gamesave(void)
{
	uint8 temp;
  if(coursorpos == COURS_POS_1){
	EEPROM_writebyte(2, rtcraw.day);
	temp = (EEPROM_readbyte(3) & 0x0F) + 1;
	if(temp > 15) temp = 0;
	EEPROM_writebyte(3, temp | (rtcraw.month << 4));
	sprintf((char*)gameslot1, "—Œ’–.1 - %2.2d.%2.2d_%d", EEPROM_readbyte(2), EEPROM_readbyte(3) >> 4,  EEPROM_readbyte(3) & 0x0F);
	EEPROM_writebyte(4, Gamer.health);
	EEPROM_writebyte(5, Gamer.energymax);
	EEPROM_writebyte(6, Gamer.gasmask_health);
	EEPROM_writebyte(7, Gamer.bombs);
	EEPROM_writebyte(8, Gamer.money);
	EEPROM_writebyte(9, Game.level_progress);
	EEPROM_writebyte(10, GameFlags.gameflagsreg);
  }
  if(coursorpos == COURS_POS_2){
	EEPROM_writebyte(11, rtcraw.day);
	temp = (EEPROM_readbyte(12) & 0x0F) + 1;
	if(temp > 15) temp = 0;
	EEPROM_writebyte(12, temp | (rtcraw.month << 4));
	sprintf((char*)gameslot2, "—Œ’–.2 - %2.2d.%2.2d_%d", EEPROM_readbyte(11), EEPROM_readbyte(12) >> 4,  EEPROM_readbyte(12) & 0x0F);
	EEPROM_writebyte(13, Gamer.health);
	EEPROM_writebyte(14, Gamer.energymax);
	EEPROM_writebyte(15, Gamer.gasmask_health);
	EEPROM_writebyte(16, Gamer.bombs);
	EEPROM_writebyte(17, Gamer.money);
	EEPROM_writebyte(18, Game.level_progress);
	EEPROM_writebyte(19, GameFlags.gameflagsreg);
  }
  LCD_bufupload_buferase();
  LCD_printstr8x5((uint8*)"—Œ’–¿Õ≈ÕŒ", 3, 36);
  LCD_bufupload_buferase();
  HAL_Delay(1000);
  gameevent = EVENT_NONE;
}

void statehandler_gameload(void)
{
  if(coursorpos == COURS_POS_1){
    Gamer.health = EEPROM_readbyte(4);
    Gamer.energymax = EEPROM_readbyte(5);
    Gamer.energy = Gamer.energymax;
    Gamer.gasmask_health = EEPROM_readbyte(6);
    Gamer.bombs = EEPROM_readbyte(7);
    Gamer.money = EEPROM_readbyte(8);
    Game.level_progress = EEPROM_readbyte(9);
    GameFlags.gameflagsreg = EEPROM_readbyte(10);
  }
  if(coursorpos == COURS_POS_2){
    Gamer.health = EEPROM_readbyte(13);
    Gamer.energymax = EEPROM_readbyte(14);
    Gamer.energy = Gamer.energymax;
    Gamer.gasmask_health = EEPROM_readbyte(15);
    Gamer.bombs = EEPROM_readbyte(16);
    Gamer.money = EEPROM_readbyte(17);
    Game.level_progress = EEPROM_readbyte(18);
    GameFlags.gameflagsreg = EEPROM_readbyte(19);
  }
  BTN_HOLD_ON_DELAY = 50;
  SchedGamerunEventsAdd();
  gamestate = STATE_RUNGAME;
  gameevent = EVENT_NONE;
}

void statehandler_gamerun(void)
{
  uint8 string[10];
  if(gamestate == STATE_PAUSEMENU) SchedGamerunEventsResume();
  
  static uint8 deadgamerdrawcounter = GAMERDEATH_ANIMATION_PERIOD;
  
  gamestate = STATE_RUNGAME;
  if(GameFlags.WinTheGame){
    SchedGamerunEventsPause();

    LCD_printstr8x5((uint8*)"œŒ¡≈ƒ¿!!!", 2, 35);

    u16_to_str(string, StarsKilled, 10);
    LCD_printstr8x5((uint8*)"  ”¡»“Œ «¬≈«ƒ:", 4, 5);
    LCD_printstr8x5((uint8*)string, 4, 90);
    u16_to_str(string, CoinReceived, 10);
    LCD_printstr8x5((uint8*)"—Œ¡–¿ÕŒ ÃŒÕ≈“:", 5, 5);
    LCD_printstr8x5((uint8*)string, 5, 90);

    static uint16 tempcounter = 0;
    if(tempcounter++ > 50) {
		IF_ANY_BTN_PRESS(
		  deadgamerdrawcounter = GAMERDEATH_ANIMATION_PERIOD;
		  gameevent = EVENT_EXIT;
		  tempcounter = 0;
		)
    }
    IF_ANY_BTN_PRESS();
  }
  else{
    if(Gamer.health > 0)
    {
      if(B4.BtnON) {
        B4.BtnON = 0;
        gameevent = EVENT_PAUSE;
      }
      else gameevent = EVENT_NONE;
    }
    else   // draw dead player
    {
      if(deadgamerdrawcounter >= (3 * (GAMERDEATH_ANIMATION_PERIOD / 4))){
        LCD_printsprite(Gamer.ln, Gamer.cl, &gamer_dead_0_sprite);
         deadgamerdrawcounter--;
      }
      if(deadgamerdrawcounter < (3 * (GAMERDEATH_ANIMATION_PERIOD / 4) + (GAMERDEATH_ANIMATION_PERIOD % 4)) && deadgamerdrawcounter > 0) {
        LCD_printsprite(Gamer.ln, Gamer.cl, &gamer_dead_1_sprite);
        deadgamerdrawcounter--;
        IF_ANY_BTN_PRESS()
      }
      if(deadgamerdrawcounter == 0)
      {
        LCD_printstr8x5((uint8*)"«¬≈«ƒ¿Õ”À—ﬂ Õ¿—Ã≈–“‹!", 2, 2);
        u16_to_str(string, StarsKilled, 10);
		LCD_printstr8x5((uint8*)"  ”¡»“Œ «¬≈«ƒ:", 4, 5);
		LCD_printstr8x5((uint8*)string, 4, 90);
		u16_to_str(string, CoinReceived, 10);
		LCD_printstr8x5((uint8*)"—Œ¡–¿ÕŒ ÃŒÕ≈“:", 5, 5);
		LCD_printstr8x5((uint8*)string, 5, 90);
        IF_ANY_BTN_PRESS(
          deadgamerdrawcounter = GAMERDEATH_ANIMATION_PERIOD;
          gameevent = EVENT_EXIT;
        )
      }
    }
  }

  SchedEventSetPeriod(createevilstar, PRD_EVILSTAR_CREATE);
  SchedEventSetPeriod(move_enemy_objects, PRD_ENEMY_MOVE);
  
  movgamer();
  movbomb();
  bullet_evilstar_collision();
  gamer_evilstar_collision();
  gamer_coin_collision();
  gamer_minmagaz_collision();
  bomb_evilstar_collision();
  drawevilstar();
  drawcoin();
  drawminmagaz();
  drawbullet();
  drawbomb(BOMB_ANIMATION_PERIOD_CALLS);
  drawgamer();
  drawinfo();
  
    
}

void stateinit_gamestop(void)
{
  SchedGamerunEventsRemove();
  for(uint8 i = 0; i < EVILSTAR_MAX; i++) EvilStar[i].state = 0;
  for(uint8 i = 0; i < COIN_MAX; i++) Coin[i].state = 0;
  for(uint8 i = 0; i < BULLET_MAX; i++) Bullet[i].state = 0;
  GAME_STORY_STRING_NUM = 0;
  BTN_HOLD_ON_DELAY = 300;
  Game.level_progress = 0;
  gamestate = STATE_MAINMENU;
  gameevent = EVENT_NONE;
  coursorpos = COURS_POS_1;
}

void stateinit_gameexit(void)
{
  CFlags.RunGameFl = 0;
  CFlags.MenuFl = 1;
  SchedRemoveAllEvents();
}

/*******************************************************************************
 *                              GAME MENU HANDLERS                            
 *******************************************************************************
 */
//EVENT CHECKER
void menu_getevent(void)
{
  static uint8 somethinghappen = 0;
  IF_ANY_BTN_PRESS(
          switch(coursorpos)
          {
            case COURS_POS_1:
              gameevent = EVENT_SELPOS_1;
              break;
            case COURS_POS_2:
              gameevent = EVENT_SELPOS_2;
              break;
            case COURS_POS_3:
              gameevent = EVENT_SELPOS_3;
              break;
            case COURS_POS_4:
              gameevent = EVENT_SELPOS_3;
              break;
            case COURS_POS_5:
              gameevent = EVENT_SELPOS_3;
              break;
          }
          somethinghappen = 1;
          )
  if(somethinghappen) somethinghappen = 0;
  else gameevent = EVENT_NONE;
}

void magaz_getevent(void)
{
  static uint8 somethinghappen = 0;
  IF_ANY_BTN_PRESS(
          switch(coursorpos)
          {
            case COURS_POS_1:
              gameevent = EVENT_EXIT;
              break;
            case COURS_POS_2:
              gameevent = EVENT_SELPOS_1;
              break;
            case COURS_POS_3:
              gameevent = EVENT_SELPOS_2;
              break;
            case COURS_POS_4:
              gameevent = EVENT_SELPOS_3;
              break;
            case COURS_POS_5:
              gameevent = EVENT_SELPOS_4;
              break;
          }
          somethinghappen = 1;
          )
  if(somethinghappen) somethinghappen = 0;
  else gameevent = EVENT_NONE;
}

void coursormovdisp(void)
{
  //move coursor
  if(gamestate != STATE_MAGAZIN)
  {
    if (joystick.down) {
      joystick.down = 0;
      coursorpos++; 
      if(coursorpos >= COURS_POS_3) coursorpos = COURS_POS_3;
    }
    if (joystick.up){
      joystick.up = 0;
      if(coursorpos != COURS_POS_1) coursorpos--; 
    }
    switch(coursorpos)
	{
	  case COURS_POS_1:
		LCD_printmenucoursor(2, 4);
		break;
	  case COURS_POS_2:
		LCD_printmenucoursor(4, 4);
		break;
	  case COURS_POS_3:
		LCD_printmenucoursor(6, 4);
		break;
	  case COURS_POS_4:
		LCD_printmenucoursor(6, 4);
		break;
	  case COURS_POS_5:
		LCD_printmenucoursor(6, 4);
		break;
	}
  }
  else
  {
    if (joystick.right) {
      joystick.right = 0;
      if(coursorpos >= COURS_POS_5) coursorpos = COURS_POS_5;
      else coursorpos++; 
    }
    if (joystick.left){
      joystick.left = 0;
      if(coursorpos != COURS_POS_1) coursorpos--; 
    }
    switch(coursorpos)
    {
      case COURS_POS_1:
        LCD_printhorline(26, 63, 36);
        LCD_printhorline(26, 54, 36);
        LCD_printvertline(9, 54, 36);
        LCD_printvertline(9, 54, 62);
        break;
      case COURS_POS_2:
        LCD_printhorline(12, 61, 66);
        LCD_printhorline(12, 62, 66);
        LCD_printvertline(3, 58, 66);
        LCD_printvertline(3, 58, 78);
        break;
      case COURS_POS_3:  
        LCD_printhorline(18, 61, 78);
        LCD_printhorline(18, 62, 78);
        LCD_printvertline(3, 58, 78);
        LCD_printvertline(3, 58, 96);
        break;
      case COURS_POS_4:  
        LCD_printhorline(14, 61, 97);
        LCD_printhorline(14, 62, 97);
        LCD_printvertline(3, 58, 97);
        LCD_printvertline(3, 58, 111);
        break;
      case COURS_POS_5:  
        LCD_printhorline(14, 61, 111);
        LCD_printhorline(14, 62, 111);
        LCD_printvertline(3, 58, 111);
        LCD_printvertline(3, 58, 125);
        break;
    }
  }
}

void statehandler_menumain(void)
{
  gamestate = STATE_MAINMENU;
  menu_getevent();
  LCD_printstr8x5((uint8*)"√¿À¿ “»◊≈— »… «¬≈«ƒ≈÷", 0, 0);
  LCD_printstr8x5((uint8*)"ÕŒ¬¿ﬂ »√–¿", 2, 19);
  LCD_printstr8x5((uint8*)"«¿√–”«»“‹ »√–”", 4, 19);
  LCD_printstr8x5((uint8*)"¬€…“»", 6, 19);
  coursormovdisp();
  if(gameevent == EVENT_SELPOS_2) coursorpos = COURS_POS_1; // coursor position when entering the load menu in "SLOT1"
}

void statehandler_menuload(void)
{ 
  gamestate = STATE_LOADMENU;
  menu_getevent();
  LCD_printstr8x5((uint8*)"«¿√–”« ¿ »√–€", 0, 5);
  LCD_printstr8x5(gameslot1, 2, 19);
  LCD_printstr8x5(gameslot2, 4, 19);
  LCD_printstr8x5((uint8*)"Õ¿«¿ƒ", 6, 19);
  coursormovdisp();
  if(gameevent == EVENT_SELPOS_3) coursorpos = COURS_POS_2; // coursor position when returning to the main menu in "LOAD"
}

void statehandler_menupause(void)
{
  if(gamestate == STATE_RUNGAME) SchedGamerunEventsPause();
  gamestate = STATE_PAUSEMENU;
  
  menu_getevent();
  LCD_printstr8x5((uint8*)"œ¿”«¿", 0, 5);
  LCD_printstr8x5((uint8*)"—Œ’–¿Õ»“‹", 2, 19);
  LCD_printstr8x5((uint8*)"¬≈–Õ”“‹—ﬂ   »√–≈", 4, 19);
  LCD_printstr8x5((uint8*)"¬€…“»", 6, 19);
  coursormovdisp();
}

void statehandler_menusave(void)
{
  gamestate = STATE_SAVEMENU;
  menu_getevent();
  LCD_printstr8x5((uint8*)"—Œ’–¿Õ≈Õ»≈ »√–€", 0, 5);
  LCD_printstr8x5(gameslot1, 2, 19);
  LCD_printstr8x5(gameslot2, 4, 19);
  LCD_printstr8x5((uint8*)"Õ¿«¿ƒ", 6, 19);
  coursormovdisp();
  if(gameevent == EVENT_SELPOS_3) coursorpos = COURS_POS_1; // coursor position when returning to the pause menu in "SAVE"
}

//------------------magazin buygoods functions----------------------
void magaz_buybomb(void)
{
  if(Gamer.money >= BOMB_MONEY_COST && Gamer.bombs < 99){
    Gamer.money -= BOMB_MONEY_COST;
    Gamer.bombs++;
  }
  gameevent = EVENT_NONE;
}
void magaz_buygasmask(void)
{
  if(Gamer.money >= GASMASK_MONEY_COST && Gamer.gasmask_health <= 0){
    Gamer.money -= GASMASK_MONEY_COST;
    Gamer.gasmask_health = 12;
  }
  gameevent = EVENT_NONE;
}
void magaz_buyenergy(void)
{
  if(Gamer.money >= BATTERY_MONEY_COST && Gamer.energymax < GAMER_ENERGY_MAX){
    Gamer.money -= BATTERY_MONEY_COST;
    Gamer.energymax += BATTERY_ENERGY_BUST;
    Gamer.energy = Gamer.energymax;
    if(Gamer.energymax > GAMER_ENERGY_MAX) Gamer.energymax = GAMER_ENERGY_MAX;
  }
  gameevent = EVENT_NONE;
}
void magaz_buyhealth(void)
{
  if(Gamer.money >= HEALTH_MONEY_COST && Gamer.health < GAMER_HEALTH_MAX){
    Gamer.money -= HEALTH_MONEY_COST;
    Gamer.health += HEALTH_REGEN; 
    if(Gamer.health > GAMER_HEALTH_MAX) Gamer.health = GAMER_HEALTH_MAX;
  }
  gameevent = EVENT_NONE;
}

//-----------------------------MAGAZIN MENU HANDLER---------------------------
void statehandler_magazin(void)
{
  if(gamestate == STATE_RUNGAME) SchedGamerunEventsPause();
  gamestate = STATE_MAGAZIN; 
  if(gameevent == EVENT_ENTERMAGAZ) // entering magazin animation
  {
    static uint8 j = MAGAZIN_INTROANIMATION_PERIOD;
    static uint8 k = MAGAZIN_FIRSTENTERINFO_PERIOD;
    if(GameFlags.FirstMagazEnter) {
      if(k == 0){
        LCD_printstr8x5(gamestory_string[4], 1, 0);
        LCD_printstr8x5((uint8*)"‰‡ÎÂÂ...", 7, 78);
        IF_ANY_BTN_PRESS(
                GameFlags.FirstMagazEnter = 0;
                k = MAGAZIN_FIRSTENTERINFO_PERIOD;
                )
      }
      else {
        IF_ANY_BTN_PRESS()
        k--;
      }
    }
    else {
      LCD_printsprite(8, (64 + (int8)j * 4), &magazin_sprite);
      j--;
      IF_ANY_BTN_PRESS()
      if(j == 0) {
        gameevent = EVENT_NONE;
        j = MAGAZIN_INTROANIMATION_PERIOD;
      }
    }
  }
  else
  {
    coursormovdisp();
    magaz_getevent();
    LCD_printsprite(8, 64, &magazin_sprite);
    LCD_printstr8x5((uint8*)"÷≈Õ¿:", 3, 94);
    LCD_printstr8x5((uint8*)"Ï‡„‡ÁËÌ", 1, 80);
    LCD_printstr8x5((uint8*)"¬€’.", 7, 38);
    uint8 price[5];
    switch(coursorpos)
    {
      case COURS_POS_1:
        LCD_printstr8x5((uint8*)"ƒÓ\nÒ‚Ë‰‡ÌËˇ!", 2, 0);
        break;
      case COURS_POS_2:
        LCD_printstr8x5((uint8*)"¡ŒÃ¡¿!!!\n¡ŒÀ‹ÿŒ…\n¡¿ƒ¿¡”Ã!", 2, 0);
        u16_to_str(price, BOMB_MONEY_COST, 1);
        LCD_printstr8x5((uint8*)price, 4, 100);
        break;
      case COURS_POS_3:
        LCD_printstr8x5((uint8*)"<¡ÓÌÂ¯ÎÂÏ.\n—ÌËÊ‡ÂÚ\nÛÓÌ.", 2, 0);
        u16_to_str(price, GASMASK_MONEY_COST, 2);
        LCD_printstr8x5((uint8*)price, 4, 94);
        break;
      case COURS_POS_4:
        LCD_printstr8x5((uint8*)"¡‡Ú‡ÂÈÍ‡!", 2, 0);
        u16_to_str(price, BATTERY_MONEY_COST, 2);
        LCD_printstr8x5((uint8*)price, 4, 94);
        break;
      case COURS_POS_5:
        LCD_printstr8x5((uint8*)"¿ÔÚÂ˜Í‡!", 2, 0);
        u16_to_str(price, HEALTH_MONEY_COST, 1);
        LCD_printstr8x5((uint8*)price, 4, 100);
        break;
    }
  }
  #if 1
  if(Gamer.cl > 8) Gamer.cl -= 4; else Gamer.cl = 0;
  if(Gamer.ln < 40) Gamer.ln += 4; else Gamer.ln = 47;
  drawgamer();
  #endif
  drawinfo();
}

void stateinit_exitmagazin(void)
{
  static uint8 counter = 0;
  if(++counter < 15) LCD_printsprite((int8)(8 + counter * 4), 64, &magazin_sprite);
  else {
    counter = 0;
    gamestate = STATE_RUNGAME;
    gameevent = EVENT_NONE;
    SchedGamerunEventsResume();
  }
  drawgamer();
  drawinfo();
}
//--------------------------SYSTEM FUNCTIONS-------------------------------  
void system_events_period25ms(void) 
{
  movesmallstar(SMALLSTAR_MOVE_PERIOD_CALLS);
  movminmagaz(MINMAGAZ_MOVE_PERIOD_CALLS);
}

void system_events_period50ms(void)
{
  //JOY_DIRECT_RESET;
  //IF_ANY_BTN_PRESS();
  check_btn_jstk();
}

void system_events_period100ms(void) 
{
  batcheck();
  createsmallstar(SMALLSTAR_CREATE_PERIOD_CALLS);
  drawsmallstar();  

  check_btn_jstk();
  screenupdate();
}

void gamestatesprocess(void)
{
  gamestate_transition_table[gamestate][gameevent]();
}
/*******************************************************************************
 *                              MAIN ENTRY - GAME CYCLE                     
 *******************************************************************************
 */
void ufobattle(void)
{
  randinit();
  SchedAddEvent(system_events_period25ms, 25);
  SchedAddEvent(system_events_period50ms, 50);
  SchedAddEvent(system_events_period100ms, 100);
  SchedAddEvent(gamestatesprocess, 75);
  SchedAddEvent(energyregen, PRD_GAMER_ENERGYREGEN);
  gamestate = STATE_MAINMENU;
  gameevent = EVENT_NONE;
  coursorpos = COURS_POS_1;
  
  if(EEPROM_readbyte(2)) sprintf((char*)gameslot1, "—Œ’–.1 - %2.2d.%2.2d_%d", EEPROM_readbyte(2), EEPROM_readbyte(3) >> 4,  EEPROM_readbyte(3) & 0x0F);
  else sprintf((char*)gameslot1, "-œ”—“Œ-");
  if(EEPROM_readbyte(11)) sprintf((char*)gameslot2, "—Œ’–.1 - %2.2d.%2.2d_%d", EEPROM_readbyte(11), EEPROM_readbyte(12) >> 4,  EEPROM_readbyte(12) & 0x0F);
  else sprintf((char*)gameslot2, "-œ”—“Œ-");

  while (CFlags.RunGameFl)
  {
    SchedEventProcess();
  }
}
