/* David Oguns
 * Computer Graphics II
 * Warren Carithers
 * Cell Ray Tracing Project
 * May 10,2008
 * spu_tag_masks.h
 *
 * This file defines the values used for each DMA command type
 * that will be needed to execute the SPU program
 */

#ifndef _SPU_TAG_MASKS_H_
#define _SPU_TAG_MASKS_H_


#define SPUDMA_PROGRAMINFO	0
#define SPUDMA_SCENE		1
#define SPUDMA_RAYS		2
#define SPUDMA_PIXELSOUT	3
#define SPUDMA_LIGHTS		4
#define SPUDMA_OBJECTS		5
#define SPUDMA_VERTEXES		6

/* number of tag id's explicitly reserved */
#define SPUDMA_TAGS_RESERVED	7

/* tag id for DMA buffer N */
#define SPUDMA_BUFFER(N)	(N + SPUDMA_TAGS_RESERVED)

#endif
