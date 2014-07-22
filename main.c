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

#include "3d_routines.h"
#include "copper_routines.h"

static void disp_fade_in(UWORD *fadeto);
static void disp_fade_out(UWORD *fadeFrom);
static void disp_fade_setpalette(void);
void disp_clear(void);
void full_clear(void);
void reset_disp_swap(void);
void disp_swap(void);
extern void disp_whack(struct BitMap *src_BitMap, struct BitMap *dest_BitMap, UWORD width, UWORD height, UWORD x, UWORD y, UWORD depth);
extern void disp_interleaved_st_format(PLANEPTR data, struct BitMap *dest_BitMap, UWORD width, UWORD height, UWORD src_y, UWORD x, UWORD y, UWORD depth);

void dots_doit(UWORD *pal);
void writer_doit(UBYTE *wrText);
void scroll_doit(void);
extern PLANEPTR load_getmem(UBYTE *name, ULONG size);
extern struct BitMap *load_as_bitmap(UBYTE *name, ULONG byte_size, UWORD width, UWORD height, UWORD depth);
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
UWORD incr[16][3];
UWORD col[16][3];

/***** Global functions & data *****/
extern struct Library *SysBase;
struct Task *myTask;
BYTE oldPri;
PLANEPTR pic;
UBYTE *mod;

/*
  Delta time
*/
struct IORequest TimerIoR;
struct Device *TimerBase=NULL;
ULONG CLK_P_SEC;
struct EClockVal gClock;
int dt_time = 0;
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
  Returns a dt time factor.
