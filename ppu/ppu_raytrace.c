/* David Oguns
 * Computer Graphics II
 * Ray Tracer
 * March 31, 2008
 * raytrace.c
 *
 * This file contains the functions necessary to accomplish the meat of the
 * ray tracing task.
 */

#define _USE_MATH_DEFINES

#ifdef _DEBUG
	#include <stdio.h>
#endif

#include <math.h>
#include <libspe2.h>
#include <pthread.h>
#include <malloc_align.h>
#include <free_align.h>
#include "ppu_raytrace.h"
#include "ray.h"

#define mfc_ea2h(ea)   (unsigned int)((unsigned long long)(ea)>>32)
#define mfc_ea2l(ea)   (unsigned int)(ea)

/* spe code handle */
/* external version */
spe_program_handle_t 	*spe_code = 0;
/* embedded version */
/* extern spe_program_handle_t 	spe_code; */
/* each SPE program context and stop info*/
pthread_t		pthreads[MAX_SPES];
spe_pthread_info_t	spe_thread[MAX_SPES];
unsigned int		completed_pixels[MAX_SPES];


/* prepares spe_program_info_t struct for each spes.
 * primarily has to initialize ray buffer, and assign relevant 
 * initial information. */
void init_spe_program_raybuffers(unsigned int width, unsigned int height,
			unsigned int samplesPerPixel, unsigned int numSpes,
			spe_pthread_info_t *info)
{
	unsigned int initialCount = (width * height) / numSpes;
	unsigned int overflow = (width * height) % numSpes;
	unsigned int i = 0;
	
	/* allocate enough rays that each spe will process */
	for(; i < numSpes; ++i)
	{
		info[i].program_info.numPixels = initialCount;
		if(i < overflow)
			info[i].program_info.numPixels += 1;
		info[i].program_info.ray_buffer = _malloc_align(sizeof(ray_t) * 
			info[i].program_info.numPixels * samplesPerPixel, 4);

		printf("Allocating %d bytes for ray buffer at addr(%8lX:%8lX)\n",
			sizeof(ray_t)*info[i].program_info.numPixels*samplesPerPixel,
			mfc_ea2h(info[i].program_info.ray_buffer),
			mfc_ea2l(info[i].program_info.ray_buffer));
	}
}

/* frees the memory associated with initial rays in the ray buffer */
void free_raybuffers(spe_pthread_info_t *info, unsigned int numSpes)
{
	unsigned int i = 0;
	/* free space for the set number of rays in each ray group */
	for(; i < numSpes; ++i)
	{	/* free memory for each chunk sent to each spe */
		_free_align(info[i].program_info.ray_buffer);
	}
}

