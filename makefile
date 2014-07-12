.prl.gst:
	sc NOLINK CODE=F DATA=F OPTI MGST $*.gst $*.prl

.c.o:
	sc NOLINK CODE=F DATA=F OPTI TO $@ $*.c

OBJ = main.o init.o 3d_routines.o Assets/cosine_table.o Assets/object_cube.o Assets/object_amiga.o Assets/object_face_00.o Assets/object_spiroid.o Assets/faces_palettes.o

thanks-andy: $(OBJ) includes.gst
	sc LINK CODE=F DATA=F OBJ $(OBJ) TO thanks-andy

includes.gst: includes.prl
main.o: main.c common.h protos.h includes.gst
init.o: init.c common.h protos.h includes.gst
3d_routines.o: 3d_routines.c common.h protos.h includes.gst
cosine_table.o: Assets/cosine_table.c
object_cube.o: Assets/object_cube.c
object_amiga.o: Assets/object_amiga.c
object_face_00.o: Assets/object_face_00.c
object_spiroid.o: Assets/object_spiroid.c
faces_palettes.o: Assets/faces_palettes.c

