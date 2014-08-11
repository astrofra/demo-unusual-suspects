/*  
    Unusual Suspects 
    Copper list routines headers. 
*/

void DeleteCopperList(void);
void CreateHigheSTColorCopperList(short scanline_offset, short y_offset);
void CreateVerticalCopperList(short y_offset, /* UWORD base_color,*/ UWORD *palette_array, short palette_size);