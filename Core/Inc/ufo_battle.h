
#ifndef UFO_BATTLE_H
#define	UFO_BATTLE_H

#include "common.h"

//------------------------------Game structures------------------------------------

typedef struct {
  uint8 level;
  uint8 level_progress;
  uint8 difficulty;
  uint8 Const1;
  uint8 Const2;
  uint8 Const3;
} tGameProcess;

typedef struct {
  int8 health;
  int8 energy;
  int8 energymax;
  int8 gasmask_health;
  uint8 bombs;
  uint8 money;
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
      unsigned GameLevel             :2;
    };
} tGAMEPROCESFLAGS;
extern tGamer Gamer;
void ufobattle(void);


#endif	/* UFO_BATTLE_H */

