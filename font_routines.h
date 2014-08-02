/*  
    Unusual Suspects 
    Fonts routines headers
*/

#include "includes.prl"

void font_writer_blit(struct BitMap *font_BitMap, struct BitMap *font_BitMap_dark, struct BitMap *dest_BitMap, const char *glyph_array, const int *x_pos_array, int x, int y, UBYTE *text_string);