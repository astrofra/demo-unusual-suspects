/*  
    Unusual Suspects 
    Copper list routines 
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

#include "common.h"
#include "protos.h"

#include "Assets/faces_all_palettes.h"

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct ViewPort *mainVP;
extern struct Custom custom;

void DeleteCopperList(void)
{
  struct UCopList *cl;
  struct TagItem  uCopTags[] =
          {
                { VTAG_USERCLIP_SET, NULL },
                { VTAG_END_CM, NULL }
          };

  cl = (struct UCopList *) AllocMem(sizeof(struct UCopList), MEMF_PUBLIC|MEMF_CLEAR);

  CWAIT(cl, 0, 0);
  CEND(cl);

  Forbid();       /*  Forbid task switching while changing the Copper list.  */
  mainVP->UCopIns = cl;
  Permit();       /*  Permit task switching again.  */

  (VOID) VideoControl( mainVP->ColorMap, uCopTags );
  // MrgCop();
  RethinkDisplay();
}

void CreateHigheSTColorCopperList(int scanline_offset, int y_offset)
{
  struct UCopList *cl;
  struct TagItem  uCopTags[] =
          {
                { VTAG_USERCLIP_SET, NULL },
                { VTAG_END_CM, NULL }
          };
  int v, c, color_index = 0;

  cl = (struct UCopList *) AllocMem(sizeof(struct UCopList), MEMF_PUBLIC|MEMF_CLEAR);

  scanline_offset *= 16;

  for (v = 0; v < 256 - y_offset; v++)
  {
    CWAIT(cl, v + y_offset + 1, 0);
    for (c = 0; c < 16; c++)
    {
      CMOVE(cl, custom.color[faces_all_index[color_index + scanline_offset]], faces_all_scanline_PaletteRGB4[color_index + scanline_offset]);
      color_index++;
    }
  }

  CEND(cl);

  Forbid();       /*  Forbid task switching while changing the Copper list.  */
  mainVP->UCopIns = cl;
  Permit();       /*  Permit task switching again.  */

  (VOID) VideoControl( mainVP->ColorMap, uCopTags );
  RethinkDisplay();
}