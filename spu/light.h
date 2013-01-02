/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracer
 *  March 24, 2008
 *  light.h
 *
 *  This file contains the definition for all of the light types used
 *  within the ray tracer.
 */

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "vector4.h"
#include "materials.h"

typedef struct
{
	vector4_t	position;	/* 3D position of light */
	color_t		color;		/* color of light */
	float		range;		/* maximum range light reaches? */
#if defined(__PPU__) || defined(__SPU__)
} pointlight_t __attribute__ ((aligned(16) ));
#else
} pointlight_t;
#endif

#endif
