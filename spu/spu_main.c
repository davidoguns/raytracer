/* David Oguns
 * Computer Graphics II
 * Cell Ray Tracer
 * May 9, 2008
 * spu_main.c
 *
 * Entry point for the spu program used by the Cell ray tracing project.
 */
#ifdef _DEBUG
	#include <stdio.h>
	#include "ray.h"
#endif
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <malloc_align.h>
#include <free_align.h>

#include "scene.h"
#include "spu_main.h"
#include "spe_program_info.h"
#include "spu_tag_masks.h"
#include "spu_raytrace.h"

unsigned int speid;

/* loads program info - blocks until done */
void load_program_info(unsigned long long ea, spe_program_info_t *info)
{
	/* initiate DMA request for program info */
	/* spu_mfcdma64(ls_addr, ea_h, ea_l, size, tag_id, cmd); */
	spu_mfcdma64(info, mfc_ea2h(ea), mfc_ea2l(ea),
		sizeof(spe_program_info_t),
		SPUDMA_PROGRAMINFO,
		MFC_GET_CMD);

	/* wait for request to complete */
	spu_writech(MFC_WrTagMask, 1 << SPUDMA_PROGRAMINFO);
	mfc_read_tag_status_all();

	/* assign to global for debugging purposes */
	speid = info->speId;

#if defined(_DEBUG) && _DEBUG > 1
	printf("Program info:\n\tSpe ID:       %d\n\tNum Pixels:   %d\n\tSpp:          %d\n\tNum Spes      %d\n\tDepth:        %d\n",
		info->speId,
		info->numPixels,
		info->samplesPerPixel,
		info->numSpes,
		info->depth);
#endif
}

/* loads the scene using DMA - blocks until done */
void load_scene(unsigned long long ea, scene_t *scene)
{
	unsigned int i = 0;
	object3d_t *objects = 0;
	pointlight_t *lights = 0;
	point_t *v = 0;

#if defined(_DEBUG) && _DEBUG > 2
	printf("Transferring %d bytes to LSaddr(%8lX) from EAadd(%8lX:%8lX) for SCENE\n",
		sizeof(scene_t),
		&scene,
		mfc_ea2h(ea),
		mfc_ea2l(ea));
#endif
	/* DMA request for scene */
	spu_mfcdma64(scene,
		mfc_ea2h(ea),
		mfc_ea2l(ea),
		sizeof(scene_t),
		SPUDMA_SCENE,
		MFC_GET_CMD);
	
	/* wait for request to complete */
	spu_writech(MFC_WrTagMask, 1 << SPUDMA_SCENE);
	mfc_read_tag_status_all();
	
	
	/* copy over objects */
	objects = _malloc_align(sizeof(object3d_t) * scene->nObjects, 4);
#if defined(_DEBUG) && _DEBUG > 2
	printf("Transferring %d bytes to LSaddr(%8lX) from EAadd(%8lX:%8lX) for OBJECTS\n",
		sizeof(object3d_t) * scene->nObjects,
		objects,
		mfc_ea2h(scene->objects_ea),
		mfc_ea2l(scene->objects_ea));
#endif
	/* initiate DMA */
	spu_mfcdma64(objects,
		mfc_ea2h(scene->objects_ea),
		mfc_ea2l(scene->objects_ea),
		sizeof(object3d_t) * scene->nObjects,
		SPUDMA_OBJECTS,
		MFC_GET_CMD);
	
	/* copy over lights */
	lights = _malloc_align(sizeof(pointlight_t) * scene->nLights, 4);	
#if defined(_DEBUG) && _DEBUG > 2
	printf("Transferring %d bytes to LSaddr(%8X) from EAadd(%8lX:%8lX) for LIGHTS\n",
		sizeof(pointlight_t) * scene->nLights,
		lights,
		mfc_ea2h(scene->lights_ea),
		mfc_ea2l(scene->lights_ea));
#endif
	/* initiate DMA for lights */
	spu_mfcdma64(lights,
		mfc_ea2h(scene->lights_ea),
		mfc_ea2l(scene->lights_ea),
		sizeof(pointlight_t) * scene->nLights,
		SPUDMA_LIGHTS,
		MFC_GET_CMD);
	
	/* wait for objects to complete */
	spu_writech(MFC_WrTagMask, 1 << SPUDMA_OBJECTS);
	mfc_read_tag_status_all();
	/* assign local store pointer to objects */
	scene->objects = objects;

	/* iterate each object locally */
	for(; i < scene->nObjects; ++i)
	{
		if(objects[i].geometryType == GEOMETRY_POLYGON)
		{
			/* allocate memory for vertex */
			v = _malloc_align(sizeof(point_t) 
				* objects[i].poly_obj.nVerticies, 4);
			/* initiate DMA to get verticies */
			spu_mfcdma64(v,
				mfc_ea2h(objects[i].poly_obj.vertex_ea),
				mfc_ea2l(objects[i].poly_obj.vertex_ea),
				sizeof(point_t)
				* objects[i].poly_obj.nVerticies,
				SPUDMA_VERTEXES,
				MFC_GET_CMD);
			/* assign local store pointer - WARNING - safe? */
			objects[i].poly_obj.vertex = v;				
		}
	}
	
	/* wait for all DMA to finish (vertexes, lights) */
	spu_writech(MFC_WrTagMask, 1 << SPUDMA_LIGHTS |
				1 << SPUDMA_VERTEXES );
	mfc_read_tag_status_all();
	/* assign local store lights pointer */
	scene->lights = lights;
}

