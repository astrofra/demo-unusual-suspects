/*
    Unusual Suspects 
    Main program 
*/

#include "includes.prl"

#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <devices/keyboard.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/videocontrol.h>
#include <clib/timer_protos.h>
#include <graphics/sprite.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <clib/graphics_protos.h>

#include "ptreplay.h"
#include "ptreplay_protos.h"
#include "ptreplay_pragmas.h"

#include "common.h"
#include "protos.h"

#include "Assets/cosine_table.h"
#include "Assets/object_cube.h"
#include "Assets/object_amiga.h"
#include "Assets/object_face_00.h"
#include "Assets/object_spiroid.h"

#include "Assets/misc_palettes.h"
#include "Assets/faces_palettes.h"
#include "Assets/faces_all_palettes.h"
#include "Assets/fonts.h"
#include "Assets/audio_sync.h"

#include "3d_routines.h"
#include "bitmap_routines.h"
#include "copper_routines.h"
#include "font_routines.h"
#include "demo_strings.h"

static void disp_fade_in(UWORD *fadeto, SHORT pal_len);
static void disp_fade_out(UWORD *fadeFrom, SHORT pal_len);
static void disp_fade_setpalette(SHORT pal_len);
void disp_clear(struct RastPort *rp);
void disp_clear_bb_only(struct RastPort *rp);
void full_clear(struct RastPort *rp);
void init_clear_bb(void);
void reset_disp_swap(void);
void disp_swap(void);

void SequenceDemoTitle(void);
void Sequence3DRotation(int duration_sec);
void SequenceDisplaySuspectProfile(int suspect);
void dots_doit(UWORD *pal);
void writer_doit(UBYTE *wrText);
void scroll_doit(void);
void mandel(PLANEPTR scrMem);
#pragma regcall(mandel(a0))

extern struct BitMap theBitMap;
extern struct BitMap theBitMap_4bpl;
extern struct BitMap theBitMap_3bpl;
extern struct BitMap theBitMap_2bpl;
extern struct BitMap theBitMap_1bpl;
extern struct RastPort theRP;
extern struct RastPort theRP_3bpl;
extern struct RastPort theRP_2bpl;
extern struct RastPort theRP_1bpl;

extern struct Custom custom;
extern struct CIA ciaa, ciab;
extern PLANEPTR theRaster;
extern struct Screen *mainScreen;
extern struct DosLibrary *DOSBase;
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library *PTReplayBase;
extern struct ViewPort *mainVP;

extern struct obj_3d o;
extern int *verts_tr;

#define DEBUG_CONSOLE_ENABLED 0

/* Keyboard device */
struct MsgPort  *KeyMP;         /* Pointer for Message Port */
struct IOStdReq *KeyIO;         /* Pointer for I/O request */
UBYTE    *keyMatrix;
#define KEY_MATRIX_SIZE 16

/* Music */
struct Module *theMod;

/* Double Buffer */
BOOL  swapFlag = FALSE;

long  frame = 0,
      frameOffset = 0;

/* Palettes */
UWORD incr[32][3];
UWORD col[32][3];

/***** Global functions & data *****/
extern struct Library *SysBase;
struct Task *myTask;
BYTE oldPri;
PLANEPTR pic;
UBYTE *mod = NULL;

struct BitMap *bitmap_background,
              *bitmap_tmp,
              *bitmap_font,
              *bitmap_font_dark,
              *bitmap_video_noise,
              *bitmap_next_face;

/*  2D bounding box
    limits the surface to be cleared
*/
int   drawn_min_x = 512, drawn_min_y = 512,
      drawn_max_x = -1, drawn_max_y = -1;

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

/*
  Dispatch system
*/
int (*dispatch_func_ptr)(int);

int  DispatchFX(void)
{
  if (dispatch_func_ptr != dispatch_func_ptr)
  {

  }

  return(0);
}

/*
      Audio sync.
*/
int  ModuleGetSyncValue(void)
{
  int audio_clock, sync_value;
  audio_clock = TimeGetGClock();
  audio_clock *= AUDIO_SYNC_FREQ;
  audio_clock >>= 8;

  while (audio_clock > AUDIO_SYNC_REC_COUNT)
    audio_clock -= AUDIO_SYNC_REC_COUNT;

  sync_value = audio_sync[audio_clock];

  SetRGB4(&mainScreen->ViewPort, 0, ((sync_value >> 6) & 3) * 2, ((sync_value >> 6) & 3) * 2, ((sync_value >> 6) & 3) * 2);

  return 0;
}

