/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracer
 *  March 22, 2008
 *  main.cpp
 *
 *  This file is the entry point for the ray tracing program
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "main.h"
#include "scene.h"
#include "raytrace.h"
#include "output.h"
#ifdef _DEBUG
	#include "matrix4.h"
	#include "ray.h"
#endif

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
		free(frame_buffer);
	}

	/* allocate memory for a new buffer */
	frame_buffer = malloc( sizeof(unsigned int) * width * height );

	for(i = 0; i < nPixels; ++i)
	{
		/* fill buffer with green - GL_ABGR */
		unsigned char tmp = (unsigned char)((i/(float)nPixels) * 255.0f);
		frame_buffer[i] = 0xFF000000 | tmp << 16;
	}
}

/* deallocates memory used for buffers */
void free_buffers()
{
	if(frame_buffer)
	{
		free(frame_buffer);
		frame_buffer = 0;
	}
}

#ifdef _DEBUG
void do_stuff()
{
	ray_t ray;
	ray.origin.x = 0.0f;
	ray.origin.y = 0.0f;
	ray.origin.z = 0.0f;
	ray.origin.w = 0.0f;
	ray.direction.x = 1.0f;
	ray.direction.y = 1.0f;
	ray.direction.z = 1.0f;
	ray.direction.w = 0.0f;
	ray.magnitude = 20.0f;

	vec4_normalize(&ray.direction);

	printf("Before tiny push:\t (%f, %f, %f)\n",
		ray.origin.x, ray.origin.y, ray.origin.z);

	ray_tinypush(&ray, &ray);

	printf("After tiny push:\t (%f, %f, %f)\n",
		ray.origin.x, ray.origin.y, ray.origin.z);

	printf("\n");
}
#endif

/* Params - 
 * 	0 - program name
 * 	1 - output image filename
 * 	2 - input scene filename
 * 	3 - image width
 * 	4 - image height
 * 	5 - samples_per_pixel^2
 * 	6 - depth
 */
int main(int argc, char **argv)
{
	const char		*imgOut = 0;
	const char		*sceneFile = 0;
	unsigned int		imgWidth = 0;
	unsigned int		imgHeight = 0;
	unsigned int		samplesPerPixel = 1;
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

	if(argc < ARGC_EXPECTED)
	{
		printf("raytrace outputFile sceneFile imgWidth imgHeight samplesPerPixel^2 depth\n");
		exit(1);
	}

	imgOut = argv[ARGV_OUTPUTIMG];
	sceneFile = argv[ARGV_SCENEFILE];
	imgWidth = atoi(argv[ARGV_IMAGEWIDTH]);
	imgHeight = atoi(argv[ARGV_IMAGEHEIGHT]);
	/* this is actually sqrt(spp) */
	samplesPerPixel = atoi(argv[ARGV_SAMPLESPERPIXEL]);
	depth = atoi(argv[ARGV_DEPTH]);
	
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
		1.0f, 200.0f, imgWidth, imgHeight, samplesPerPixel, depth);

	/* output file image to file or screen (netbpm lib?) */
	write_image(imgOut, frame_buffer, imgWidth, imgHeight);

	free_scene(&scene);
	free_buffers();

	time(&end);
	diff = difftime(end, start);
	printf("Run time:\t%f\n", diff);

	return 0;
}
