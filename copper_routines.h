/*  
    Unusual Suspects 
    Copper list routines headers. 
*/

void DeleteCopperList(void);
void CreateHigheSTColorCopperList(int scanline_offset, int y_offset);
void CreateVerticalCopperList(short y_offset, /* UWORD base_color,*/ UWORD *palette_array, short palette_size);