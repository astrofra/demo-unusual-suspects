.prl.gst:
	sc NOLINK CODE=F DATA=F OPTI MGST $*.gst $*.prl

.c.o:
	sc NOLINK CODE=F DATA=F OPTI TO $@ $*.c

OBJ = main.o init.o

wild: $(OBJ) includes.gst
	sc LINK CODE=F DATA=F OBJ $(OBJ) TO wild.new

includes.gst: includes.prl
main.o: main.c common.h protos.h includes.gst
init.o: init.c common.h protos.h includes.gst
