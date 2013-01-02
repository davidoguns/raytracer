/* David Oguns
 * Computer Graphics II
 * March 23, 2008
 * Ray Tracer
 * object3d.h
 *
 * This header file contains the definition for the 3d objects that we will
 * be rendering for the ray tracer and functions associated with them.
 */

#ifndef _OBJECT3D_H_
#define _OBJECT3D_H_

#include "geometry.h"
#include "materials.h"

typedef struct
{
	unsigned short	geometryType;
#if !defined(__PPU__) && !defined(__SPU__)
	char		*debugName;		/* debug name */
#endif
	material_t 	material;		/* object surface properties */

	/* union of all geometry types */
	union
	{
		sphere_t 		sphr_obj;
		polygon_t	 	poly_obj;
	};
#if defined(__PPU__) || defined(__SPU__)
} object3d_t __attribute__(( aligned(16) ));
#else
} object3d_t;
#endif

/* gets the normal of an object at a specified point */
vector4_t* get_object_normal(vector4_t *vecout, const object3d_t *obj,
							 const point_t *pt);
/* gets the color of an object at a specified point */
/* note: not exactly the way I'd like to do this, but it seems to be
 * the cleanest approach at this point */
color_t* get_object_color(color_t *colorout, const object3d_t *obj,
							const point_t *pt);

#endif

