# David Oguns
# March 22, 2008
# Computer Graphics Ray Tracer project makefile

CC=gcc
CFLAGS=
LDFLAGS=-lm -lnetpbm -lGL -lglut
SOURCES=color.c geometry.c main.c object3d.c output.c plane.c ray.c raytrace.c scene.c vector4.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=raytrace

all: ${SOURCES} ${EXECUTABLE}

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o raytrace
