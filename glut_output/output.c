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
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "output.h"

/* global command line args for GLUT */
extern int g_argc;
extern char **g_argv;

/* buffer to draw whenever glDisplayFunc callback is invoked */
unsigned int *draw_buffer = 0;
unsigned int draw_width = 0;
unsigned int draw_height = 0;

/* create buffer glDrawPixels can use it's own format */
void covert_buffer(unsigned int *buffer, unsigned int width, unsigned int height)
{
	unsigned int i = 0;
	unsigned int j = 0;
	draw_buffer = (unsigned int *)malloc(sizeof(unsigned int)*width*height);

	for(; i < height; ++i)
	{
		for(j = 0; j < width; ++j)
		{	/* reverse row order and byte order */
			GLubyte *dest = (GLubyte *)&draw_buffer[i * width + j];
			GLubyte *src = (GLubyte *)&buffer[(height-(i+1)) * width + j];
			dest[3] = src[3];	/* A = A */
			dest[0] = src[2];	/* B = R */
			dest[1] = src[1];	/* G = G */
			dest[2] = src[0];	/* R = B */
		}
	}
}

/* draw function */
void render(void)
{
	unsigned int i = 0;
	unsigned int *buffer = draw_buffer;
	if(draw_buffer)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glOrtho(0, draw_width, 0, draw_height, 0.0f, 10.0f);
		/* loop necessary to draw right side up */
		glDrawPixels(draw_width, draw_height, GL_RGBA, GL_UNSIGNED_BYTE, draw_buffer);

		glFlush();
	}
}

/* write buffer to file as a 32 bit image */
int write_image(const char *filename, unsigned int *buffer,
	unsigned int width, unsigned int height)
{
	/* initialize glut */
	glutInit(&g_argc, g_argv);
	/* create window at 100, 100 */
	glutInitWindowPosition(100, 100);
	/* create proper window size */
	glutInitWindowSize(width, height);
	/* initialize display mode to 32 bit color */
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	/* create window we just initialized */
	glutCreateWindow("Ray Tracer Output");

	/* mark which buffer to draw */
	//draw_buffer = buffer;	
	draw_width = width;
	draw_height = height;
	covert_buffer(buffer, width, height);

	/* indicate what our display function callback is */
	glutDisplayFunc(render);

	glClearColor(.5, .5, .5, 1.0);

	/* initialize main loop */
	glutMainLoop();

	free(draw_buffer);

	return 1;
}
