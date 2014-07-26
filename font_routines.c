/*  
    Unusual Suspects 
    Fonts routines 
*/

#include "includes.prl"
#include "Assets/fonts.h"

extern struct GfxBase *GfxBase;

int font_glyph_find_index(char glyph, const char *glyph_array)
{
	int i, l,
		r = -1;

	l = sizeof(glyph_array) / sizeof(glyph_array[0]);

	for(i = 0; i < l; i++)
		if (glyph == glyph_array[i])
		{
			r = i;
			break;
		}

	return(r);
}

void font_writer_blit(struct BitMap *font_BitMap, struct BitMap *dest_BitMap, const char *glyph_array, const int *x_pos_array, int x, int y, UBYTE *text_string)
{
	UBYTE *current_char;
	int glyph_index,
		glyph_w, glyph_h;

	current_char = text_string;
	glyph_h = font_BitMap->Rows;

	printf("font_writer_blit() str = %s\n", text_string);

	while(*current_char)
	{
		while(*current_char != '\n')
		{
			glyph_index = font_glyph_find_index(*current_char, glyph_array);

			if (glyph_index >= 0)
			{
				glyph_w = x_pos_array[glyph_index + 1] - x_pos_array[glyph_index];
				BltBitMap(font_BitMap, x_pos_array[glyph_index], 0, \
				            dest_BitMap, x, y,  \
				            glyph_w, glyph_h,      \
				            0xC0, 0xFF, NULL);
				// x += TextLength(&theRP_2bpl, currChar, 1);
			}
			current_char++;
		}
		current_char++;
		y += glyph_h;
		x = 0;
	}

	// x = 0;
	// y = y_base;
}