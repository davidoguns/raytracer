/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 23, 2008
 *  plane.h
 *
 *  This file contains the math functions related to planes.
 */

#include <stdio.h>
#include "plane.h"

#ifdef _DEBUG
/* debug output plane to command line */
void plane_output(const plane_t *plane)
{
	printf("Plane: %fx + %fy + %fz + %f = 0\n",
		plane->A, plane->B, plane->C, plane->F);
}
#endif
