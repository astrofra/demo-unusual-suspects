#include <exec/types.h>

extern struct IORequest TimerIoR;
extern struct Device *TimerBase;
extern ULONG CLK_P_SEC;
extern struct EClockVal gClock;
extern int dt_time;
extern ULONG start_clock;
extern ULONG prev_g_clock;

extern int InitTimerDevice(void);
extern void TimeInitGClock(void);
extern int GetDeltaTime(void);
extern ULONG TimeGetGClock(void);