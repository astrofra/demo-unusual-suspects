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

	l = 88;
	for(i = 0; i < l; i++)
	{
		// printf("%c", (*glyph_array + i));
		if (glyph == (*glyph_array + i))
		{
			r = i;
			break;
		}
	}

	// printf("\n");
	return(r);
}

void font_writer_blit(struct BitMap *font_BitMap, struct BitMap *dest_BitMap, const char *glyph_array, const int *x_pos_array, int x, int y, UBYTE *text_string)
{
	UBYTE *current_char;
	int glyph_index, cur_x,
		glyph_w, glyph_h;

	cur_x = x;
	current_char = text_string;
	glyph_h = font_BitMap->Rows;

	printf("font_writer_blit() str = %s\n", text_string);

	while(*current_char)
	{
		while(*current_char != '\n')
		{
			if (*current_char == ' ')
				cur_x += 4;
			else
			{
				glyph_index = font_glyph_find_index((char)*current_char, glyph_array);
				if (glyph_index >= 0)
				{
					glyph_w = x_pos_array[glyph_index + 1] - x_pos_array[glyph_index];
					BltBitMap(font_BitMap, x_pos_array[glyph_index], 0,
					            dest_BitMap, cur_x, y,
					            glyph_w, glyph_h,
					            0xC0, 0xFF, NULL);

					cur_x += (glyph_w + 1);

					printf("c = %c, ", *current_char);
					printf("w = %i, x = %i\n", glyph_w, cur_x);
				}
			}
			current_char++;
			// *current_char = 'A';
		}
		current_char++;
		y += (glyph_h + 2);
		cur_x = x;
	}

	// x = 0;
	// y = y_base;
}