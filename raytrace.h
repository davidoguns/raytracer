/* David Oguns
 * Computer Graphics II
 * Ray Tracer
 * March 31, 2008
 * raytrace.h
 *
 * This file contains the prototypes for functions defined in raytrace.c
 * that will be called from outside of the source file itself.
 */

#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_

#include "scene.h"

/* called to start the ray tracing process */
void raytrace(unsigned int *buffer, scene_t *scene,
		float fovY, float aspectRatio, float nearZ, float farZ,
		unsigned int width, unsigned int height,
		unsigned int samplesPerPixelSq, unsigned int depth);

#endif