*/
int GetDeltaTime(void)
{
  if (TimerBase != 0)
  {
    CLK_P_SEC = ReadEClock(&gClock);

    dt_time = (int)(gClock.ev_lo - prev_g_clock);
    dt_time = ((dt_time << 10) / CLK_P_SEC);
    // dt_time = dt_time >> 4;
    if (dt_time < 1)
      dt_time = 1;

    prev_g_clock = gClock.ev_lo;

    // printf("CLK_P_SEC = %i, dt_time = %i\n", CLK_P_SEC, dt_time);
  }

  return dt_time;
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
  PTStop(theMod);
  PTFreeMod(theMod);
  FreeMem(mod, 83488);
  Prepare2DVertexList();

  Permit();
  SetTaskPri(myTask, oldPri);

  /*  Close the keyboard device */
  if (!(CheckIO((struct IORequest *)KeyIO)))
    AbortIO((struct IORequest *)KeyIO);   //  Ask device to abort request, if pending 
  // WaitIO((struct IORequest *)KeyIO);   /* Wait for abort, then clean up */
  CloseDevice((struct IORequest *)KeyIO);
  FreeMem(keyMatrix,KEY_MATRIX_SIZE);
  
  /* Close opened resources */
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
  int frame_idx,
      abs_frame_idx = 0,
      m_scale_x;

  int scroll_y;

  PLANEPTR face;

  struct BitMap *tmp_bitmap;

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

  Prepare2DVertexList();

  // filter_off();

  myTask = FindTask(NULL);
  oldPri = SetTaskPri(myTask, 127);
  Forbid();

  // OFF_SPRITE;

  mod = load_getmem((UBYTE *)"assets/module.bin", 95430);
  theMod = PTSetupMod((APTR)mod);
  PTPlay(theMod);

  goto skipintro;

  /* Unusual faces part #1 */
  InitEHBScreen();
  disp_clear();
  LoadRGB4(mainVP, faces_all_PaletteRGB4, 32);

  pic = load_getmem((UBYTE *)"assets/faces_all.bin", 40 * 360 * 6);

  WaitTOF();           
  disp_swap();
  disp_interleaved_st_format(pic, &theBitMap, 320, 180, 0, 8, 32 + frameOffset, 6);
  LoadRGB4(mainVP, faces_all_PaletteRGB4, 32);
  CreateHigheSTColorCopperList(0, 32);
  WaitTOF();           
  disp_swap();

  fVBLDelay(5);

  WaitTOF();    
  disp_swap();
  disp_clear();

  /* Unusual faces part #2 */
  WaitTOF();           
  disp_swap();
  disp_interleaved_st_format(pic, &theBitMap, 320, 180, 180, 8, 32 + frameOffset, 6);
  LoadRGB4(mainVP, faces_all_PaletteRGB4, 32);
  CreateHigheSTColorCopperList(180, 32);
  WaitTOF();
  disp_swap();

  FreeMem(pic, 40 * 360 * 6);

  fVBLDelay(5);

  full_clear();

skipintro:;
  Init32ColorsScreen();
  DeleteCopperList();

  reset_disp_swap();

  tmp_bitmap = load_as_bitmap((UBYTE *)"assets/background1.bin", 40 * 5 * 256, 320, 256, 5);
  disp_whack(tmp_bitmap, &theBitMap, 320, 256, 0, 0, 5);
  LoadRGB4(mainVP, background1PaletteRGB4, 32);
  fVBLDelay(100);
  FreeBitMap(tmp_bitmap);

  tmp_bitmap = load_as_bitmap((UBYTE *)"assets/face_01.bin", 3440, 80, 86, 4);

  SetAPen(&theRP, 0);
  RectFill(&theRP, 48, frameOffset + 55, 48 + 70, 55 + 85);

  disp_whack(tmp_bitmap, &theBitMap_4bpl, 71, 86, 48, 55, 5);
  LoadRGB4(mainVP, face_01PaletteRGB4, 16);
  fVBLDelay(500);

  disp_clear();
  FreeBitMap(tmp_bitmap);

  Init16ColorsScreen();

  tmp_bitmap = load_as_bitmap((UBYTE *)"assets/demo-title.bin", 40 * 4 * 256, 320, 256, 4);
  disp_whack(tmp_bitmap, &theBitMap, 320, 256, 0, 0, 4);
  disp_fade_in(demo_title_PaletteRGB4);

  // CreateCopperList();
  FreeBitMap(tmp_bitmap);
  fVBLDelay(350);

  full_clear();

  PREPARE_3D_MESH(o, object_cube_verts, object_cube_faces, 256, 256, 0);

  m_scale_x = 24;
  for(frame_idx = 0; frame_idx < 256; frame_idx++)
  {
    if (frame_idx < 24)
      m_scale_x--;
    else
    if (frame_idx > 256 - 24)
      m_scale_x++;

    abs_frame_idx += dt_time;
    GetDeltaTime();
    WaitTOF();           
    disp_swap();
    disp_clear();
    Draw3DMesh((abs_frame_idx >> 4)&(COSINE_TABLE_LEN - 1), (abs_frame_idx >> 3)&(COSINE_TABLE_LEN - 1), frameOffset, m_scale_x);
    sys_check_abort();
  }

  PREPARE_3D_MESH(o, object_spiroid_verts, object_spiroid_faces, 256, 160, 0);

  m_scale_x = 24;
  for(frame_idx = 0; frame_idx < 256; frame_idx++)
  {
    if (frame_idx < 24)
      m_scale_x--;
    else
    if (frame_idx > 256 - 24)
      m_scale_x++;

    abs_frame_idx += dt_time;
    GetDeltaTime();
    WaitTOF();           
    disp_swap();
    disp_clear();
    Draw3DMesh((abs_frame_idx >> 4)&(COSINE_TABLE_LEN - 1), (abs_frame_idx >> 3)&(COSINE_TABLE_LEN - 1), frameOffset, m_scale_x);
    sys_check_abort();
  }

  PREPARE_3D_MESH(o, object_face_00_verts, object_face_00_faces, 800, 256, 1);

  m_scale_x = 24;
  for(frame_idx = 0; frame_idx < 256; frame_idx++)
  {
    if (frame_idx < 24)
      m_scale_x--;
    else
    if (frame_idx > 256 - 24)
      m_scale_x++;

    abs_frame_idx += dt_time;
    GetDeltaTime();
    WaitTOF();           
    disp_swap();
    disp_clear();
    Draw3DMesh((abs_frame_idx >> 4)&(COSINE_TABLE_LEN - 1), (abs_frame_idx >> 3)&(COSINE_TABLE_LEN - 1), frameOffset, m_scale_x);
    sys_check_abort();
  }

  PREPARE_3D_MESH(o, object_amiga_verts, object_amiga_faces, 800, 512, 0);

  m_scale_x = 24;
  for(frame_idx = 0; frame_idx < 256; frame_idx++)
  {
    if (frame_idx < 24)
      m_scale_x--;
    else
    if (frame_idx > 256 - 24)
      m_scale_x++;

    abs_frame_idx += dt_time;
    GetDeltaTime();
    WaitTOF();           
    disp_swap();
    disp_clear();
    Draw3DMesh((abs_frame_idx >> 4)&(COSINE_TABLE_LEN - 1), (abs_frame_idx >> 5)&(COSINE_TABLE_LEN - 1), frameOffset, m_scale_x);
    sys_check_abort();
  }

  // fVBLDelay(350);
  disp_fade_out(pal7);
  reset_disp_swap();
  disp_clear();
  FreeMem(pic, 40 * 4 * 256);

  // writer_doit((UBYTE *)"#####     GETTIN' TIRED OF...?#");
  // disp_clear();

  writer_doit((UBYTE *) "A multitasking#"
                        "syncing töntro#"
                        "Coded in pure C!!!#");

  disp_clear();
  fVBLDelay(50);
  dots_doit(pal2);
  fVBLDelay(50);
  disp_fade_out(pal2);
  disp_clear();

  writer_doit((UBYTE *) "Coding : Picnic#"
                        "Art : Gunrider#"
                        "Text : Indian of TriBal#"
                        "Panther : Peter Kürcman#");
  disp_clear();

  fVBLDelay(50);
  dots_doit(pal3);
  fVBLDelay(50);
  disp_fade_out(pal3);
  disp_clear(); 

  writer_doit((UBYTE *)
              "Calculating errors :#" 
	             "Pentium 90 mHz#"
              "Moral support : A guy and his#" 
              "oven, the snubbe with a#"
              "liggunderlag and others..#"
              "Inteloutside logo supplied by :#"
              "A Danish guy with an A600..#");
  disp_clear();

  fVBLDelay(50);
  dots_doit(pal4);
  fVBLDelay(50);
  disp_fade_out(pal4);
  disp_clear();

  /* LoadRGB4(mainVP, pal3, 16);
  mandel(theRaster);
  fVBLDelay(100);
  disp_clear(); */
  
  disp_clear();
  fVBLDelay(50);
  scroll_doit();

  disp_clear();
  fVBLDelay(50);
  writer_doit((UBYTE *) "Q: According to Intel, the Pentium#"
                        "conforms to the IEEE standards 754#"
                        "and 854 for floating point#"
                        "arithmetic. If you fly in aircraft#"
                        "designed using a Pentium, what is#" 
                        "the correct pronunciation of#"
                        "'IEEE'?##"
                        "A: [Aaaaaaaiiiiiiiiieeeeeeeeeeeee]#");

  disp_clear();
  fVBLDelay(50);

  pic = load_getmem((UBYTE *)"assets/outside.bin", 34 * 210 * 4);
  // disp_whack(pic, &theBitMap, 272, 210, 32, 18, 4);
  disp_fade_in(pal6);
  fVBLDelay(1000);
  disp_fade_out(pal6);
  disp_clear();
  FreeMem(pic, 34 * 120 * 4);
  
  // /* Close opened resources */
  ForceDemoClose();
  return (0);
}