/* cleans up dynamically allocated memory in scene */
void free_scene(scene_t *scene)
{
	unsigned int i = 0;

	/* clean up lights */
	_free_align(scene->lights);
	
	for(; i < scene->nObjects; ++i)
	{	/* if object is polygon, free vertexes */
		if(scene->objects[i].geometryType == GEOMETRY_POLYGON)
		{
			_free_align(scene->objects[i].poly_obj.vertex);
		}
	}
	
	/* clean up objects */
	_free_align(scene->objects);
}

void preview_scene(scene_t *scene)
{
	unsigned int i = 0;
	unsigned int j;

	printf("SPE%d Scene:\n\tnObjects:\t%d\n\tnLights:\t%d\n\tbgColor:\t(%f, %f, %f)\n\tU:\t(%f, %f, %f)\n\tV:\t(%f, %f, %f)\n\tN:\t(%f, %f, %f)\n",
		speid,
		scene->nObjects, scene->nLights,
		scene->bgColor.r, scene->bgColor.g, scene->bgColor.b,
		scene->U.x, scene->U.y, scene->U.z,
		scene->V.x, scene->V.y, scene->V.z,
		scene->N.x, scene->N.y, scene->N.z);

	for(; i < scene->nLights; ++i)
	{
		printf("Light(%d) - pos(%f, %f, %f)  color(%f, %f, %f)\n",
			i, scene->lights[i].position.x, 
			scene->lights[i].position.y, scene->lights[i].position.z,
			scene->lights[i].color.r, scene->lights[i].color.g,
			scene->lights[i].color.b);
	}

	for(i = 0; i < scene->nObjects; ++i)
	{
		if(scene->objects[i].geometryType == GEOMETRY_POLYGON)
		{
			printf("Object(%d) is a polygon with %d verticies.\n",
				scene->objects[i].poly_obj.nVerticies);
			for(j = 0; j < scene->objects[i].poly_obj.nVerticies; ++j)
			{
				printf("\tVertex[%d] = (%f, %f, %f).\n", j,
					scene->objects[i].poly_obj.vertex[j].x,
					scene->objects[i].poly_obj.vertex[j].y,
					scene->objects[i].poly_obj.vertex[j].z);

			}
		}
		else if(scene->objects[i].geometryType == GEOMETRY_SPHERE)
		{
			printf("Object(%d) is a sphere with radius %f and center (%f, %f, %f).\n",
			i,
			scene->objects[i].sphr_obj.radius,
			scene->objects[i].sphr_obj.center.x,
			scene->objects[i].sphr_obj.center.y,
			scene->objects[i].sphr_obj.center.z);
		}
		else
		{
			printf("Object(%d) is of unknown type!\n", i);
		}
	}
}

int main(unsigned long long spe, unsigned long long argp,
	unsigned long long envp)
{
	spe_program_info_t	info;
	scene_t			scene;

#if defined(_DEBUG)
	printf("SPU params {%d, %d, %d}\n",
		(unsigned int)spe, (unsigned int)argp, 
		(unsigned int)envp, sizeof(spe_program_info_t));
#if _DEBUG > 2
	printf("SPU sizeof object3d_t: %d\n", sizeof(object3d_t));
	printf("SPU sizeof material_t: %d\n", sizeof(material_t));
	printf("SPU sizeof plane_t: %d\n", sizeof(plane_t));
	printf("SPU sizeof polygon_t: %d\n", sizeof(polygon_t));
	printf("SPU sizeof sphere_t: %d\n", sizeof(sphere_t));
	printf("SPU sizeof unsigned int: %d\n", sizeof(unsigned int));
	printf("SPU sizeof unsigned long long: %d\n", sizeof(unsigned long long));
	printf("SPU sizeof point_t*: %d\n", sizeof(point_t *));
	printf("SPU sizeof ray_t: %d\n", sizeof(ray_t));
#endif	/* _DEBUG > 1 */
#endif	/* _DEBUG */
	
	load_program_info(argp, &info);

	load_scene(info.scene_ea, &scene);

#if defined(_DEBUG) && _DEBUG > 1
	preview_scene(&scene);
#endif

	/* now start churnin rays */
	spu_raytrace(&info, &scene);

	return 0;
}
