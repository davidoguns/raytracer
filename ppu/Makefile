#David Oguns
#Computer Graphics II
#Warren Carithers
#PPU Makefile
#May 13, 2008
#

CC=ppu-gcc
SOURCES=ppu_main.c color.c geometry.c object3d.c output.c plane.c ppu_raytrace.c ray.c scene.c vector4.c
LDFLAGS=-lsimdmath -lnetpbm -lspe2
CFLAGS=-maltivec
INCLUDE=-I/opt/cell/sysroot/opt/cell/sdk/usr/include
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ppu_main

all: ${SOURCES} ${EXECUTABLE}

${EXECUTABLE}: ${OBJECTS}
	${CC} ${INCLUDE} ${CFLAGS} ${LDFLAGS} ${OBJECTS} -o $@

.c.o:
	$(CC) $(CFLAGS) ${INCLUDE} $< -o $@

clean:
	rm -rf *.o ppu_main
