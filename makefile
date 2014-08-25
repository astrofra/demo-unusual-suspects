.prl.gst:
	sc NOLINK CODE=F DATA=F OPTI MGST $*.gst $*.prl

.c.o:
	sc NOLINK CODE=F DATA=F OPTI TO $@ $*.c

OBJ = main.o init.o timer_routines.o 3d_routines.o bitmap_routines.o copper_routines.o font_routines.o \
		Assets/cosine_table.o Assets/3d_objects.o \
		demo_strings.o Assets/fonts.o \
		Assets/misc_palettes.o Assets/faces_palettes.o Assets/faces_all_palettes.o Assets/vert_copper_palettes.o

unusual-suspects: $(OBJ) includes.gst
	sc LINK CODE=F DATA=F OBJ $(OBJ) TO unusual-suspects.exe

includes.gst: includes.prl
main.o: main.c common.h protos.h timer_routines.h 3d_routines.h copper_routines.h bitmap_routines.h \
	Assets/cosine_table.h Assets/3d_objects.h \
	Assets/misc_palettes.h Assets/faces_palettes.h Assets/vert_copper_palettes.h \
	Assets/faces_all_palettes.h Assets/fonts.h demo_strings.h includes.gst
init.o: init.c common.h protos.h includes.gst
timer_routines.o: timer_routines.c common.h protos.h includes.gst
3d_routines.o: 3d_routines.c common.h protos.h includes.gst
bitmap_routines.o: bitmap_routines.c common.h protos.h includes.gst
copper_routines.o: copper_routines.c common.h protos.h includes.gst
font_routines.o: font_routines.c common.h protos.h includes.gst
cosine_table.o: Assets/cosine_table.c
3d_objects.o: Assets/3d_objects.c
misc_palettes.o: Assets/misc_palettes.c
faces_palettes.o: Assets/faces_palettes.c
faces_all_palettes.o: Assets/faces_all_palettes.c
vert_copper_palettes.o: Assets/vert_copper_palettes.c
fonts.o: Assets/fonts.c
demo_strings.o: demo_strings.c

