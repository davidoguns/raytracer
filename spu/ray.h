/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 22, 2008
 *  ray.h
 *
 *  This file contains the ray_t data type as well as the prototypes
 *  for the associated math functions to do intersection tests against
 *  geometry.
 */

#ifndef _RAY_H_
#define _RAY_H_

#include "geometry.h"

/* This structure is NOT 16 byte aligned thus will not be optimal
 * For DMA transfers on the Cell. */
typedef struct 
{
	point_t 	origin;		/* source point of ray */
	vector4_t	direction;	/* normalized vector */
	float		magnitude;	/* magnitude of ray - should be positive */
#if defined(__PPU__) || defined(__SPU__)
	unsigned char	pad[12];	/* end on 16byte boundary */
#endif
} ray_t;

/* functions to create rays conveniently */

/* create a ray from two points (vector4_t under the hood) */
ray_t* ray_create(ray_t *rayout, const point_t *p1, const point_t *p2);

/* copies a ray into another ray */
ray_t* ray_copy(ray_t *rayout, const ray_t *ray);

/* reverse the direction and origin of a ray */
ray_t* ray_reverse(ray_t *rayout, const ray_t *ray);

/* this function is needed for a serious hack that I really
 * hate doing.  R.I.P. clean code R.I.P. 
 * This function basically displaces origin by a tiny amount
 * along the direction vector to move it away from it's actual
 * spawn point.  This is necessary to prevent rays from intersecting
 * the objects that they spawned from */
ray_t* ray_tinypush(ray_t *rayout, const ray_t *ray);

/* all intersection functions return non zero if intersection occurs 
 * with the given geometry type and 0 if there is no intersection at all.
 * pt is the intersection point if one exists.
 */

/* tests if ray intersects a given polygon */
int ray_intersect_polygon(const ray_t *ray, const polygon_t* poly,
							point_t *pt, float *distance);
/* tests if ray intersects a given sphere */
int ray_intersect_sphere(const ray_t *ray, const sphere_t* sphere,
							point_t *pt, float *distance);

#endif
