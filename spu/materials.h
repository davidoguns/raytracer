/* David Oguns
 * Computer Graphics II
 * Ray Tracer
 * March 23, 2008
 * material.h
 *
 * This file contains the definition for material properties
 * associated with 3D objects in the ray tracer.  This file
 * will be expanded as more materials need to be defined.
 */

#ifndef _MATERIALS_H_
#define _MATERIALS_H_

/* being used as an enum */
#define MATERIAL_NUMCOLORTYPES		2
#define MATERIAL_DIFFUSECOLOR		0
#define MATERIAL_SPECULARCOLOR		1

#include "color.h"

/* all of the properties that goes in the material of 
 * a solid surface */
typedef struct
{
	color_t			colors[MATERIAL_NUMCOLORTYPES]; /* reflective color values */
	float			phong_ka;		/* ambient coefficient */
	float			phong_kd;		/* diffuse coefficient */
	float			phong_ks;		/* specular coefficient */
	float			phong_ke;		/* specular exponent */
	float			kr;			/* coefficient of reflection */
	float			kt;			/* coefficient of transmission */
	float			n;			/* index of refraction */
} material_t;


#endif
