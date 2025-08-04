#include "scheduler.h"


static uint8 SchedulerRegistredEvents = 0;
tEvent SchedulerEvent[MAX_EVENT];

uint8 SchedAddEvent(void (*func)(void), uint16 period)
{
  SchedulerEvent[SchedulerRegistredEvents].callfunc = func;
  SchedulerEvent[SchedulerRegistredEvents].period = period;
  SchedulerEvent[SchedulerRegistredEvents].run_flag = ON;
  return SchedulerRegistredEvents++;
}

void SchedRemoveAllEvents(void)
{
  SchedulerRegistredEvents = 0;
}

void SchedEventProcess(void)
{
  for(uint8 i = 0; i < SchedulerRegistredEvents; i++)
  {
    if(SchedulerEvent[i].eventcounter >= SchedulerEvent[i].period)
    {
      SchedulerEvent[i].eventcounter = 0;
      SchedulerEvent[i].callfunc();
    }
  }
}

void SchedPeriodIncr(void)
{
  for(uint8 i = 0; i < SchedulerRegistredEvents; i++) {
    if(SchedulerEvent[i].run_flag) 
      SchedulerEvent[i].eventcounter++;
  }
}

void  SchedRemoveEvent(void (*func)(void))
{
    uint8 temp_fl = 0;
    for(uint8 i = 0; i < SchedulerRegistredEvents; i++){
        if(SchedulerEvent[i].callfunc == func || temp_fl) {
            temp_fl = 1;
            SchedulerEvent[i].callfunc = SchedulerEvent[i+1].callfunc;
            SchedulerEvent[i].period = SchedulerEvent[i+1].period;
            SchedulerEvent[i].run_flag = SchedulerEvent[i+1].run_flag;
            SchedulerEvent[i].eventcounter = SchedulerEvent[i+1].eventcounter;
        }
    }
    if(temp_fl) SchedulerRegistredEvents--;
}

void  SchedPauseEvent(void (*func)(void))
{
    for(uint8 i = 0; i < SchedulerRegistredEvents; i ++){
        if(SchedulerEvent[i].callfunc == func){
            SchedulerEvent[i].run_flag = OFF;
        }
    }
}
void  SchedResumeEvent(void (*func)(void))
{
    for(uint8 i = 0; i < SchedulerRegistredEvents; i ++){
        if(SchedulerEvent[i].callfunc == func){
            SchedulerEvent[i].run_flag = ON;
        }
    }
}

void  SchedEventSetPeriod(void (*func)(void), uint16 period)
{
    for(uint8 i = 0; i < SchedulerRegistredEvents; i ++){
        if(SchedulerEvent[i].callfunc == func){
            SchedulerEvent[i].period = period;
        }
    }
}
