/*  
    Unusual Suspects 
    Fonts routines 
*/

#include "includes.prl"
#include "Assets/fonts.h"

extern struct GfxBase *GfxBase;
extern void sys_check_abort(void);

int font_glyph_find_index(char glyph, const char *glyph_array)
{
	int i = 0,
		r = -1;

	while(glyph_array[i])
	{
		if (glyph == glyph_array[i])
		{
			r = i;
			break;
		}

		i++;
	}

	return(r);
}

void font_writer_blit(struct BitMap *font_BitMap, struct BitMap *dest_BitMap, const char *glyph_array, const int *x_pos_array, int x, int y, UBYTE *text_string)
{
	// UBYTE *current_char;
	int i = 0, glyph_index, cur_x,
		line_feed = 0,
		glyph_w, glyph_h;

	cur_x = x;
	// current_char = text_string;
	glyph_h = font_BitMap->Rows;

	while(text_string[i])
	{
		while(text_string[i] && text_string[i] != '\n')
		{
			sys_check_abort();

			if (text_string[i] == ' ')
			{
				cur_x += 4;
				WaitTOF();
				WaitTOF();
			}
			else
			{
				glyph_index = font_glyph_find_index((char)text_string[i], glyph_array);
				if (glyph_index >= 0)
				{
					glyph_w = x_pos_array[glyph_index + 1] - x_pos_array[glyph_index];
					BltBitMap(font_BitMap, x_pos_array[glyph_index], 0,
					            dest_BitMap, cur_x, y,
					            glyph_w, glyph_h,
					            0xC0, 0xFF, NULL);

					cur_x += (glyph_w);
				}
			}
			i++;
		}
		i++;
		y += (glyph_h + 1);
		line_feed++;		
		cur_x = x;
		if (line_feed > 4)
			cur_x -= 50;
	}
}