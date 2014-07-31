/*  Unusual Suspects 
    Main program initialization */

#include "includes.prl"

#include "common.h"
#include "protos.h"

#define SCR_HEIGHT  (256 << 1)

/***** Static function declarations *****/

static void init_conerr(UBYTE *str);

/***** External variables *****/

/* Library pointers */
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *DiskfontBase;
struct Library *PTReplayBase;

PLANEPTR theRaster;
struct RastPort theRP;
struct RastPort theRP_5bpl;
struct RastPort theRP_4bpl;
struct RastPort theRP_3bpl;
struct RastPort theRP_2bpl;
struct RastPort theRP_1bpl;
struct BitMap theBitMap;
struct BitMap theBitMap_5bpl;
struct BitMap theBitMap_4bpl;
struct BitMap theBitMap_3bpl;
struct BitMap theBitMap_2bpl;
struct BitMap theBitMap_1bpl;
struct NewScreen theScreenEHB =
{
  0, 0, 320, SCR_HEIGHT, 6, 0, 1, 0 | EXTRA_HALFBRITE,
  CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET, NULL, NULL, NULL, &theBitMap
};

struct NewScreen theScreen32 =
{
  0, 0, 320, SCR_HEIGHT, 5, 0, 1, 0,
  CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET, NULL, NULL, NULL, &theBitMap
};

struct NewScreen theScreen16 =
{
  0, 0, 320, SCR_HEIGHT, 4, 0, 1, 0,
  CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET, NULL, NULL, NULL, &theBitMap
};

int current_screen_depth = 0,
    prev_screen_depth = 0;

struct Screen *mainScreen;
struct ViewPort *mainVP;
struct TextFont *writerFont;
struct TextAttr writerAttr =
{ (STRPTR)"futuraB.font", 32, 0, 0 };

/***** Global functions *****/

/* Open all needed global resources */
BOOL init_open_libs(void)
{
  /* Check for at least release 3.0 */
  if (SysBase->LibNode.lib_Version < 33)
  {
    init_conerr((UBYTE *)"This program requires Amiga Kickstart Release 1.2 +\n");
    return (FALSE);
  }

  /* Open needed libraries */
  if (!(IntuitionBase = (struct IntuitionBase *)
  OpenLibrary((UBYTE *)"intuition.library", _LIB_VERSION)))
  {
    init_conerr((UBYTE *)"Unable to open intuition.library version 33\n");
    return (FALSE);
  }
  if (!(GfxBase = (struct GfxBase *)
  OpenLibrary((UBYTE *)"graphics.library", _LIB_VERSION)))
  {
    init_conerr((UBYTE *)"Unable to open graphics.library version 33\n");
    return (FALSE);
  }
  if (!(DiskfontBase = OpenLibrary((UBYTE *)"diskfont.library", _LIB_VERSION)))
  {
    init_conerr((UBYTE *)"Unable to open diskfont.library version 33\n");
    return (FALSE);
  }

  if (!AssignPath("Libs","Libs"))
  {
    init_conerr((UBYTE *)"Failed to Assign the local Libs drawer. Please copy ptreplay.library into your Libs: drawer.\n");
    // return (FALSE);
  }

  if (!(PTReplayBase = OpenLibrary((UBYTE *)"ptreplay.library", 0)))
  {
    init_conerr((UBYTE *)"Unable to open ptreplay.library\n");
    return (FALSE);
  }

  // return (FALSE);

  return (TRUE);
}

