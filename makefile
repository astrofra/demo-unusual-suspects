.prl.gst:
	sc NOLINK CODE=F DATA=F OPTI MGST $*.gst $*.prl

.c.o:
	sc NOLINK CODE=F DATA=F OPTI TO $@ $*.c

OBJ = main.o init.o Assets/cosine_table.o Assets/object_cube.o Assets/object_amiga.o Assets/object_face_00.o

thanks-andy: $(OBJ) includes.gst
	sc LINK CODE=F DATA=F OBJ $(OBJ) TO thanks-andy

includes.gst: includes.prl
main.o: main.c common.h protos.h includes.gst
init.o: init.c common.h protos.h includes.gst
cosine_table.o: Assets/cosine_table.c
object_cube.o: Assets/object_cube.c
object_amiga.o: Assets/object_amiga.c
object_face_00.o: Assets/object_face_00.c
