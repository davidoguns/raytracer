/* David Oguns
 * Computer Graphics II
 * March 31, 2008
 * output.h
 * Ray Tracing Project
 *
 * This file contains the definition for the functions used for outputting buffers
 * to an image file or to the screen.
 */

#include <stdlib.h>
#include <pam.h>
#include <ppm.h>
#include <stdio.h>

#include "output.h"

/* global command line args for PAM */
extern int g_argc;
extern char **g_argv;

#define COLORVAL_MAX		0xFF

/* extract relevant color channels from a 32 bit pixel value */
#define COLOR_R(x)		((x & 0x00FF0000) >> 16)
#define COLOR_G(x)		((x & 0x0000FF00) >> 8)
#define COLOR_B(x)		((x & 0x000000FF))

/* write buffer to file as a 32 bit image */
int write_image(const char *filename, unsigned int *buffer,
	unsigned int width, unsigned int height)
{
	pixel **output_buffer = 0;
	pixval max = COLORVAL_MAX;
	unsigned int i;
	unsigned int j = 0;
	FILE *fp = fopen(filename, "w+");

	if(!fp)
	{
		printf("Could not open file {%s} for writing.\n", filename);
		return 0;
	}

	/* initialize PAM with command line args */
	ppm_init(&g_argc, g_argv);

	/* reserve space for image */
	output_buffer = ppm_allocarray(width, height);

	for(; j < height; ++j)
	{
		for(i = 0; i < width; ++i)
		{
			PPM_ASSIGN(output_buffer[j][i],
				COLOR_R(buffer[i+j*width]),
				COLOR_G(buffer[i+j*width]),
				COLOR_B(buffer[i+j*width]));
		}
	}

	/* write buffer to file */
	ppm_writeppm(fp, output_buffer, width, height, max, 0);

	/* free space */
	ppm_freearray(output_buffer, height);

	return 1;
}