void init_raybuffer(float fovY, float aspectRatio, 
		float nearZ, float farZ, unsigned int width, unsigned int height,
		unsigned int samplesPerPixelSq, const scene_t *scene,
		unsigned int numSpes, spe_pthread_info_t *info)
{
	float	fovX = (fovY/2.0f) * aspectRatio;
	float	viewD = nearZ;		/* distance along N to move */
	/* float	viewPlaneHalfWidth = viewD * tan(fovX*(M_PI/180.0f));*/	
	float	viewPlaneHalfHeight = viewD * tan((fovY/2.0f)*(M_PI/180.0f));
	float	viewPlaneHalfWidth = viewPlaneHalfHeight * aspectRatio;
	/* width of "pixel" in world space - a square so pix height is the same*/
	float	pixelWidth = viewPlaneHalfWidth / (width/2.0f);
	/* now calculate width of sample square in world space */
	float	samplePixelWidth = pixelWidth / (float)samplesPerPixelSq;
	/* shorthand named variable */
	unsigned int spp = samplesPerPixelSq;
	/* ray buffer to write to - recalculated each pixel*/
	ray_t 	*raybuffer = 0;
	/* pixel index in ray buffer to write to */
	unsigned int pixIndex;
	/* offset due to super sampling in ray buffer */
	unsigned int samplePixOffset;
	/* ray buffer index of each ray */
	unsigned int rayBufferIndex;
	/* pointer to ray being generated */
	ray_t	*curr_ray;

	vector4_t tmp;		/* used for intermediate values for translation */
	/* counting variables that are reused */
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int m = 0;
	
	float uBaseDistance;	/* x offset from center of view plane for each pixel*/
	float vBaseDistance;	/* y offset from center of view plane for each pixel*/
	float uDistance;	/* final u position of ray origin on view place */
	float vDistance;	/* final v position of ray origin on view plane */ 
	float uSampleOffset;	/* offset on (x)u due to super sampling */
	float vSampleOffset;	/* offset on (y)v due to super sampling */
	
	/* create space for the set number of rays in each ray group */
	for(; j < height; ++j)
	{	/* for each row */
		/* calculate the v shift */
		vBaseDistance = (((height/2.0f) - j)/(height/2.0f)) * (float)viewPlaneHalfHeight;
		for(i = 0; i < width; ++i)
		{	/* for each ray group in row */			
			/* raygroup = raybuffer[i + j*width]; */
			/* select the proper ray buffer to write to based on pixel position*/
			raybuffer = info[(i+j*width) % numSpes].program_info.ray_buffer;
			/* calculate which numbered pixel this is for the current spe */
			pixIndex = (i+j*width) / numSpes;
	
			/* calculate the u shift */
			uBaseDistance = ((i-(width/2.0f))/(width/2.0f)) * (float)viewPlaneHalfWidth;
			
			for(m = 0; m < spp; ++m)
			{	/* for each row in ray group */
				vSampleOffset = samplePixelWidth/2.0f + m*samplePixelWidth;
				for(k = 0; k < spp; ++k)
				{	/* for each ray in row */
					samplePixOffset = k+m*spp;
					rayBufferIndex = pixIndex * samplesPerPixelSq + samplePixOffset;
					curr_ray = &raybuffer[rayBufferIndex];

					uSampleOffset = samplePixelWidth/2.0f + k*samplePixelWidth;
					uDistance = uBaseDistance + uSampleOffset;
					vDistance = vBaseDistance - vSampleOffset;

					/* move into the scene from eye point along the N vector */
					vec4_scale(&tmp, (vector4_t *)&scene->N, viewD);
					vec4_add(&curr_ray->origin, &scene->eyePos, &tmp);
					/* now move along the U vector */
					vec4_scale(&tmp, (vector4_t *)&scene->U, uDistance);
					vec4_add(&curr_ray->origin, &curr_ray->origin, &tmp);
					/* now move along V vector */
					vec4_scale(&tmp, (vector4_t *)&scene->V, vDistance);
					vec4_add(&curr_ray->origin, &curr_ray->origin, &tmp);
					
					/* now origin of ray is properly placed in the 3D world */
					/* we can calcuate the direction of the ray by subtracting
						the eye position from the origin point on the view plane */
					vec4_sub(&curr_ray->direction, &curr_ray->origin,
						&scene->eyePos);

					curr_ray->magnitude = vec4_magnitude(&curr_ray->direction);
					/* use the fact that we already have the magnitude for speed up 
						of normalization */
					vec4_scale(&curr_ray->direction, &curr_ray->direction,
						1.0f / curr_ray->magnitude);
				}
			}
		}
	}
}

/* calculates the color of an individual pixel value */
color_t* get_pixel_color(color_t *colorout, const ray_t *rays,
						unsigned int nRays, const scene_t *scene)
{
	unsigned int i = 0;
	color_t color;

	/* trace all rays in ray group and average color values */
	for(; i < nRays; ++i)
	{
		/* add color to accumulated color */
		color_add(colorout, colorout, &color, 0);
	}

	/* divide values by number of points being sampled */
	return color_scale(colorout, colorout, 1.0f/(float)nRays, 0);
}

/* fill in attributes for generating rays on the fly */
void prepare_scene(scene_t *scene, unsigned int width, unsigned int height,
		unsigned int sqrtSpp, float fovY, float aspectRatio, float nearZ)
{
	
	float	fovX = (fovY/2.0f) * aspectRatio;
	scene->viewDistance = nearZ;		/* distance along N to move */
	scene->viewPlaneHalfHeight = scene->viewDistance * tan((fovY/2.0f)*(M_PI/180.0f));
	scene->viewPlaneHalfWidth = scene->viewPlaneHalfHeight * aspectRatio;
	/* width of "pixel" in world space - a square so pix height is the same*/
	scene->sppWidth = (scene->viewPlaneHalfWidth / (width/2.0f)) / (float)sqrtSpp;
	scene->frameBufferWidth = width;
	scene->frameBufferHeight = height;
	scene->sqrtSpp = sqrtSpp;
}

/* thread function that actually runs spe executables */
void * run_spe_raytrace(void *thread_arg)
{
	int ret;
	unsigned int entry = SPE_DEFAULT_ENTRY;
	spe_pthread_info_t *info = (spe_pthread_info_t *)thread_arg;

	/*ret = spe_context_run(info->spe_context, &entry, SPE_RUN_USER_REGS, */
	ret = spe_context_run(info->spe_context, &entry, 0,
		&info->program_info,
		(void *)(unsigned long long)info->program_info.speId,
		&info->spe_stop);
	if(ret < 0)
	{
		perror("spe_context_run");
		return 0;
	}

	return 0;
}

