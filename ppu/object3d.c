/* David Oguns
 * Computer Graphics II
 * Ray Tracer
 * object3d.c
 * April 12, 2008
 *
 * This file contains the definitions for functions associated with working
 * with objects in the ray tracing project.
 */

#include <math.h>
#include "object3d.h"

/* gets the normal of an object at a specified point */
vector4_t* get_object_normal(vector4_t *vecout, const object3d_t *obj,
							 const point_t *pt)
{
	switch(obj->geometryType)
	{
		case GEOMETRY_SPHERE:
			return get_sphere_normal(vecout, &obj->sphr_obj, pt);
		case GEOMETRY_POLYGON:
			return get_polygon_normal(vecout, &obj->poly_obj, pt);
		default:
			return vecout;
	}
}

/* gets the u,v position of a 3d point on a polygon surface */
point_t* get_2dproject_position(point_t *ptout, const polygon_t *poly,
				const point_t *pt)
{
	ptout->x = (pt->x + 15.0f) / 22.0f;
	ptout->y = (pt->z + 100.0f) / 100.0f;
	ptout->z = 0.0f;
	ptout->w = 0.0f;

	return ptout;
}

int get_row_pos(const sphere_t *sphere, const point_t *pt, int nStacks)
{
	/* how tall is this sphere */
	float height  = sphere->radius * 2.0f;
	/* how far is point offset from top on the Y */
	float yOff = (sphere->center.y + sphere->radius) - pt->y;
	
	return (yOff/height) * nStacks;	
}

int get_col_pos(const sphere_t *sphere, const point_t *pt, int nSlices)
{
	/* how wide is this sphere */
	float width = sphere->radius * 2.0f;
	/* how far is this point offset from left on the X */
	float xOff = (sphere->center.x - sphere->radius) - pt->x;

	return (xOff/width) * nSlices;
}

/* gets the color of an object at a specified point */
/* note: not exactly the way I'd like to do this, but it seems to be
 * the cleanest approach at this point */
color_t* get_object_color(color_t *colorout, const object3d_t *obj,
							const point_t *pt)
{
	point_t proj;
	int nTilesX = 11;
	int nTilesY = 50;
	int xtile = 0;
	int ytile = 0;
	int nStacks = 10;
	int nSlices = 10;

	switch(obj->geometryType)
	{
		case GEOMETRY_SPHERE:
			color_copy(colorout,
				&obj->material.colors[MATERIAL_DIFFUSECOLOR]);
/*
			if(get_row_pos(&obj->sphr_obj, pt, nStacks)%2)
			{
				colorout->r = 0.0f;
				colorout->g = 1.0f;
				colorout->b = 0.0f;
			}
*/
/*
			if(!(get_col_pos(&obj->sphr_obj, pt, nStacks)%2))
			{
				colorout->r = 0.0f;
				colorout->g = 1.0f;
				colorout->b = 0.0f;
			}
*/
			break;
		case GEOMETRY_POLYGON:
			/* find position in polygon plane */
			get_2dproject_position(&proj, &obj->poly_obj, pt);
			/* use this value to determine if red or yellow */
			xtile = nTilesX * proj.x;
			ytile = nTilesY * proj.y;
			if( (xtile%2) == (ytile%2) )
			{
				color_init(colorout);
				colorout->r  = 1.0f;
			}
			else
			{
				color_init(colorout);
				colorout->r = 1.0f;
				colorout->g = 1.0f;
			}
			break;
	}
	return colorout;
}




