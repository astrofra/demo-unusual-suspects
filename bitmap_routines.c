/*  
    Unusual Suspects 
    Misc bitmap routines 
*/

#include "includes.prl"
#include "common.h"

extern struct DosLibrary *DOSBase;
extern struct GfxBase *GfxBase;

extern struct BitMap theBitMap;

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

struct BitMap *load_as_bitmap(UBYTE *name, ULONG byte_size, UWORD width, UWORD height, UWORD depth)
{
  BPTR fileHandle;
  struct BitMap *new_bitmap;

  if (!(fileHandle = Open(name, MODE_OLDFILE)))
    return (NULL);

  new_bitmap = AllocBitMap(width, height, depth, BMF_STANDARD, &theBitMap);

  printf("new_bitmap, BytesPerRow = %i, Rows = %i, Depth = %i, pad = %i, &Planes = %p, byte_size = %i, \n",
        (*new_bitmap).BytesPerRow,
        (*new_bitmap).Rows,
        (*new_bitmap).Depth,
        (*new_bitmap).pad,
        (*new_bitmap).Planes,
        byte_size);

  // Read(fileHandle, (*new_bitmap).Planes, byte_size);
  Close(fileHandle);

  return (new_bitmap);
}

void disp_whack(struct BitMap *src_BitMap, struct BitMap *dest_BitMap, UWORD width, UWORD height, UWORD x, UWORD y, UWORD depth)
{
  // PLANEPTR src, dest;
  // UWORD i, j, k;
  ULONG blit_count;

  blit_count = BltBitMap(src_BitMap, 0, 0,
            dest_BitMap, x, y,
            width, height,
            0xC0, 0xFF, NULL);

  printf("disp_whack() blit_plane_count = %i\n", blit_count);

  //   x_byte = x >> 3;
  //   width_byte = width >> 3;

  //   src = data;
  //   for (k = 0; k < depth; k ++)
  //   {
  //     dest = (*dest_BitMap).Planes[k] + 48 * y + x_byte;
  //     for (i = 0; i < height; i ++)
  //     {
  //       for (j = 0; j < width_byte; j ++)
  //       {
  //         *dest ++ = *src ++;
  //       }
  //       dest += (48 - width_byte);
  //       src += (width - (width_byte << 3));
  //     }
  //   }
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