/* called to start the ray tracing process */
void raytrace(unsigned int *buffer, scene_t *scene,
		float fovY, float aspectRatio, float nearZ, float farZ,
		unsigned int width,	unsigned int height,
		unsigned int samplesPerPixel, unsigned int depth,
		unsigned int numSpes)
{
	unsigned int i;		/* counting variable */
	unsigned int initialCount = (width * height) / numSpes;
	unsigned int overflow = (width * height) % numSpes;
	int ret;		/* used for functions called from here */
	unsigned int numSpesDone = numSpes;	/* used to know when all pixels have arrived */

	/* immediate color observed */
	color_t color;

#if defined(_DEBUG) && DEBUG > 2
	printf("Sizeof spe_program_info_t:\t%d\n", sizeof(spe_program_info_t));
#endif

	prepare_scene(scene, width, height, samplesPerPixel, fovY, aspectRatio, nearZ);
	/* initialize each spe's basic program info */
	for(i = 0; i < numSpes; ++i)
	{
		spe_thread[i].program_info.numPixels = initialCount;
		if(i < overflow)
			spe_thread[i].program_info.numPixels += 1;
		spe_thread[i].program_info.speId = i;
		spe_thread[i].program_info.numSpes = numSpes;
		spe_thread[i].program_info.samplesPerPixel = samplesPerPixel * samplesPerPixel;
		spe_thread[i].program_info.depth = depth;
		spe_thread[i].program_info.scene = (void *)scene;
		spe_thread[i].program_info.frame_buffer = (void *)buffer;
		completed_pixels[i] = 0;
	}

	/* allocate memory for the ray buffer each spe will read from */
	/* generate initial rays using aribtrary view plane */
/*
	init_spe_program_raybuffers(width, height, samplesPerPixel * samplesPerPixel,
					 numSpes, spe_thread);
	init_raybuffer(fovY, aspectRatio, nearZ, farZ, 
		width, height, samplesPerPixel, scene, numSpes, spe_thread);
*/

	/* load spu executable from disk */
	spe_code = spe_image_open("./spu_raytrace");
	if(!spe_code)
	{
		perror("spe_image_open");
		exit(1);
	}

	for(i = 0; i < numSpes; ++i)
	{
		/* create each spe context */
		spe_thread[i].spe_context = spe_context_create(0, 0);
		if(!spe_thread[i].spe_context)
		{	/* if there was an error creating spe context, output error and exit */
			perror("spe_context_create");
			exit(1);
		}

		/* load spe program to each spe's local store */
		ret = spe_program_load(spe_thread[i].spe_context, spe_code);
		if(ret)
		{
			perror("spe_program_load");
			exit(1);
		}

		/* create pthreads that will in turn execute the spe programs */
		ret = pthread_create(&pthreads[i], 0, run_spe_raytrace,
					(void *)&spe_thread[i]);
		if(ret)
		{
			perror("pthread_create");
			exit(1);
		}
	}

#ifndef _PPE_NOWAIT
	/* handle incoming pixels from SPE outbound mailboxes */
	while(numSpesDone)
	{	/* check every spe's mailbox */
		for(i = 0; i < numSpes; ++i)
		{
			/* dont read if spe is done writing */
			if(completed_pixels[i] == spe_thread[i].program_info.numPixels)
				continue;
			
			/* try to read one pixel */
			ret = spe_out_mbox_read(spe_thread[i].spe_context,
				&buffer[i + completed_pixels[i] * numSpes],1);
			if(ret < 0)
			{
				perror("spe_out_mbox_read");
			}
			else
			{	/* ret might be 0 - if it is, then completed pixels wont
				 * increment and the mailbox will try to read to the same
				 * location again anyways */
				completed_pixels[i] += ret;
				
				/* if this read completes this spe, decrement this counter */
				if(completed_pixels[i] == spe_thread[i].program_info.numPixels)
					numSpesDone--;
			}
		}
	}
#endif

	/* wait for spes to finish - they should be after the previous loop finishes */
	for(i = 0; i < numSpes; ++i)
	{
		/* wait for spe to terminate */
		pthread_join(pthreads[i], 0);
		/* destroy spe context */
		ret = spe_context_destroy(spe_thread[i].spe_context);
		if(ret)
		{
			perror("spe_context_destroy");
		}
	}

	/* close the spe program image */
	spe_image_close(spe_code);

	/* clean up allocated memory in spe_pthread_info_t */
	/*free_raybuffers(spe_thread, numSpes);*/

}
