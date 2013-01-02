/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 31, 2008
 *  geometry.h
 *
 *  This file contains the functions associated with working with geometry
 *  in the ray tracer.
 */

#include "geometry.h"

/* generate a plane equation given a polygon - uses the first three vertices */
plane_t* poly_plane(plane_t *planeout, const polygon_t *poly)
{
	vector4_t	v1;			/* from vertex 2 to vertex 1 */
	vector4_t	v2;			/* from vertex 2 to vertex 3 */

	vec4_sub(&v1, &poly->vertex[0], &poly->vertex[1]);
	vec4_sub(&v2, &poly->vertex[2], &poly->vertex[1]);
	/* take cross product between both vectors */
	vec4_cross((vector4_t *)planeout, &v2, &v1);
	/* now normalize it */
	vec4_normalize((vector4_t *)planeout);

	/* now calculate F - shortest distance from origin to point on plane */
	/* we do this by taking the dot product of the normal and a known
	 * point on the plane */
	planeout->F = vec4_dot((vector4_t *)planeout, &poly->vertex[0]);
	/* force F to be positive? */
	if(planeout->F < 0.0f)
		planeout->F = -planeout->F;
	return planeout;
}

/* get normal vector to sphere at point passed in */
vector4_t* get_sphere_normal(vector4_t *vecout, const sphere_t *sphere,
							 const point_t *pt)
{	/* subtract point on surface from center point of sphere */
	vec4_sub(vecout, pt, &sphere->center);
	/* normalize normal vector */
	vec4_normalize(vecout);

	return vecout;
}

/* get normal vector to plane at point passed in */
vector4_t* get_polygon_normal(vector4_t *vecout, const polygon_t *poly,
							  const point_t *pt)
{
	vecout->x = poly->plane.A;
	vecout->y = poly->plane.B;
	vecout->z = poly->plane.C;
	vecout->w = 1.0f;

	return vecout;
}

