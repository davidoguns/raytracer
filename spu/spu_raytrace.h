/* David Oguns
 * Computer Graphics II
 * Warren Carithers
 * Cell Ray Tracer Project
 * spu_raytrace.h
 * May 14, 2008
 *
 * This file defines the main functions and algorithm that will be used for 
 * running the ray tracer on the SPUs.  It handles the DMA of reading in rays
 * (if any) and DMA of writing out pixels back to main memory.
 *
 */

#ifndef _SPU_RAYTRACE_H_
#define _SPU_RAYTRACE_H_

#include "spe_program_info.h"
#include "spu_tag_masks.h"
#include "scene.h"

/* handles the ray tracing task handed to this spe and returns */
void spu_raytrace(spe_program_info_t *info, const scene_t *scene);

#endif


