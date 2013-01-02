/* David Oguns
 * Computer Graphics II
 * March 31, 2008
 * output.h
 * Ray Tracing Project
 *
 * This file contains the definition for the functions used for outputting buffers
 * to an image file or to the screen.
 */

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

/* write buffer to file as a 32 bit image */
int write_image(const char *filename, unsigned int *buffer,
	unsigned int width, unsigned int height);

#endif

