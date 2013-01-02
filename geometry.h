/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 23, 2008
 *  geometry.h
 *
 *  This file contains the definitions for the data types representing
 *  the 3d geometry we will render in the ray tracer.
 */

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "vector4.h"
#include "plane.h"

#define GEOMETRY_SPHERE		0x01
#define GEOMETRY_POLYGON	0x02


/* spheres will be defined by a center point and radius */
typedef struct
{
	point_t		center;
	float		radius;
} sphere_t;

/* polygons are defined by a plane, and a set of verticies that
 * are hopefully on the plane.  We will assume the first three
 * verticies in the list of verticies define the plane that the
 * remaining vertices lie on.  This plane will be generated at
 * scene loading time.
 */
typedef struct
{
	plane_t			plane;		/* plane this polygon exists on */
	unsigned int	 	nVerticies;	/* how many vertices in this polygon (n > 3)*/

#ifdef __SPU__
	union
	{
		unsigned long long	vertex_ea;
#endif
		point_t			*vertex; /* corners defining polygon */
#ifdef __SPU__
	};
#endif
} polygon_t;

/* generate a plane equation given a polygon - uses the first three vertices */
plane_t* poly_plane(plane_t *planeout, const polygon_t *poly);

/* get normal vector to sphere at point passed in */
vector4_t* get_sphere_normal(vector4_t *vecout, const sphere_t *sphere,
							 const point_t *pt);

/* get normal vector to plane at point passed in */
vector4_t* get_polygon_normal(vector4_t *vecout, const polygon_t *poly,
							  const point_t *pt);

#endif