/*Switch on the low-pass filter */
void filter_on(void)
{
   *((char *)0x0bfe001)&=0xFD;
}

/*Switch off the low-pass filter*/
void filter_off(void)
{
   *((char *)0x0bfe001)|=0x02;
}

/*Write a message to the CLI*/
void WriteMsg(char *errMsg)
{
   Write(Output(),errMsg,strlen(errMsg));
}

/*  Demo exits  */
void  ForceDemoClose(void)
{
  /*  Free the audio module */

  if (mod != NULL)
  {
    PTStop(theMod);
    PTFreeMod(theMod);
    FreeMem(mod, 83488);
  }

  /* Free the transformed vertex buffer */
  Delete3DVertexList();

  // Permit();
  SetTaskPri(myTask, oldPri);

  /*  Close the keyboard device */
  if (!(CheckIO((struct IORequest *)KeyIO)))
    AbortIO((struct IORequest *)KeyIO);   //  Ask device to abort request, if pending 
  // WaitIO((struct IORequest *)KeyIO);   /* Wait for abort, then clean up */
  CloseDevice((struct IORequest *)KeyIO);
  FreeMem(keyMatrix,KEY_MATRIX_SIZE);
  
  /* Close opened resources */
  FREE_BITMAP(bitmap_background);
  FREE_BITMAP(bitmap_tmp);
  FREE_BITMAP(bitmap_font);
  FREE_BITMAP(bitmap_font_dark);
  FREE_BITMAP(bitmap_video_noise);
  FREE_BITMAP(bitmap_next_face);

  init_close_video();
  init_close_libs();

  // ON_SPRITE;
  exit(0);
}

/*  Test is mouse button was pressed */
//  TODO : make the test os friendly!
void sys_check_abort(void)
{
  KeyIO->io_Command=KBD_READMATRIX;
  KeyIO->io_Data=(APTR)keyMatrix;
  KeyIO->io_Length = SysBase->lib_Version >= 36 ? KEY_MATRIX_SIZE : 13;
  DoIO((struct IORequest *)KeyIO);

//   printf("%i", (int)(keyMatrix[0x45/8] & (0x20)));
  if (keyMatrix[0x45/8] & (0x20))
    ForceDemoClose();

  // ModuleGetNormalizedPosition();
}

/*  Custom delay function */
int fVBLDelay(int _sec)
{
  int _count;

  for (_count = 0; _count < _sec; _count++)
  {
    GetDeltaTime();
    WaitTOF();
    DispatchFX();
    sys_check_abort();
    ModuleGetSyncValue();
  }

  return(0);
}

int InitKeyboard(void)
{
    if (KeyMP=CreatePort(NULL,NULL))
      if (KeyIO=(struct IOStdReq *)CreateExtIO(KeyMP,sizeof(struct IOStdReq)))
        if (OpenDevice( "keyboard.device",NULL,(struct IORequest *)KeyIO,NULL))
        {
          printf("keyboard.device did not open\n");
          return(0);
        }
        else
        if (!(keyMatrix=AllocMem(KEY_MATRIX_SIZE,MEMF_PUBLIC|MEMF_CLEAR)))
        {
          printf("Cannot allocate keyboard buffer\n");
          return(0);
        }
}

#define COLOR_MAKE_LIGHTER(COL,QT) (((COL & 0xF00) + (QT << 8)) & 0xF00) | (((COL & 0x0F0) + (QT << 4)) & 0x0F0) | (((COL & 0x00F) + QT) & 0x00F)

UWORD ColorMakeLighter(UWORD color_in, int dt)
{
  int r,g,b;
  r = (color_in & 0xF00) >> 8;
  g = (color_in & 0x0F0) >> 4;
  b = color_in & 0x00F;
  r += dt;
  g += (dt / 2);
  b += (dt * 2);
  if (r > 15)
    r = 15;
  if (g > 15)
    g = 15;
  if (b > 15)
    b = 15;

  return ((UWORD)((r << 8) | (g << 4) | b));
}