BOOL Init16ColorsScreen(void)
{
  PLANEPTR temp;
  UWORD i;

  struct Screen *tmp_mainScreen = NULL;
  PLANEPTR tmp_theRaster = NULL;

  if (mainScreen) tmp_mainScreen = mainScreen;
  if (theRaster) tmp_theRaster = theRaster;

  InitBitMap(&theBitMap, 4, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_3bpl, 3, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_2bpl, 2, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_1bpl, 1, 384, SCR_HEIGHT);

  if (!(theRaster = AllocRaster(384 * 4, SCR_HEIGHT)))
  {
    init_conerr((UBYTE *)"Unable to allocate screen memory\n");
    return (FALSE);
  }

  temp = theRaster;
  for (i = 0; i < 4; i ++)
  {
    theBitMap.Planes[i] = temp;
    theBitMap_3bpl.Planes[i] = temp;
    theBitMap_2bpl.Planes[i] = temp;
    theBitMap_1bpl.Planes[i] = temp;
    temp += (48 * SCR_HEIGHT);
  }

  InitRastPort(&theRP);
  InitRastPort(&theRP_3bpl);
  InitRastPort(&theRP_2bpl);
  InitRastPort(&theRP_1bpl);

  theRP.BitMap = &theBitMap;
  theRP_3bpl.BitMap = &theBitMap_3bpl;
  theRP_2bpl.BitMap = &theBitMap_2bpl;
  theRP_1bpl.BitMap = &theBitMap_1bpl;
  SetRast(&theRP, 0);

  if (!(mainScreen = OpenScreen(&theScreen16)))
  {
    init_conerr((UBYTE *)"Unable to open main screen\n");
    return (FALSE);
  }
  mainVP = &mainScreen->ViewPort;
  for (i = 0; i < 16; i ++)
    SetRGB4(&mainScreen->ViewPort, i, 0, 0, 0);

  prev_screen_depth = current_screen_depth;
  current_screen_depth = 4;

  /* Close screen */
  if (tmp_mainScreen) CloseScreen(tmp_mainScreen);
  if (tmp_theRaster) FreeRaster(tmp_theRaster, prev_screen_depth * 384, SCR_HEIGHT);

  return (TRUE);
}

BOOL Init32ColorsScreen(void)
{
  PLANEPTR temp;
  UWORD i;

  struct Screen *tmp_mainScreen = NULL;
  PLANEPTR tmp_theRaster = NULL;

  if (mainScreen) tmp_mainScreen = mainScreen;
  if (theRaster) tmp_theRaster = theRaster;

  InitBitMap(&theBitMap, 5, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_4bpl, 4, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_3bpl, 3, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_2bpl, 2, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_1bpl, 1, 384, SCR_HEIGHT);

  if (!(theRaster = AllocRaster(384 * 5, SCR_HEIGHT)))
  {
    init_conerr((UBYTE *)"Unable to allocate screen memory\n");
    return (FALSE);
  }

  temp = theRaster;
  for (i = 0; i < 5; i ++)
  {
    theBitMap.Planes[i] = temp;
    theBitMap_4bpl.Planes[i] = temp;
    theBitMap_3bpl.Planes[i] = temp;
    theBitMap_2bpl.Planes[i] = temp;
    theBitMap_1bpl.Planes[i] = temp;
    temp += (48 * SCR_HEIGHT);
  }

  InitRastPort(&theRP);
  InitRastPort(&theRP_4bpl);
  InitRastPort(&theRP_3bpl);
  InitRastPort(&theRP_2bpl);
  InitRastPort(&theRP_1bpl);

  theRP.BitMap = &theBitMap;
  theRP_4bpl.BitMap = &theBitMap_4bpl;
  theRP_3bpl.BitMap = &theBitMap_3bpl;
  theRP_2bpl.BitMap = &theBitMap_2bpl;
  theRP_1bpl.BitMap = &theBitMap_1bpl;
  SetRast(&theRP, 0);

  if (!(mainScreen = OpenScreen(&theScreen32)))
  {
    init_conerr((UBYTE *)"Unable to open main screen\n");
    return (FALSE);
  }

  mainVP = &mainScreen->ViewPort;
  for (i = 0; i < 32; i ++)
    SetRGB4(&mainScreen->ViewPort, i, 0, 0, 0);

  prev_screen_depth = current_screen_depth;
  current_screen_depth = 5;

  /* Close screen */
  if (tmp_mainScreen) CloseScreen(tmp_mainScreen);
  if (tmp_theRaster) FreeRaster(tmp_theRaster, prev_screen_depth * 384, SCR_HEIGHT);

  return (TRUE);
}

