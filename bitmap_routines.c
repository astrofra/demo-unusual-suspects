/*  Unusual Suspects 
    Misc bitmap routines */

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

void disp_interleaved_st_format(PLANEPTR data, struct BitMap *dest_BitMap, UWORD width, UWORD height, UWORD src_y, UWORD x, UWORD y, UWORD depth)
{
  PLANEPTR src, dest;
  UWORD i, j, k;
  UWORD x_byte, width_byte;

  while(width != ((width >> 4) << 4))
    width++;

  x_byte = x >> 3;
  width_byte = width >> 3;

  src = data;
  for (i = 0; i < height; i ++)
  {
    for (k = 0; k < depth; k ++)
    {
      for (j = 0; j < width_byte; j ++)
      {
        src = data + (j + (i + src_y) * 40 * depth) + (k * 40);
        dest = (*dest_BitMap).Planes[k] + j + x_byte + (48 * i) + 48 * y;

        *dest = *src;
      }
    }
  }
}