/*********** FADER ***************/

/* Fade palette from all black to specified colors */
static void disp_fade_in(UWORD *fadeto)
{
  SHORT i, p;
  

  for (i = 0; i < 3; i ++)
    for (p = 0; p < 16; p ++)
      col[p][i] = 0;

  for (i = 0; i < 16; i ++)
  {
    incr[i][0] = ((fadeto[i] << 4) & 0xf000) / 15;
    incr[i][1] = ((fadeto[i] << 8) & 0xf000) / 15;
    incr[i][2] = ((fadeto[i] << 12) & 0xf000) / 15;
  }

  disp_fade_setpalette();
  for (i = 1; i < 16; i ++)
  {
    for (p = 0; p < 16; p ++)
    {
      col[p][0] += incr[p][0];
      col[p][1] += incr[p][1];
      col[p][2] += incr[p][2];
    }

    WaitTOF();
    WaitTOF();
    WaitTOF();
    disp_fade_setpalette();
    sys_check_abort();
  }

  WaitTOF();
  WaitTOF();
  WaitTOF();
  LoadRGB4(mainVP, fadeto, 16);
}

/* Fade palette from colors to all black */
static void disp_fade_out(UWORD *fadeFrom)
{
  UWORD i, p;

  for (i = 0; i < 16; i ++)
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
    WaitTOF();
    WaitTOF();
    disp_fade_setpalette();
    sys_check_abort();
  }
  WaitTOF();
  WaitTOF();
  WaitTOF();
  for (i = 0; i < 16; i ++)
    SetRGB4(mainVP, i, 0, 0, 0);
}

/* Set palette registers to currents colors while fading */
static void disp_fade_setpalette(void)
{
  UWORD i, temp, pal[16];

  for (i = 0; i < 16; i ++)
  {
    pal[i] = (col[i][0] & 0xf000) >> 4;
    temp = (col[i][1] & 0xf000) >> 8;
    pal[i] |= temp;
    temp = (col[i][2] & 0xf000) >> 12;
    pal[i] |= temp;
  }
  LoadRGB4(mainVP, pal, 16);
}

/***************** DISPLAY SUPPORT ********************/

void disp_clear(void)
{
  // SetRast(&theRP, 0);
  SetAPen(&theRP, 0);
  RectFill(&theRP, 0, frameOffset, 320, frameOffset + 256);
}

void full_clear(void)
{
  // SetRast(&theRP, 0);
  SetAPen(&theRP, 0);
  RectFill(&theRP, 0, 0, 320, 256 * 2);
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

  disp_fade_in(pal1);
  fVBLDelay(400);
  disp_fade_out(pal1);
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

  disp_fade_in(gradientPaletteRGB4); //pal5);
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

  disp_fade_out(gradientPaletteRGB4); //pal5);  
  FreeMem(font, 80 * 256);
  FreeMem(pic, 40 * 256 * 4);
}
