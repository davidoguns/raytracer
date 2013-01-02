/*  David Oguns
 *  Computer Graphics II
 *  Warren Carithers
 *  May 10, 2008
 *  spe_program_info.h
 *
 *  This file defines the structure that will be passed to each SPE
 *  when it starts up
 */

#ifndef _SPE_PROGRAM_INFO_H_
#define _SPE_PROGRAM_INFO_H_

typedef struct
{
	unsigned int	speId;		/* SPE's id (used as index) */
	unsigned int	numPixels;	/* num pixels to process */
	unsigned int	samplesPerPixel;/* number of rays per pixel */
	unsigned int	numSpes;	/* number of spes being used */
	unsigned int	depth;		/* how many levels of recursion */
#ifdef __PPU__
	void	 	*scene;		/* ea of scene in XDR*/
	void	 	*frame_buffer;	/* ea of frame buffer in XDR */
	void		*ray_buffer;	/* ea of ray buffer in XDR */
#elif defined(__SPU__)
	unsigned long long	scene_ea;
	unsigned long long	frame_buffer_ea;
	unsigned long long	ray_buffer_ea;
#endif
} spe_program_info_t __attribute__((aligned(16) ));


#endif