UWORD ColorMakeDarker(UWORD color_in, int dt)
{
  int r,g,b;
  r = (color_in & 0xF00) >> 8;
  g = (color_in & 0x0F0) >> 4;
  b = color_in & 0x00F;
  r -= dt;
  g -= (dt / 2);
  b -= (dt / 3);
  if (r < 0)
    r = 0;
  if (g < 0)
    g = 0;
  if (b < 0)
    b = 0;

  return ((UWORD)((r << 8) | (g << 4) | b));
}

/* Main program entry point */
int main(void)
{
  bitmap_background = NULL;
  bitmap_tmp = NULL;
  bitmap_font = NULL;
  bitmap_font_dark = NULL;
  bitmap_video_noise = NULL;
  bitmap_next_face = NULL;

  WriteMsg("Amiga C demo^Mandarine/Mankind 2014.\n");

  dispatch_func_ptr = NULL;

  InitKeyboard();
  InitTimerDevice();

  srand((ciaa.ciatodmid << 8) | ciaa.ciatodlow);
  /* Open all needed resources */
  if (!init_open_libs())
  {
    init_close_libs();
    return (10);
  }

  if (!Init32ColorsScreen())
  {
    init_close_video();
    return (10);
  }

  Prepare3DVertexList();

  myTask = FindTask(NULL);
  oldPri = SetTaskPri(myTask, 127);
  // Forbid();

  /*
    Load common assets
  */
  bitmap_tmp = load_as_bitmap((UBYTE *)"assets/demo-title.bin", 28000, 320, 140, 5);

  mod = load_getmem((UBYTE *)"assets/module.bin", 83488);
  theMod = PTSetupMod((APTR)mod);
  PTPlay(theMod);

  /*
    Set the start of the global demo clock
  */
  TimeInitGClock();

  Init32ColorsScreen();
  full_clear(NULL);

  SequenceDemoTitle();

  bitmap_background = load_as_bitmap((UBYTE *)"assets/background1.bin", 40 * 5 * 256, 320, 256, 5);
  bitmap_font = load_as_bitmap((UBYTE *)"assets/future_font.bin", 5700, 595, 15, 5);
  bitmap_font_dark = load_as_bitmap((UBYTE *)"assets/future_font-dark.bin", 5700, 595, 15, 5);
  bitmap_video_noise = load_as_bitmap((UBYTE *)"assets/video-noise.bin", 5120, 71, 128, 4);
  bitmap_next_face = load_as_bitmap((UBYTE *)"assets/face_01.bin", 3440, 80, 86, 4);

  fVBLDelay(350);
  full_clear(NULL);

  LoadRGB4(mainVP, meshDisplayRGB4, 8);
  PREPARE_3D_MESH(o, object_cube_verts, object_cube_faces, 256, 256, 0);
  Sequence3DRotation(5);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(0);
  fVBLDelay(25);

  LoadRGB4(mainVP, meshDisplayRGB4, 8);
  PREPARE_3D_MESH(o, object_spiroid_verts, object_spiroid_faces, 256, 160, 0);
  Sequence3DRotation(5);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(1);
  fVBLDelay(25);

  LoadRGB4(mainVP, meshDisplayRGB4, 8);
  PREPARE_3D_MESH(o, object_face_00_verts, object_face_00_faces, 800, 256, 0);
  Sequence3DRotation(5);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(2);
  fVBLDelay(25);

  LoadRGB4(mainVP, meshDisplayRGB4, 8);
  PREPARE_3D_MESH(o, object_face_00_verts, object_face_00_faces, 800, 256, 1);
  Sequence3DRotation(5);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(3);
  fVBLDelay(25);

  LoadRGB4(mainVP, meshDisplayRGB4, 8);
  PREPARE_3D_MESH(o, object_amiga_verts, object_amiga_faces, 800, 512, 0);
  Sequence3DRotation(5);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(4);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(5);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(6);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(7);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(8);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(9);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(10);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(11);
  fVBLDelay(10);  

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(12);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(13);
  fVBLDelay(10);

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(14);
  fVBLDelay(10);              

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(15);
  fVBLDelay(10);              

  reset_disp_swap();
  disp_clear(NULL);
  SequenceDisplaySuspectProfile(16);
  fVBLDelay(10);              

  disp_fade_out(pal7, 16);
  reset_disp_swap();
  disp_clear(NULL);

  /* 
    Unusual faces 
    Part #1 
  */
  InitEHBScreen();
  disp_clear(NULL);
  LoadRGB4(mainVP, faces_all_PaletteRGB4, 32);

  pic = load_getmem((UBYTE *)"assets/faces_all.bin", 86400);

  WaitTOF();           
  disp_swap();
  disp_interleaved_st_format(pic, &theBitMap, 320, 180, 0, 8, 32 + frameOffset, 6);
  LoadRGB4(mainVP, faces_all_PaletteRGB4, 32);
  CreateHigheSTColorCopperList(0, 32);
  WaitTOF();           
  disp_swap();

  fVBLDelay(250);

  /* 
    Unusual faces
    Part #2
  */
  WaitTOF();           
  disp_swap();
  disp_interleaved_st_format(pic, &theBitMap, 320, 180, 180, 8, 32 + frameOffset, 6);
  LoadRGB4(mainVP, faces_all_PaletteRGB4, 32);
  CreateHigheSTColorCopperList(180, 32);
  WaitTOF();
  disp_swap();

  FreeMem(pic, 86400);

  fVBLDelay(250);

  full_clear(NULL);

  DeleteCopperList();

  // writer_doit((UBYTE *) "A multitasking#"
  //                       "syncing töntro#"
  //                       "Coded in pure C!!!#");
  
  /* Close opened resources */
  ForceDemoClose();
  return (0);
}

