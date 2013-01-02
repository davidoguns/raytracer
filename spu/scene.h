/* David Oguns
 * Computer Graphics II
 * March 29, 2008
 * scene.h
 * Ray Tracing Project
 *
 * This file contains the definition for the scene data structure.  Essentially
 * it is the container for all of the physical inputs of the ray tracer which
 * are the lights, objects, and camera parameters
 */
#ifndef _SCENE_H_
#define _SCENE_H_

#include "object3d.h"
#include "light.h"

#define STRING_BUFFER_SIZE	1024

/* this structure has evolved to a more general ray tracing support
 * structure beyond what would be considered something in the "scene."
 * Time constraints prevent me from making this fix clean */
typedef struct
{
	vector4_t		eyePos;		/* position of the camera */
	vector4_t		lookAt;		/* look at point */
	vector4_t		upVec;		/* up vector */
	vector4_t		U;		/* right vector */
	vector4_t		V;		/* up vector */
	vector4_t		N;		/* into the scene vector */

	unsigned int		frameBufferWidth;
	unsigned int		frameBufferHeight;
	unsigned int		sqrtSpp;
	float			viewDistance;
	float			viewPlaneHalfWidth;
	float			viewPlaneHalfHeight;
	float			sppWidth;

	color_t			bgColor;	/* background color */
	color_t			ambientLightColor;	/*ambient light color */

	float			ldMax;		/* max luminance of display */
	float			lMax;		/* max luminance of scene */

	unsigned int		nLights;	/* how many lights */
	unsigned int		nObjects;	/* how many objects */
#ifdef __SPU__
	union
	{
		struct
		{
			unsigned long long	lights_ea;
			unsigned long long	objects_ea;
		};		
#endif
		struct
		{
			pointlight_t	*lights;/* all lights in the scene */
			object3d_t	*objects;/* all objects in the scene */
		};
#ifdef __SPU__
	};
#endif
} scene_t;

/* load scene and camera properties from file */
int parse_scene(const char *filename, scene_t *scene);

/* cleanup dynamic memory from creating scene */
void free_scene(scene_t *scene);

#endif
