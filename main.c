/* Main program */

#include "includes.prl"

#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <devices/keyboard.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>

#include "ptreplay.h"
#include "ptreplay_protos.h"
#include "ptreplay_pragmas.h"

#include "common.h"
#include "protos.h"

#include "Assets/object_cube.c"

static void disp_fade_in(UWORD *fadeto);
static void disp_fade_out(UWORD *fadeFrom);
static void disp_fade_setpalette(void);
void disp_clear(void);
void disp_swap(void);
void disp_whack(PLANEPTR data, UWORD width, UWORD height, UWORD x, UWORD y, UWORD depth);
void dots_doit(UWORD *pal);
void writer_doit(UBYTE *wrText);
void scroll_doit(void);
PLANEPTR load_getmem(UBYTE *name, ULONG size);
void mandel(PLANEPTR scrMem);
#pragma regcall(mandel(a0))

extern struct BitMap theBitMap;
extern struct BitMap theBitMap_3bpl;
extern struct BitMap theBitMap_2bpl;
extern struct BitMap theBitMap_1bpl;
extern struct RastPort theRP;
extern struct RastPort theRP_3bpl;
extern struct RastPort theRP_2bpl;
extern struct RastPort theRP_1bpl;

extern struct Custom custom;
extern struct CIA ciaa, ciab;
extern PLANEPTR theRaster, theRaster2;
extern struct Screen *mainScreen;
extern struct DosLibrary *DOSBase;
extern struct GfxBase *GfxBase;
extern struct Library *PTReplayBase;
extern struct ViewPort *mainVP;

/* Keyboard device */
struct MsgPort  *KeyMP;         /* Pointer for Message Port */
struct IOStdReq *KeyIO;         /* Pointer for I/O request */
UBYTE    *keyMatrix;
#define KEY_MATRIX_SIZE 16

struct Module *theMod;

BOOL swapFlag = FALSE;

UWORD incr[16][3];
UWORD col[16][3];

UWORD pal1[] =
{ 0x000, 0xFFF, 0x15b, 0x000, 0x000, 0x000, 0x000, 0x000,
  0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000 };

UWORD pal2[] =
{ 0x111, 0x223, 0x335, 0x447, 0x559, 0x66b, 0x77d, 0x88f,
  0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000 };

UWORD pal3[] =
{ 0x111, 0x322, 0x533, 0x744, 0x955, 0xb66, 0xd77, 0xf88,
  0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000 };

UWORD pal4[] =
{ 0x111, 0x232, 0x353, 0x474, 0x595, 0x6b6, 0x7d7, 0x8f8,
  0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000 };

UWORD pal5[] =
{ 0x000, 0x0B2, 0x082, 0x061, 0x235, 0x246, 0x357, 0x468,
  0x679, 0x78B, 0x9AC, 0x013, 0x014, 0x225, 0x300, 0x400 };

UWORD pal6[] =
{ 0x000, 0x111, 0x222, 0x333, 0x444, 0x555, 0x666, 0x777,
  0x888, 0x999, 0xAAA, 0xBBB, 0xCCC, 0xDDD, 0xEEE, 0xFFF };

UWORD pal7[] =
{ 0x000, 0x00F, 0x00B, 0x007, 0x13F, 0x303, 0x404, 0x050,
  0xFF0, 0x0DD, 0x0AF, 0x07C, 0x00F, 0x70F, 0xC0E, 0xC08 };

UWORD gradientPaletteRGB4[] =
{
  0x0CCC,0x0A20,0x0B40,0x0C60,0x0C70,0x0D90,0x0EB0,0x0BA0,
  0x07A0,0x0490,0x0080,0x0190,0x0290,0x04A0,0x05A0,0x06B0
};

/***** Global functions *****/

extern struct Library *SysBase;
struct Task *myTask;
BYTE oldPri;
PLANEPTR pic;
UBYTE *mod;

/********* 3D Code *********/

#define VERT_COUNT(n) (sizeof(n)/sizeof(n[0])/3)
#define FACE_COUNT(n) (sizeof(n)/sizeof(n[0])/4)

struct obj_3d
{
    int const* verts;
    int nverts;
    int const* faces;
    int nfaces;
};

#define vX(I) (3 * I + 0)
#define vY(I) (3 * I + 1)
#define vZ(I) (3 * I + 2)

#define Fc0(I) (4 * i + 0)
#define Fc1(I) (4 * i + 1)
#define Fc2(I) (4 * i + 2)
#define Fc3(I) (4 * i + 3)

#define fixed_pt_pre  512
#define fake_float (int)(0.7 * fixed_pt_pre)

int   tcos[360],
        tsin[360];

/*  Trigonometry PrecalcCosArray*/
void  PrecalcCosArray(void)
{
  int i;

  for(i = 0; i < 360; i++)
  {
    tcos[i] = fake_float; //cos(i * 2 * PI);
    tsin[i] = fake_float; //sin(i * 2 * PI);
  }
}