BOOL InitEHBScreen(void)
{
  PLANEPTR temp;
  UWORD i;

  struct Screen *tmp_mainScreen = NULL;
  PLANEPTR tmp_theRaster = NULL;

  if (mainScreen) tmp_mainScreen = mainScreen;
  if (theRaster) tmp_theRaster = theRaster;

  InitBitMap(&theBitMap, 6, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_5bpl, 5, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_4bpl, 4, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_3bpl, 3, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_2bpl, 2, 384, SCR_HEIGHT);
  InitBitMap(&theBitMap_1bpl, 1, 384, SCR_HEIGHT);

  if (!(theRaster = AllocRaster(384 * 6, SCR_HEIGHT)))
  {
    init_conerr((UBYTE *)"Unable to allocate screen memory\n");
    return (FALSE);
  }

  temp = theRaster;
  for (i = 0; i < 6; i ++)
  {
    theBitMap.Planes[i] = temp;
    theBitMap_5bpl.Planes[i] = temp;
    theBitMap_4bpl.Planes[i] = temp;
    theBitMap_3bpl.Planes[i] = temp;
    theBitMap_2bpl.Planes[i] = temp;
    theBitMap_1bpl.Planes[i] = temp;
    temp += (48 * SCR_HEIGHT);
  }

  InitRastPort(&theRP);
  InitRastPort(&theRP_5bpl);
  InitRastPort(&theRP_4bpl);  
  InitRastPort(&theRP_3bpl);
  InitRastPort(&theRP_2bpl);
  InitRastPort(&theRP_1bpl);

  theRP.BitMap = &theBitMap;
  theRP_5bpl.BitMap = &theBitMap_5bpl;
  theRP_4bpl.BitMap = &theBitMap_4bpl;  
  theRP_3bpl.BitMap = &theBitMap_3bpl;
  theRP_2bpl.BitMap = &theBitMap_2bpl;
  theRP_1bpl.BitMap = &theBitMap_1bpl;
  SetRast(&theRP, 0);

  if (!(mainScreen = OpenScreen(&theScreenEHB)))
  {
    init_conerr((UBYTE *)"Unable to open main screen\n");
    return (FALSE);
  }
  mainVP = &mainScreen->ViewPort;
  for (i = 0; i < 32; i ++)
    SetRGB4(&mainScreen->ViewPort, i, 0, 0, 0);

  prev_screen_depth = current_screen_depth;
  current_screen_depth = 6;

  /* Close previous screen */
  if (tmp_mainScreen) CloseScreen(tmp_mainScreen);
  if (tmp_theRaster) FreeRaster(tmp_theRaster, prev_screen_depth * 384, SCR_HEIGHT);

  return (TRUE);
}

void init_close_video(void)
{
  /* Close screen */
  if (mainScreen) CloseScreen(mainScreen);
  if (theRaster) FreeRaster(theRaster, current_screen_depth * 384, SCR_HEIGHT);
}

/* Close all global resources opened */
void init_close_libs(void)
{
  /* Close opened libraries */
  if (PTReplayBase) CloseLibrary(PTReplayBase);
  if (DiskfontBase) CloseLibrary(DiskfontBase);
  if (GfxBase) CloseLibrary((struct Library *)GfxBase);
  if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
}

/***** Static functions *****/

/* Display an error message in a small console window */
static void init_conerr(UBYTE *str)
{
  BPTR fileHandle;		/* Console window filehandle */

  /* Open small console window */
  if (!(fileHandle = Open((UBYTE *)"CON:50/50/500/100/Picnic Editor error",
  MODE_OLDFILE)))
    return;
  /* Write message in window */
  Write(fileHandle, str, strlen((char *)str));
  /* Wait for 3 seconds */
  Delay(150);
  /* Close window */
  Close(fileHandle);
}
