/*  
    Unusual Suspects 
    Fonts routines 
*/

#include "includes.prl"
#include "Assets/fonts.h"

void font_writer_blit(struct BitMap *src_BitMap, int x, int y, UBYTE *text_string)
{
  UBYTE *current_char;
  current_char = text_string;

  printf("font_writer_blit() str = %s\n", text_string);
}