/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 23, 2008
 *  plane.h
 *
 *  This file contains the 3D plane data type as well as the prototypes
 *  for any functions associated with it
 */

#ifndef _PLANE_H_
#define _PLANE_H_

typedef struct
{
	union
	{
		float c[4];		/* array access */
		struct
		{	/* dimension access */
			float A, B, C, F;
		};
	};
} plane_t;

#ifdef _DEBUG
	/* debug output plane to command line */
	void plane_output(const plane_t *plane);
#else
	#define plane_output(x)
#endif

#endif