/*********** FADER ***************/

/* Fade palette from all black to specified colors */
static void disp_fade_in(UWORD *fadeto, SHORT pal_len)
{
  SHORT i, p;

  for (i = 0; i < 3; i ++)
    for (p = 0; p < pal_len; p ++)
      col[p][i] = 0;

  for (i = 0; i < pal_len; i ++)
  {
    incr[i][0] = ((fadeto[i] << 4) & 0xf000) / 15;
    incr[i][1] = ((fadeto[i] << 8) & 0xf000) / 15;
    incr[i][2] = ((fadeto[i] << 12) & 0xf000) / 15;
  }

  disp_fade_setpalette(pal_len);

  for (i = 1; i < 16; i ++)
  {
    for (p = 0; p < pal_len; p ++)
    {
      col[p][0] += incr[p][0];
      col[p][1] += incr[p][1];
      col[p][2] += incr[p][2];
    }

    WaitTOF();
    disp_fade_setpalette(pal_len);

    sys_check_abort();
  }

  WaitTOF();

  LoadRGB4(mainVP, fadeto, pal_len);
}

/* Fade palette from colors to all black */
static void disp_fade_out(UWORD *fadeFrom, SHORT pal_len)
{
  UWORD i, p;

  for (i = 0; i < pal_len; i ++)
  {
    col[i][0] = (fadeFrom[i] & 0x0f00) << 4;
    col[i][1] = (fadeFrom[i] & 0x00f0) << 8;
    col[i][2] = (fadeFrom[i] & 0x000f) << 12;
    incr[i][0] = col[i][0] / 15;
    incr[i][1] = col[i][1] / 15;
    incr[i][2] = col[i][2] / 15;
  }

  for (i = 1; i < 16; i ++)
  {
    for (p = 0; p < 16; p ++)
    {
      col[p][0] -= incr[p][0];
      col[p][1] -= incr[p][1];
      col[p][2] -= incr[p][2];
    }
    WaitTOF();
    disp_fade_setpalette(pal_len);

    sys_check_abort();
  }

  WaitTOF();

  for (i = 0; i < pal_len; i ++)
    SetRGB4(mainVP, i, 0, 0, 0);
}

/* Set palette registers to currents colors while fading */
static void disp_fade_setpalette(SHORT pal_len)
{
  UWORD i, temp, pal[32];

  for (i = 0; i < pal_len; i ++)
  {
    pal[i] = (col[i][0] & 0xf000) >> 4;
    temp = (col[i][1] & 0xf000) >> 8;
    pal[i] |= temp;
    temp = (col[i][2] & 0xf000) >> 12;
    pal[i] |= temp;
  }
  LoadRGB4(mainVP, pal, pal_len);
}

