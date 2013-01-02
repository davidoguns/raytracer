#David Oguns
#Computer Graphics II
#Warren Carithers
#SPU Makefile
#May 16, 2008
#

CC=spu-gcc
SOURCES=spu_main.c color.c geometry.c object3d.c plane.c spu_raytrace.c ray.c vector4.c
LDFLAGS=
CFLAGS=
LDIRS=
INCLUDE=-I/opt/cell/sysroot/opt/cell/sdk/usr/spu/include -I/opt/cell/sdk/src/lib/vector
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=spu_raytrace

all: ${SOURCES} ${EXECUTABLE}

${EXECUTABLE}: ${OBJECTS}
	${CC} ${INCLUDE} ${CFLAGS} ${LDIRS} ${LDFLAGS} ${OBJECTS} -o $@

.c.o:
	$(CC) $(CFLAGS) ${LDIRS} ${LDFLAGS} ${INCLUDE} $< -o $@

clean:
	rm -rf *.o spu_raytrace
