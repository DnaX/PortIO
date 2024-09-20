all: portio

OBJECTS = main.o uber-graph.o uber-buffer.o
#HEADERS = gtkmodebutton.h
PKGS = glib-2.0 gtk+-2.0

main.o: main.c Makefile
	$(CC) -g -c -o $@ $(WARNINGS) $(INCLUDES) main.c $(shell pkg-config --libs --cflags $(PKGS))

%.o: %.c %.h Makefile
	$(CC) -g -c -o $@ $(WARNINGS) $(INCLUDES) $*.c $(shell pkg-config --libs --cflags $(PKGS))


portio: $(OBJECTS) Makefile
	$(CC) -g -o $@ $(SOURCES) $(shell pkg-config --libs --cflags $(PKGS)) $(OBJECTS)

clean:
	rm -rf portio