/***************** DISPLAY SUPPORT ********************/

void disp_clear(struct RastPort *rp)
{
  // SetRast(&theRP, 0);
  if (rp == NULL)
    rp = &theRP;

  SetAPen(rp, 0);
  RectFill(rp, 0, frameOffset, 320 - 1, frameOffset + (SCR_HEIGHT / 2) - 1);
}

void init_clear_bb(void)
{
  drawn_min_x = 512;
  drawn_min_y = 1024;
  drawn_max_x = -1;
  drawn_max_y = -1;
}

void disp_clear_bb_only(struct RastPort *rp)
{
  // printf("min_x = %d, min_y = %i, max_x = %i, max_y = %i\n", drawn_min_x, drawn_min_y, drawn_max_x, drawn_max_y);

  if (drawn_max_x > 0)
  {
    if (rp == NULL)
      rp = &theRP;

    SetAPen(rp, 0);
    RectFill(rp, drawn_min_x, drawn_min_y + frameOffset, drawn_max_x, drawn_max_y + frameOffset);
  }
}

void full_clear(struct RastPort *rp)
{
  if (rp == NULL)
    rp = &theRP;

  SetRast(rp, 0);
  // SetAPen(rp, 0);
  // RectFill(rp, 0, 0, 320 - 1, SCR_HEIGHT - 1);
}

void reset_disp_swap(void)
{
  frame = 0;
  frameOffset = 0;
  mainVP->RasInfo->RyOffset = frameOffset;
  ScrollVPort(mainVP);
}

void disp_swap(void)
{
      mainVP->RasInfo->RyOffset = frameOffset;
      ScrollVPort(mainVP);

      frame ^= 1;
      frameOffset = frame * 256;
}

/***************** SHADEDOTS ********************/

void dots_doit(UWORD *pal)
{
  WORD x[20], y[20];
  WORD i, j, h, c;

  LoadRGB4(mainVP, pal, 16);
  for (i = 0; i < 20; i ++)
  {
    x[i] = rand() % 320;
    y[i] = rand() % 256;
  }

  for (j = 0; j < 1500; j ++)
  {
    for (i = 0; i < 15; i ++)
    {
      h = rand() % 6;
      switch (h)
      {
        case 0:
          x[i] ++;
          break;

        case 1:
        case 4:
          x[i] --;
          break;

        case 2:
        case 5:
          y[i] ++;
          break;

        case 3:
          y[i] --;
          break;
      }
      if (x[i] > 319) x[i] -= 320;
      if (x[i] < 0) x[i] += 320;
      if (y[i] > 255) y[i] -= 256;
      if (y[i] < 0) y[i] += 256;

      c = ReadPixel(&theRP_3bpl, x[i], y[i]);
      c += 1; /* ((rand() % 2) << 1) - 1; */
      if (c > 7) c = 2;
      SetAPen(&theRP_3bpl, c);
      WritePixel(&theRP_3bpl, x[i], y[i]);
    }
    DispatchFX();
    sys_check_abort();
  }
}

/**************** WRITER *******************/

void writer_doit(UBYTE *wrText)
{
  UBYTE *currChar;
  UWORD y_base = 32;
  UWORD x = 0, y = y_base;
  UWORD y_line_h = 32;

  currChar = wrText;
  SetDrMd(&theRP_2bpl, JAM1);
  while(*currChar)
  {
    while(*currChar != '#')
    {
      SetAPen(&theRP_2bpl, 2);
      Move(&theRP_2bpl, x - 1, y);
      Text(&theRP_2bpl, currChar, 1);
      Move(&theRP_2bpl, x + 1, y);
      Text(&theRP_2bpl, currChar, 1);
      x += TextLength(&theRP_2bpl, currChar, 1);
      currChar ++;
    }
    currChar ++;
    y += y_line_h;
    x = 0;
  }

  x = 0;
  y = y_base;

  currChar = wrText;
  SetDrMd(&theRP_2bpl, JAM1);
  while(*currChar)
  {
    while(*currChar != '#')
    {
      // SetAPen(&theRP_2bpl, 2);
      // Move(&theRP_2bpl, x - 4, y);
      // Text(&theRP_2bpl, currChar, 1);
      SetAPen(&theRP_2bpl, 1);
      Move(&theRP_2bpl, x, y);
      Text(&theRP_2bpl, currChar, 1);
      x += TextLength(&theRP_2bpl, currChar, 1);
      currChar ++;
    }
    currChar ++;
    y += y_line_h;
    x = 0;
  }

  disp_fade_in(pal1, 16);
  fVBLDelay(400);
  disp_fade_out(pal1, 16);
}

