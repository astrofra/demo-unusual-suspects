#include "includes.prl"
#include "common.h"

extern struct DosLibrary *DOSBase;

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

void disp_whack(PLANEPTR data, struct BitMap *dest_BitMap, UWORD width, UWORD height, UWORD x, UWORD y, UWORD depth)
{
  PLANEPTR src, dest;
  UWORD i, j, k;
  UWORD x_byte, width_byte;

  while(width != ((width >> 4) << 4))
   	width++;

  x_byte = x >> 3;
  width_byte = width >> 3;

  src = data;
  for (k = 0; k < depth; k ++)
  {
    dest = (*dest_BitMap).Planes[k] + 48 * y + x_byte;
    for (i = 0; i < height; i ++)
    {
      for (j = 0; j < width_byte; j ++)
      {
        *dest ++ = *src ++;
      }
      dest += (48 - width_byte);
      src += (width - (width_byte << 3));
    }
  }
}

// void disp_pixel_copy(PLANEPTR *source, struct RastPort *raster_port, UWORD width, UWORD height, UWORD x, UWORD y, UWORD depth)
// {
// 	UWORD i,j,p;
// 	static WORD c;

// 	for (j = 0; j < height; j++)
// 		for(i = 0; i < width; i++)
// 		{
// 			c = 0;
// 			for (p = 0; p < depth; p++)
// 			{

// 			}
// 			c = ReadPixel(raster_port, i, j);
// 			WritePixel(raster_port, i + x, j + y);
// 		}
// }