/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracer
 *  April 16, 2008
 *  ppu_main.cpp
 *
 *  This file is the entry point for the ray tracing program.
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <altivec.h>
#include <vec_types.h>
#include <malloc_align.h>
#include <free_align.h>
#include <time.h>

#include "ppu_main.h"
#include "scene.h"
#include "ppu_raytrace.h"

/* global command line args for GLUT */
int g_argc = 0;
char **g_argv = 0;

/* frame buffer that holds final image */
unsigned int *frame_buffer = 0;

/* initialize frame buffer and any associated buffers
 * necessary to complete the ray tracing task (depth
 * buffer, etc)
 */
void init_buffers(unsigned int width, unsigned int height)
{
	unsigned int i = 0;
	unsigned int nPixels = width * height;
	if(frame_buffer)
	{
		/* free any allocated buffer if it is allocated */
		_free_align(frame_buffer);
	}

	/* allocate memory for a new buffer */
	frame_buffer = _malloc_align(sizeof(unsigned int)*width*height, 2);

#ifdef _DEBUG
	for(i = 0; i < nPixels; ++i)
	{
		/* fill buffer with red gradient - GL_ABGR */
		unsigned char tmp = (unsigned char)((i/(float)nPixels) * 255.0f);
		frame_buffer[i] = 0xFF000000 | tmp << 16;
	}
#endif
}

/* deallocates memory used for buffers */
void free_buffers()
{
	if(frame_buffer)
	{
		_free_align(frame_buffer);
		frame_buffer = 0;
	}
}

void do_stuff()
{
#ifdef __PPU__
	vector float v1 = {1.0f, 2.0f, 4.0f, 8.0f};
	vector unsigned char vp;
	vp[15] = 4 << 3;	/* shift 4 bytes */
	vector float v2 = {3.0f, 3.0f, 3.0f, 3.0f};
	vector float v3 = vec_slo(v1, VECPATTERN_SHIFTLEFT(4));

/*
	vec_float4 v1 = {1.0f, 1.0f, 1.0f, 1.0f};
	vec_float4 v2 = {3.0f, 3.0f, 3.0f, 3.0f};
	vec_float4 v3 = v1 + v2;
*/

	printf("vector: <%f, %f, %f, %f>\n", v3[0], v3[1],
					v3[2], v3[3]);
	printf("sizeof pointer: %d\n", sizeof(void *));
	printf("sizeof object3d_t: %d\n", sizeof(object3d_t));
	printf("sizeof material_t: %d\n", sizeof(material_t));
	printf("sizeof plane_t: %d\n", sizeof(plane_t));
	printf("sizeof polygon_t: %d\n", sizeof(polygon_t));
	printf("sizeof sphere_t: %d\n", sizeof(sphere_t));
	printf("sizeof unsigned int: %d\n", sizeof(unsigned int));
	printf("sizeof unsigned long long: %d\n", sizeof(unsigned long long));
	printf("sizeof point_t*: %d\n", sizeof(point_t *));
	printf("sizeof ray_t*: %d\n", sizeof(ray_t));
#endif
}

/* Params - 
 * 	0 - program name
 * 	1 - output image filename
 * 	2 - input scene filename
 * 	3 - image width
 * 	4 - image height
 * 	5 - samples_per_pixel^2
 */
int main(int argc, char **argv)
{
	const char		*imgOut = 0;
	const char		*sceneFile = 0;
	unsigned int		imgWidth = 0;
	unsigned int		imgHeight = 0;
	unsigned int		samplesPerPixel = 1;
	unsigned int		numSpes = MAX_SPES;
	unsigned int		depth = 1;
	scene_t			scene;
	time_t			start, end;
	double			diff;

	time(&start);

	/* copy command line params to global */
	g_argc = argc;
	g_argv = argv;

#ifdef _DEBUG	
	do_stuff();
#endif

	if(argc < 8)
	{
		printf("raytrace outputFile sceneFile imgWidth imgHeight samplesPerPixel^2 ndepth umSpes\n");
		exit(1);
	}

	imgOut = argv[ARGV_OUTPUTIMG];
	sceneFile = argv[ARGV_SCENEFILE];
	imgWidth = atoi(argv[ARGV_IMAGEWIDTH]);
	imgHeight = atoi(argv[ARGV_IMAGEHEIGHT]);
	samplesPerPixel = atoi(argv[ARGV_SAMPLESPERPIXEL]);
	numSpes = atoi(argv[ARGV_NUMSPES]);
	depth = atoi(argv[ARGV_DEPTH]);
	
	if(numSpes > MAX_SPES)
	{
		printf("Cannot exceed %d concurrent spes.  Exitting...\n", MAX_SPES);
		exit(1);
	}

	/* initialize frame buffer and such */
	init_buffers(imgWidth, imgHeight);

	printf("Running Ray Tracer...\n");

#ifdef _DEBUG
	printf("Output file name = %s\n", imgOut);
	printf("Image Width = %d\n", imgWidth);
	printf("Image Height = %d\n", imgHeight);
#endif

	/* parse scene file */
	if(!parse_scene(sceneFile, &scene))
	{
		printf("Error parsing scene file.  Exiting...\n");
		exit(1);
	}


	raytrace(frame_buffer, &scene, 45.0f, imgWidth/(float)imgHeight,
		1.0f, 200.0f, imgWidth, imgHeight, samplesPerPixel,
		 depth, numSpes);

	/* output file image to file or screen (netbpm lib?) */
	write_image(imgOut, frame_buffer, imgWidth, imgHeight);

	free_scene(&scene);
	free_buffers();

	time(&end);
	diff = difftime(end, start);
	printf("Run time:\t%f\n", diff);


	return 0;
}