/**************** SCROLLTEXT *********************/

void scroll_doit(void)
{
  PLANEPTR font, pic;
  UBYTE *currChar;
  static struct BitMap fontMap;
  static UWORD offs[] =
  { 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 128, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 64, 192,
    288, 144, 256, 192, 192, 192, 192, 192,
    32, 192, 192, 192, 224, 96, 192, 192,
    256, 96, 0, 144, 32, 144, 64, 144,
    96, 144, 128, 144, 160, 144, 192, 144,
    224, 144, 256, 144, 96, 192, 0, 192,
    192, 192, 192, 192, 192, 192, 160, 192,
    192, 192, 0, 0, 32, 0, 64, 0,
    96, 0, 128, 0, 160, 0, 192, 0,
    224, 0, 256, 0, 0, 48, 32, 48,
    64, 48, 96, 48, 128, 48, 160, 48,
    192, 48, 224, 48, 256, 48, 0, 96,
    32, 96, 64, 96, 96, 96, 224, 192,
    128, 96, 160, 96, 192, 96, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 0, 0, 32, 0, 64, 0,
    96, 0, 128, 0, 160, 0, 192, 0,
    224, 0, 256, 0, 0, 48, 32, 48,
    64, 48, 96, 48, 128, 48, 160, 48,
    192, 48, 224, 48, 256, 48, 0, 96,
    32, 96, 64, 96, 96, 96, 224, 192,
    128, 96, 160, 96, 192, 96, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192 };
  static UBYTE scrText[] = "welcome to the little scrollthingy... "
  // "first the serious stuff :       "
  // "q: what's another name for the 'intel inside' sticker they put on pentiums?          "
  // "a: the warning label.         "
  // "now we'll greet some people:       tcc, chaos pm, lcg, exceed, inventors of coca cola, "
  // "the staff at this great party, kyrcman's microwave oven (and the man ;), " 
  "motorola inc. (wod else ;) and the snubbe...   stay tuned for more pentium fun...                      ";

  font = load_getmem((UBYTE *)"assets/scrollfont.bin", 80 * 256);
  InitBitMap(&fontMap, 2, 320, 256);
  fontMap.Planes[0] = font;
  fontMap.Planes[1] = font + 40 * 256;
  pic = load_getmem((UBYTE *)"assets/gradient.bin", 40 * 256 * 4); // load_getmem((UBYTE *)"assets/panther.bin", 34 * 167 * 4);
  // disp_whack(pic, &theBitMap, 320, 256, 0, 0, 4);
  currChar = scrText;

  disp_fade_in(gradientPaletteRGB4, 16); //pal5);
  fVBLDelay(100);
  
  while (*currChar)
  {
    BltBitMap(&fontMap, offs[(*currChar) << 1], offs[((*currChar) << 1) + 1],
    &theBitMap_3bpl, 320, 208, 32, 48, 0xc0, 0xff, NULL);
    currChar ++;

    WaitTOF();
    ScrollRaster(&theRP_2bpl, 8, 0, 0, 208, 383, 255);
    WaitTOF();
    ScrollRaster(&theRP_2bpl, 8, 0, 0, 208, 383, 255);
    WaitTOF();
    ScrollRaster(&theRP_2bpl, 8, 0, 0, 208, 383, 255);
    WaitTOF();
    ScrollRaster(&theRP_2bpl, 8, 0, 0, 208, 383, 255);

    DispatchFX();
    sys_check_abort();
  }

  disp_fade_out(gradientPaletteRGB4, 16); //pal5);  
  // FreeMem(font, 80 * 256);
  // FreeMem(pic, 40 * 256 * 4);
}