int Draw3DMesh(void)
{
  struct obj_3d o = { (int const *)&object_cube_verts, VERT_COUNT(object_cube_verts),
                   (int const *)&object_cube_faces, FACE_COUNT(object_cube_faces) };
  int i,tx,ty,
  x1,x2,x3,x4, 
  y1,y2,y3,y4,
  hidden,
  rx,ry;

  int XC,YC;
  int dist,alt;

  int *verts_tr;

  int cs, ss, cc, sc;

  XC = 160;
  YC = 160;
  dist = 128;
  alt = 512;

  verts_tr = (int *)malloc(sizeof(int) * o.nverts * 2);

  /*  Transform & project the vertices */

  rx = 45;
  ry = 45;

  //  pre-rotations
  cs = (tcos[rx] * tsin[ry]) / fixed_pt_pre,
  ss = (tsin[ry] * tsin[rx]) / fixed_pt_pre,
  cc = (tcos[rx] * tcos[ry]) / fixed_pt_pre,
  sc = (tsin[rx] * tcos[ry]) / fixed_pt_pre; 

  for (i = 0; i < o.nverts; ++i)
  {
    printf("vert %d/%d: %d %d %d\n", i, o.nverts,
           o.verts[vX(i)], o.verts[vY(i)], o.verts[vZ(i)]);

    /* 
        Rotation on 3 axis of each vertex
    */
    verts_tr[vX(i)] = o.verts[vX(i)];
    verts_tr[vY(i)] = o.verts[vY(i)];
    verts_tr[vZ(i)] = o.verts[vZ(i)];

    // verts_tr[vX(i)] = (o.verts[vX(i)] * tsin[rx] + o.verts[vY(i)] * tcos[rx]) / fixed_pt_pre;
    // verts_tr[vY(i)] = (o.verts[vX(i)] * cs  - o.verts[vY(i)] * ss + o.verts[vZ(i)] * tcos[ry]) / fixed_pt_pre;
    // verts_tr[vZ(i)] = (o.verts[vX(i)] * cc -  o.verts[vY(i)] * sc - o.verts[vZ(i)] * tsin[ry]) / fixed_pt_pre;

    /*
      Classic 3D -> 2D projection
    */

    tx = (verts_tr[vX(i)] * dist) / (verts_tr[vZ(i)] + alt);
    ty = (verts_tr[vY(i)] * dist) / (verts_tr[vZ(i)] + alt);
    verts_tr[vX(i)] = tx;
    verts_tr[vY(i)] = ty;

    printf("tr vert %d/%d: %d %d\n", i, o.nverts,
           verts_tr[vX(i)], verts_tr[vY(i)]);

  }

  for (i = 0; i < o.nfaces; ++i)
    printf("face %d/%d: %d %d %d %d\n", i, o.nfaces,
            o.faces[Fc0(i)], o.faces[Fc1(i)], o.faces[Fc2(i)], o.faces[Fc3(i)]);

  /*
    Draw each face (we assume it's a quad)
  */
  for (i = 0; i < o.nfaces; ++i)
  {
  //   fa = (f + i);
    x1 = XC + verts_tr[vX(o.faces[Fc0(i)])];
    y1 = YC + verts_tr[vY(o.faces[Fc0(i)])];
  //   x1 = XC + ( v2d + (fa -> v1) ) -> x;
  //   y1 = YC + ( v2d + (fa -> v1) ) -> y;

    x2 = XC + verts_tr[vX(o.faces[Fc1(i)])];
    y2 = YC + verts_tr[vY(o.faces[Fc1(i)])];
  //   x2 = XC + ( v2d + (fa -> v2) ) -> x;
  //   y2 = YC + ( v2d + (fa -> v2) ) -> y;

    x3 = XC + verts_tr[vX(o.faces[Fc2(i)])];
    y3 = YC + verts_tr[vY(o.faces[Fc2(i)])];
  //   x3 = XC + ( v2d + (fa -> v3) ) -> x;
  //   y3 = YC + ( v2d + (fa -> v3) ) -> y;

    x4 = XC + verts_tr[vX(o.faces[Fc3(i)])];
    y4 = YC + verts_tr[vY(o.faces[Fc3(i)])];

    //  should we draw the face ?
    // hidden = (x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1);

    printf("2D face (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n", x1, y1, x2, y2, x3, y3, x4, y4);

    // if (hidden > 0)
    {
      SetAPen(&theRP_3bpl, 15);

      Move(&theRP_3bpl, x1, y1);
      Draw(&theRP_3bpl, x2, y2);
      Draw(&theRP_3bpl, x3, y3);
      Draw(&theRP_3bpl, x4, y4);
      Draw(&theRP_3bpl, x1, y1);
    }
  }      
 
  // free(verts_tr);

  return 0;
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
  FreeMem(mod, 95430);

  Permit();
  SetTaskPri(myTask, oldPri);

  /*  Close the keyboard device */
  if (!(CheckIO((struct IORequest *)KeyIO)))
    AbortIO((struct IORequest *)KeyIO);   //  Ask device to abort request, if pending 
  // WaitIO((struct IORequest *)KeyIO);   /* Wait for abort, then clean up */
  CloseDevice((struct IORequest *)KeyIO);
  FreeMem(keyMatrix,KEY_MATRIX_SIZE);
  
  /* Close opened resources */
  init_close_all();
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

void CreateCopperList(void)
{
  // struct UCopList *cl;

  // cl = (struct UCopList *) AllocMem(sizeof(struct UCopList), MEMF_PUBLIC|MEMF_CLEAR);
  // CWAIT(cl, 100, 0);
  // CMOVE(cl, custom.color[0], 0xf0f);
  // CEND(cl);

  // mainVP->UCopIns = cl;

  // MrgCop();
  // RethinkDisplay();
}

/* Main program entry point */
int main(void)
{

  WriteMsg("Amiga C demo^Mandarine/Mankind 2014.\n");

  dispatch_func_ptr = NULL;

  InitKeyboard();

  srand((ciaa.ciatodmid << 8) | ciaa.ciatodlow);
  /* Open all needed resources */
  if (!init_open_all())
  {
    init_close_all();
    return (10);
  }

  // CreateCopperList();

  filter_off();

  myTask = FindTask(NULL);
  oldPri = SetTaskPri(myTask, 127);
  Forbid();

  mod = load_getmem((UBYTE *)"assets/module.bin", 95430);
  theMod = PTSetupMod((APTR)mod);
  PTPlay(theMod);

  // writer_doit((UBYTE *) "Barking Mad"
  //                       "Hedgehogs#"
  //                       "PRESENTS#");
  disp_clear();

  pic = load_getmem((UBYTE *)"assets/logo.bin", 40 * 4 * 256);
  disp_whack(pic, 40, 256, 0, 0, 4);
  disp_fade_in(pal7);

  Draw3DMesh();

  fVBLDelay(350);
  disp_fade_out(pal7);
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
  disp_whack(pic, 34, 210, 4, 18, 4);
  disp_fade_in(pal6);
  fVBLDelay(1000);
  disp_fade_out(pal6);
  disp_clear();
  FreeMem(pic, 34 * 120 * 4);

  // PTStop(theMod);
  // PTFreeMod(theMod);
  // FreeMem(mod, 95430);

  // Permit();
  // SetTaskPri(myTask, oldPri);
  
  // /* Close opened resources */
  // init_close_all();
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
  SetRast(&theRP, 0);
}

void disp_swap(void)
{
  UWORD i;
  PLANEPTR temp;

  if (swapFlag)
    temp = theRaster;
  else
    temp = theRaster;
  swapFlag = !swapFlag;

  for(i = 0; i < 4; i ++)
  {
    mainVP->RasInfo->BitMap->Planes[i] = temp;
    theBitMap.Planes[i] = temp;
    theBitMap_3bpl.Planes[i] = temp;
    theBitMap_2bpl.Planes[i] = temp;
    theBitMap_1bpl.Planes[i] = temp;
    temp += (48 * 256);
  }
  MakeVPort(GfxBase->ActiView, mainVP);
  MrgCop(GfxBase->ActiView);
  LoadView(GfxBase->ActiView);
}

void disp_whack(PLANEPTR data, UWORD width, UWORD height, UWORD x, UWORD y, UWORD depth)
{
  PLANEPTR src, dest;
  UWORD i, j, k;

  src = data;
  for (k = 0; k < depth; k ++)
  {
    dest = theBitMap.Planes[k] + 48 * y + x;
    for (i = 0; i < height; i ++)
    {
      for (j = 0; j < width; j ++)
      {
        *dest ++ = *src ++;
      }
      dest += (48 - width);
    }
  }
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
  disp_whack(pic, 40, 256, 0, 0, 4);
  currChar = scrText;

  disp_fade_in(gradientPaletteRGB4); //pal5);
  fVBLDelay(100);
  
  while (*currChar)
  {
    BltBitMap(&fontMap, offs[(*currChar) << 1], offs[((*currChar) << 1) + 1],
    &theBitMap_2bpl, 320, 208, 32, 48, 0xc0, 0xff, NULL);
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

/*************** LOADER *********************/

PLANEPTR load_getmem(UBYTE *name, ULONG size)
{
  BPTR fileHandle;
  PLANEPTR mem;

  if (!(fileHandle = Open(name, MODE_OLDFILE)))
    return (NULL);

  if (!(mem = AllocMem(size, MEMF_CHIP)))
    return (NULL);

  Read(fileHandle, mem, size);
  Close(fileHandle);

  return (mem);
}
