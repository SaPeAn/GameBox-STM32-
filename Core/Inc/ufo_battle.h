
#ifndef UFO_BATTLE_H
#define	UFO_BATTLE_H

#include "common.h"

typedef struct {
  uint8 level;
  uint8 level_progress;
  uint8 difficulty;
  uint8 Const1;
  uint8 Const2;
  uint8 Const3;
} tGameProcess;

//------------------------------Game objects------------------------------------

typedef struct {
  int8 health;
  int8 energy;
  int8 energymax;
  int8 gasmask_health;
  uint8 bombs;
  uint16 money;
  int8 ln;
  int8 cl;
} tGamer;

typedef struct {
  uint8 state;
  int8 ln;
  int8  cl;
} tDispObj;

typedef struct {
  uint8 state; // 0 - Comet disable; 1 - Comet enable; 2 - Comet distroed
  int8 ln;
  int8  cl;
  uint8 distr_ttl_count;
} tEvilStar;

typedef struct {
  uint8 state;
  int8 ln;
  int8  cl;
  uint8 animation_count;
} tCoin;

typedef struct {
  uint8 state; // 0 - none, 1 - bomb, 2- , 3- , 4- , 5- , 
  int8 ln;
  int8  cl;
  uint16 animation_count;
} tBomb;

typedef struct {
  uint8 state;
  int8 ln;
  int8  cl;
} tSmallStar;
//------------------------------Game const & vars-------------------------------

#define EVILSTAR_MAX                    10
#define EVILSTAR_DEATHANIMATION_PERIOD  2
#define EVILSTAR_DAMAGE                 2

#define BULLET_MAX                      18
#define BULLET_ENERGY_COST              2
#define BULLET_DAMAGE                   2
#define BULLET_GENERATE_PERIOD          1
#define BULLET_MOVE_PERIOD              13

#define SMALLSTAR_MAX                   12
#define SMALLSTAR_MOVE_PERIOD           2
#define SMALLSTAR_CREATE_PERIOD         5

#define COIN_MAX                        10
#define COIN_ANIMATION_PERIOD           10

#define MAGAZIN_INTROANIMATION_PERIOD   16
#define MAGAZIN_FIRSTENTERINFO_PERIOD   10
#define MINMAGAZ_MOVE_PERIOD            10

#define BOMB_MONEY_COST                 5
#define GASMASK_MONEY_COST              15
#define BATTERY_MONEY_COST              50
#define HEALTH_MONEY_COST               5

#define BATTERY_ENERGY_BUST             4
#define HEALTH_REGEN                    4
#define BOMB_DAMAGE                     10
#define BOMBSHARD_DAMAGE                5
#define BOMB_ANIMATION_PERIOD           4
#define BOMB_GENERATE_PERIOD            1
#define BOMB_MOVE_SPEED                 7

#define GASCLOUD_DAMAGE                 1

#define GAMER_HEALTH_MAX                24
#define GAMER_ENERGY_MAX                24
#define GAMERDEATH_ANIMATION_PERIOD     20
#define GAME_PROGRESS_PERIOD            20

tGameProcess Game;                         
uint32 runtimecounter = 0;
tGamer Gamer;
tDispObj Bullet[BULLET_MAX] = {0};
tBomb    Bomb = {0};
tDispObj MinMagaz = {0, 8, 127};
tEvilStar EvilStar[EVILSTAR_MAX] = {0};
tCoin Coin[COIN_MAX] = {0};
tSmallStar SmallStar[SMALLSTAR_MAX] = {0};

uint16  PRD_EVILSTAR_CREATE = 1400;
uint16  PRD_EVILSTAR_CREATE_PREV = 1400;
uint8   PRD_ENEMY_MOVE = 24;
uint8   PRD_ENEMY_MOVE_PREV = 24;
uint16  PRD_GAMER_ENERGYREGEN = 4;
uint8   GAME_STORY_STRING_NUM = 0;

typedef union{
    uint8 gameflagsreg;
    struct 
    {
      unsigned EvilstarCreateEnable  :1;
      unsigned ChemistCreateEnable   :1;
      unsigned FirstMagazEnter       :1;
      unsigned MagazEnter            :1;
      unsigned WinTheGame            :1;
      unsigned Flag2       :1;
      unsigned Flag3       :1;
      unsigned Flag4       :1;
    };
} tGAMEPROCESFLAGS;
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

uint8 gameslot1[20] = "-осярн-";
uint8 gameslot2[20] = "-осярн-";


void ufobattle(void);

void gamestatesprocess(void);

#endif	/* UFO_BATTLE_H */