void SequenceDemoTitle(void)
{
  int i, j;

  LoadRGB4(mainVP, whitePaletteRGB4, 32);

  SetAPen(&theRP, 15);

  for(i = 1, j = 1; i < 70; i += j, j++)
  {
    WaitTOF();
    RectFill(&theRP, 0, 128 - i, 319, 128 + i);
    if (i == 1) WaitTOF();
  }

  WaitTOF();
  WaitTOF();
  BLIT_BITMAP_S(bitmap_tmp, &theBitMap, 320, 140, 0, (256 - 140) / 2);

  disp_fade_in(demo_title_PaletteRGB4, 32);
  LoadRGB4(mainVP, demo_title_PaletteRGB4, 32);
  FREE_BITMAP(bitmap_tmp);
}

void Sequence3DRotation(int duration_sec)
{
  int abs_frame_idx = 0,
      m_scale_x;

  ULONG seq_start_clock, elapsed_clock = 0;
  duration_sec <<= 8;

  seq_start_clock = TimeGetGClock();

  while(elapsed_clock <= duration_sec)
  {
    elapsed_clock = TimeGetGClock() - seq_start_clock;

    if (elapsed_clock < (1 << 7))
      m_scale_x = (24 * QMAX(((1 << 7) - elapsed_clock), 0)) >> 8;
    else
    {
      if (elapsed_clock > duration_sec - (1 << 7))
        m_scale_x = (24 * QMAX(elapsed_clock - (duration_sec - (1 << 7)), 0)) >> 8;
      else
        m_scale_x = 0;
    }

    abs_frame_idx += dt_time;
    GetDeltaTime();
    WaitTOF();           
    disp_swap();
    disp_clear(&theRP_2bpl);
    // disp_clear_bb_only(&theRP_2bpl);
    // init_clear_bb();
    Draw3DMesh((abs_frame_idx >> 4)&(COSINE_TABLE_LEN - 1), (abs_frame_idx >> 3)&(COSINE_TABLE_LEN - 1), frameOffset, m_scale_x);
    sys_check_abort();
    ModuleGetSyncValue();
  }
}

