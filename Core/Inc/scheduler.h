
#ifndef SCHEDULER_H
#define	SCHEDULER_H

#include "common.h"

#define      MAX_EVENT    30
#define      ON           1
#define      OFF          0


typedef struct {
  void     (*callfunc)(void);
  uint16   period;
  uint16   eventcounter;
  uint8    run_flag;
} tEvent;

extern tEvent SchedulerEvent[MAX_EVENT];

void  SchedPeriodIncr(void);
uint8 SchedAddEvent(void (*)(void), uint16);
void  SchedRemoveEvent(void (*)(void));
void  SchedRemoveAllEvents(void);
void  SchedPauseEvent(void (*)(void));
void  SchedResumeEvent(void (*)(void));
void  SchedEventProcess(void);
void  SchedEventSetPeriod(void (*)(void), uint16);

#endif	

