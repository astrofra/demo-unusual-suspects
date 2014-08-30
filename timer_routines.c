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

struct timerequest *TimerIO;
struct MsgPort *TimerMP;
struct Message *TimerMSG;

int dt_time = 0;
ULONG start_clock = 0;
ULONG prev_g_clock = 0;

ULONG timer_secs, timer_mics;

int InitTimerDevice(void)
{
  // if (OpenDevice(TIMERNAME, UNIT_ECLOCK, &TimerIoR , TR_GETSYSTIME) != 0)
  // {
  //   printf("Unable to open Timer.device");
  //   TimerBase = 0;
  //   return 0;
  // }

  // TimerBase = TimerIoR.io_Device;

  TimerMP = CreatePort(0,0);
  TimerIO = (struct timerequest *)CreateExtIO(TimerMP,sizeof(struct timerequest));
  OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)TimerIO, 0L);

  return 1;
}

/*
  Sets the start of the global clock.
*/
void TimeInitGClock(void)
{
  ULONG tmp_clock;
  // CLK_P_SEC = ReadEClock(&gClock);
  // start_clock = gClock.ev_lo;
  // prev_g_clock = start_clock;

  TimerIO->tr_node.io_Command = TR_GETSYSTIME;
  DoIO((struct IORequest *) TimerIO);

  timer_mics=TimerIO->tr_time.tv_micro / 20000;
  timer_secs=TimerIO->tr_time.tv_secs * 50;

  tmp_clock = timer_secs + timer_mics;

  start_clock = tmp_clock;
  prev_g_clock = start_clock;
}

/*
  Computes a fixed point 'dt time'.
*/
int GetDeltaTime(void)
{
  ULONG tmp_clock;
  // CLK_P_SEC = ReadEClock(&gClock);

  TimerIO->tr_node.io_Command = TR_GETSYSTIME;
  DoIO((struct IORequest *) TimerIO);

  timer_mics=TimerIO->tr_time.tv_micro / 20000;
  timer_secs=TimerIO->tr_time.tv_secs * 50;

  tmp_clock = timer_secs + timer_mics;

  dt_time = tmp_clock - prev_g_clock;

  if (dt_time < 1)
    dt_time = 1;

  prev_g_clock = tmp_clock;

  return dt_time;
}

/*
  Returns a fixed point global clock.
*/
ULONG TimeGetGClock(void)
{  
  ULONG tmp_clock;
  // CLK_P_SEC = ReadEClock(&gClock) >> 4;

  TimerIO->tr_node.io_Command = TR_GETSYSTIME;
  DoIO((struct IORequest *) TimerIO);

  timer_mics=TimerIO->tr_time.tv_micro / 20000;
  timer_secs=TimerIO->tr_time.tv_secs * 50;

  tmp_clock = timer_secs + timer_mics;
  // printf("os3.0clock = %i, tmp_clock = %i\n", ((gClock.ev_lo - start_clock) << 4) / CLK_P_SEC, tmp_clock);
  // printf("tmp_clock = %i\n", tmp_clock);
   
  // // printf("%i %i\n", gClock.ev_hi, gClock.ev_lo);
  // return ((gClock.ev_lo - start_clock) << 4) / CLK_P_SEC;
  return tmp_clock;
}