void SequenceDisplaySuspectProfile(int suspect_index)
{
  int i;
  /*  Text dispatch */
  UBYTE *c_desc_str;
  UBYTE *c_face;
  UWORD *c_pal;

  switch(suspect_index)
  {
    case 0: c_desc_str = (UBYTE *)DESC_CHAR_STR(0); c_face = (UBYTE *)"assets/face_02.bin"; c_pal = face_01PaletteRGB4; break;
    case 1: c_desc_str = (UBYTE *)DESC_CHAR_STR(1); c_face = (UBYTE *)"assets/face_03.bin"; c_pal = face_02PaletteRGB4; break;
    case 2: c_desc_str = (UBYTE *)DESC_CHAR_STR(2); c_face = (UBYTE *)"assets/face_04.bin"; c_pal = face_03PaletteRGB4; break;
    case 3: c_desc_str = (UBYTE *)DESC_CHAR_STR(3); c_face = (UBYTE *)"assets/face_05.bin"; c_pal = face_04PaletteRGB4; break;
    case 4: c_desc_str = (UBYTE *)DESC_CHAR_STR(4); c_face = (UBYTE *)"assets/face_06.bin"; c_pal = face_05PaletteRGB4; break;
    case 5: c_desc_str = (UBYTE *)DESC_CHAR_STR(5); c_face = (UBYTE *)"assets/face_07.bin"; c_pal = face_06PaletteRGB4; break;
    case 6: c_desc_str = (UBYTE *)DESC_CHAR_STR(6); c_face = (UBYTE *)"assets/face_08.bin"; c_pal = face_07PaletteRGB4; break;
    case 7: c_desc_str = (UBYTE *)DESC_CHAR_STR(7); c_face = (UBYTE *)"assets/face_09.bin"; c_pal = face_08PaletteRGB4; break;
    case 8: c_desc_str = (UBYTE *)DESC_CHAR_STR(8); c_face = (UBYTE *)"assets/face_10.bin"; c_pal = face_09PaletteRGB4; break;
    case 9: c_desc_str = (UBYTE *)DESC_CHAR_STR(9); c_face = (UBYTE *)"assets/face_17.bin"; c_pal = face_10PaletteRGB4; break;
    case 10: c_desc_str = (UBYTE *)DESC_CHAR_STR(16); c_face = (UBYTE *)"assets/face_11.bin"; c_pal = face_17PaletteRGB4; break;
    case 11: c_desc_str = (UBYTE *)DESC_CHAR_STR(10); c_face = (UBYTE *)"assets/face_12.bin"; c_pal = face_11PaletteRGB4; break;
    case 12: c_desc_str = (UBYTE *)DESC_CHAR_STR(11); c_face = (UBYTE *)"assets/face_13.bin"; c_pal = face_12PaletteRGB4; break;
    case 13: c_desc_str = (UBYTE *)DESC_CHAR_STR(12); c_face = (UBYTE *)"assets/face_14.bin"; c_pal = face_13PaletteRGB4; break;
    case 14: c_desc_str = (UBYTE *)DESC_CHAR_STR(13); c_face = (UBYTE *)"assets/face_15.bin"; c_pal = face_14PaletteRGB4; break;
    case 15: c_desc_str = (UBYTE *)DESC_CHAR_STR(14); c_face = (UBYTE *)"assets/face_16.bin"; c_pal = face_15PaletteRGB4; break;
    case 16: c_desc_str = (UBYTE *)DESC_CHAR_STR(15); c_face = NULL; c_pal = face_16PaletteRGB4; break;
    default: return;
  }

  BLIT_BITMAP_S(bitmap_background, &theBitMap, 320, 256, 0, 0);
  LoadRGB4(mainVP, background1PaletteRGB4, 32);
  fVBLDelay(10);

  /*  Clear the portrait area */
  SetAPen(&theRP, 0);
  WaitTOF();
  RectFill(&theRP, 42, frameOffset + 55, 42 + 72, 55 + 87);

  LoadRGB4(mainVP, videoPaletteRGB4, 16);

  /*  Switched on tube FX */

  SetAPen(&theRP, 15);
  WaitTOF();
  RectFill(&theRP, 42, frameOffset + 55 + 40, 42 + 72, 55 + 87 - 40);
  WaitTOF();
  RectFill(&theRP, 42, frameOffset + 55 + 32, 42 + 72, 55 + 87 - 32);

  for (i = 0; i < 5; i++)
  {
    WaitTOF();
    BltBitMap(bitmap_video_noise, 0, (i << 2) + ((8 - i) << 1), &theBitMap, 43, 56 + ((8 - i) << 2), 71, 86 - ((8 - i) << 3), 0xC0, 0xFF, NULL);
    if (i == 0)   RectFill(&theRP, 42, frameOffset + 55 + 40, 42 + 72, 55 + 87 - 40);

    ModuleGetSyncValue();
  }

  WaitTOF();
  BltBitMap(bitmap_video_noise, 0, 128 - 90, &theBitMap, 43, 56, 71, 86, 0xC0, 0xFF, NULL);

  SetAPen(&theRP, 0);
  WaitTOF();
  RectFill(&theRP, 42, frameOffset + 55, 42 + 72, 55 + 87);

  /*  Load the portrait's palette */
  LoadRGB4(mainVP, c_pal, 16);

  /*  Draw the portrait */
  BLIT_BITMAP_S(bitmap_next_face, &theBitMap, 71, 86, 43, 56);
  BltBitMap(bitmap_background, 36, 140, &theBitMap, 36, 140, 9, 8, 0xC0, 0xFF, NULL);

  fVBLDelay(10);

  /*  Write the profile description */
  font_writer_blit(bitmap_font, bitmap_font_dark, &theBitMap, (const char *)&future_font_glyph_array, (const int *)&future_font_x_pos_array, 124, 63, c_desc_str);

  FREE_BITMAP(bitmap_tmp);

  if (c_face != NULL)
    bitmap_next_face = load_as_bitmap(c_face, 3440, 80, 86, 4);

  for(i = 0; i < 250; i++)
  {
    WaitTOF();
    ModuleGetSyncValue();
  }

  WaitTOF();
  disp_fade_in(blackPaletteRGB4, 32);

  disp_clear(NULL);
}
