#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/libraries.h>
#include <libraries/dos.h>

#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>
#include <clib/graphics_protos.h>
#include <clib/timer_protos.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <stdlib.h>
#include <stdio.h>

/*
  Delta time & g_clock
*/
struct IORequest TimerIoR;
struct Device *TimerBase=NULL;
ULONG CLK_P_SEC;
struct EClockVal gClock;
int dt_time = 0;
ULONG start_clock = 0;
ULONG prev_g_clock = 0;

int InitTimerDevice(void)
{
  if (OpenDevice(TIMERNAME, UNIT_ECLOCK, &TimerIoR , TR_GETSYSTIME) != 0)
  {
    printf("Unable to open Timer.device");
    TimerBase = 0;
    return 0;
  }

  TimerBase = TimerIoR.io_Device;

  return 1;
}

/*
  Sets the start of the global clock.
*/
void TimeInitGClock(void)
{
  CLK_P_SEC = ReadEClock(&gClock);
  start_clock = gClock.ev_lo;
  prev_g_clock = start_clock;
}

/*
  Computes a fixed point 'dt time'.
*/
int GetDeltaTime(void)
{
  CLK_P_SEC = ReadEClock(&gClock);

  dt_time = (int)(gClock.ev_lo - prev_g_clock);
  dt_time = ((dt_time << 10) / CLK_P_SEC);

  if (dt_time < 1)
    dt_time = 1;

  prev_g_clock = gClock.ev_lo;

  return dt_time;
}

/*
  Returns a fixed point global clock.
*/
ULONG TimeGetGClock(void)
{  
  CLK_P_SEC = ReadEClock(&gClock) >> 4;
  // printf("%i %i\n", gClock.ev_hi, gClock.ev_lo);
  return ((gClock.ev_lo - start_clock) << 4) / CLK_P_SEC;
}