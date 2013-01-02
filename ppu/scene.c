/* David Oguns
 * Computer Graphics II
 * March 29, 2008
 * scene.c
 * Ray Tracing Project
 *
 * This file contains the definitions for functions associated with scene management.
 */

#define _CRT_SECURE_NO_WARNINGS

#if defined(__SPU__) || defined(__PPU__)
	/* #include <libmisc.h> */
	#include <malloc_align.h>
	#include <free_align.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"

/* load scene and camera properties from file */
int parse_scene(const char *filename, scene_t *scene)
{
	/* open file for reading */
	FILE* fp = fopen(filename, "r");
	char string_buffer[STRING_BUFFER_SIZE];
	char ch = '\0';		/*for flushing comment lines*/

	if(!fp)
	{
		printf("Error opening file {%s} for reading.\n", filename);
		return 0;
	}
	
	/* scan while not end of file */
	while(!feof(fp))
	{
		/* scan the next string found in file
		 * ignore scan if error is returned */
		if(fscanf(fp, "%s", string_buffer) > 0)
		{
			/* printf("Token found:\t %s\n", string_buffer); */
			/* if we read in a comment, flush rest of line */
			if(string_buffer[0] == '#')
			{
				/* printf("Flushing comment...\n"); */
				/* read every character until newline is read 
 * 				 * or until fscanf stops reading for some
 * 				 * reason like eof */
				while(ch != '\n' && fscanf(fp, "%c", &ch) > 0);
				ch = '\0'; /* finished */
			}
			else 
			{
				if(!strcmp(string_buffer, "camera"))
				{	/* if we read the camera token */
					
				}
				else if(strcmp(string_buffer, ""))
				{

				}
				else if(strcmp(string_buffer, ""))
				{

				}
				else if(strcmp(string_buffer, ""))
				{

				}
				else if(strcmp(string_buffer, ""))
				{

				}
			}
		}
	}

	
	/* close file */
	fclose(fp);

	/* hard coded scene description */
	scene->ldMax = 100.0f;
	scene->lMax = 1000.0f;

	scene->bgColor.r = .4470588f;
	scene->bgColor.g = .6274501f;
	scene->bgColor.b = .8705882f;

	scene->nLights = 1;
#if defined(__SPU__) || defined(__PPU__)
	/* use malloc align so DMA can transfer this chunk of memory */
	scene->lights = _malloc_align(sizeof(pointlight_t) * scene->nLights, 4);
#else
	scene->lights = malloc(sizeof(pointlight_t) * scene->nLights);
#endif

	scene->nObjects = 3;
#if defined(__SPU__) || defined(__PPU__)
	scene->objects = _malloc_align(sizeof(object3d_t) * scene->nObjects, 4);
#else
	scene->objects = malloc(sizeof(object3d_t) * scene->nObjects);
#endif
	
#if !defined(__SPU__) && !defined(__PPU__)
	scene->objects[0].debugName = "Sphere 1";
#endif
	scene->objects[0].sphr_obj.center.x = 0.0f;
	scene->objects[0].sphr_obj.center.y = 5.0f;
	scene->objects[0].sphr_obj.center.z = -6.0f;
	scene->objects[0].sphr_obj.center.w = 1.0f;
	scene->objects[0].sphr_obj.radius = 1.15f;
	scene->objects[0].material.colors[MATERIAL_DIFFUSECOLOR].r = 1.0f;
	scene->objects[0].material.colors[MATERIAL_DIFFUSECOLOR].b = 1.0f;
	scene->objects[0].material.colors[MATERIAL_DIFFUSECOLOR].g = 1.0f;
	scene->objects[0].material.colors[MATERIAL_SPECULARCOLOR].r = 1.0f;
	scene->objects[0].material.colors[MATERIAL_SPECULARCOLOR].b = 1.0f;
	scene->objects[0].material.colors[MATERIAL_SPECULARCOLOR].g = 1.0f;
	scene->objects[0].material.phong_ke = 20.0f;
	scene->objects[0].material.phong_kd = 0.075f;
	scene->objects[0].material.phong_ks = 0.2f;
	scene->objects[0].material.phong_ka = 0.075f;
	scene->objects[0].material.kr = 0.01f;
	scene->objects[0].material.kt = 0.85f;
	scene->objects[0].material.n = 0.95f;
	scene->objects[0].geometryType = GEOMETRY_SPHERE;

#if !defined(__SPU__) && !defined(__PPU__)
	scene->objects[1].debugName = "Sphere 2";
#endif
	scene->objects[1].sphr_obj.center.x = -1.25f;
	scene->objects[1].sphr_obj.center.y = 3.75f;
	scene->objects[1].sphr_obj.center.z = -7.25f;
	scene->objects[1].sphr_obj.center.w = 1.0f;
	scene->objects[1].sphr_obj.radius = 1.0f;
	scene->objects[1].material.colors[MATERIAL_DIFFUSECOLOR].r = 0.7f;
	scene->objects[1].material.colors[MATERIAL_DIFFUSECOLOR].b = 0.7f;
	scene->objects[1].material.colors[MATERIAL_DIFFUSECOLOR].g = 0.7f;
	scene->objects[1].material.colors[MATERIAL_SPECULARCOLOR].r = 1.0f;
	scene->objects[1].material.colors[MATERIAL_SPECULARCOLOR].b = 1.0f;
	scene->objects[1].material.colors[MATERIAL_SPECULARCOLOR].g = 1.0f;
	scene->objects[1].material.phong_ke = 20.0f;
	scene->objects[1].material.phong_kd = 0.25f;
	scene->objects[1].material.phong_ks = 1.0f;
	scene->objects[1].material.phong_ka = 0.15f;
	scene->objects[1].material.kr = 0.75f;
	scene->objects[1].material.kt = 0.0f;
	scene->objects[1].material.n = 1.0f;
	scene->objects[1].geometryType = GEOMETRY_SPHERE;

#if !defined(__SPU__) && !defined(__PPU__)
	scene->objects[2].debugName = "Floor";
#endif
	scene->objects[2].poly_obj.nVerticies = 4;

#if defined(__SPU__) || defined(__PPU__)
	scene->objects[2].poly_obj.vertex = _malloc_align(
		sizeof(vector4_t)*scene->objects[2].poly_obj.nVerticies, 4);
#else
	scene->objects[2].poly_obj.vertex = malloc(
		sizeof(vector4_t)*scene->objects[2].poly_obj.nVerticies);
#endif

	scene->objects[2].poly_obj.vertex[0].x = 7.0f;
	scene->objects[2].poly_obj.vertex[0].y = 0.0f;
	scene->objects[2].poly_obj.vertex[0].z = 0.0f;
	scene->objects[2].poly_obj.vertex[0].w = 1.0f;

	scene->objects[2].poly_obj.vertex[1].x = 7.0f;
	scene->objects[2].poly_obj.vertex[1].y = 0.0f;
	scene->objects[2].poly_obj.vertex[1].z = -100.0f;
	scene->objects[2].poly_obj.vertex[1].w = 1.0f;

	scene->objects[2].poly_obj.vertex[2].x = -15.0f;
	scene->objects[2].poly_obj.vertex[2].y = 0.0f;
	scene->objects[2].poly_obj.vertex[2].z = -100.0f;
	scene->objects[2].poly_obj.vertex[2].w = 1.0f;

	scene->objects[2].poly_obj.vertex[3].x = -15.0f;
	scene->objects[2].poly_obj.vertex[3].y = 0.0f;
	scene->objects[2].poly_obj.vertex[3].z = 0.0f;
	scene->objects[2].poly_obj.vertex[3].w = 1.0f;
	
	/* generate the plane for this polygon given the first three points */
	poly_plane(&scene->objects[2].poly_obj.plane, &scene->objects[2].poly_obj);

	scene->objects[2].material.colors[MATERIAL_DIFFUSECOLOR].r = 0.0f;
	scene->objects[2].material.colors[MATERIAL_DIFFUSECOLOR].b = 0.0f;
	scene->objects[2].material.colors[MATERIAL_DIFFUSECOLOR].g = 1.0f;
	scene->objects[2].material.colors[MATERIAL_SPECULARCOLOR].r = 1.0f;
	scene->objects[2].material.colors[MATERIAL_SPECULARCOLOR].b = 1.0f;
	scene->objects[2].material.colors[MATERIAL_SPECULARCOLOR].g = 1.0f;
	scene->objects[2].material.phong_ke = 2.0f;
	scene->objects[2].material.phong_kd = 0.7f;
	scene->objects[2].material.phong_ks = 0.2f;
	scene->objects[2].material.phong_ka = 0.1f;
	scene->objects[2].material.kr = 0.0f;
	scene->objects[2].material.kt = 0.0f;
	scene->objects[2].material.n = 1.0f;
	scene->objects[2].geometryType = GEOMETRY_POLYGON;

	/* set the light properties */
	scene->lights[0].color.r = 1.0f;
	scene->lights[0].color.g = 1.0f;
	scene->lights[0].color.b = 1.0f;
	scene->lights[0].position.x = 1.0f;
	scene->lights[0].position.y = 8.0f;
	scene->lights[0].position.z = 1.0f;
	scene->lights[0].position.w = 1.0f;
	scene->lights[0].range = 150.0f;

	
	/* set the light properties */
/*
	scene->lights[1].color.r = 0.5f;
	scene->lights[1].color.g = 0.5f;
	scene->lights[1].color.b = 0.5f;
	scene->lights[1].position.x = -4.0f;
	scene->lights[1].position.y = 8.0f;
	scene->lights[1].position.z = 1.0f;
	scene->lights[1].position.w = 1.0f;
	scene->lights[1].range = 150.0f;
*/
	

	/* add ambient light to scene */
	scene->ambientLightColor.r = 1.0f;
	scene->ambientLightColor.g = 1.0f;
	scene->ambientLightColor.b = 1.0f;


	/* view matrix */
	scene->eyePos.x = 0.0f;
	scene->eyePos.y = 4.5f;
	scene->eyePos.z = 0.0f;
	scene->eyePos.w = 1.0f;

	scene->lookAt.x = 0.0f;
	scene->lookAt.y = 0.0f;
	scene->lookAt.z = -1.0f;
	scene->lookAt.w = 1.0f;
	vec4_add(&scene->lookAt, &scene->eyePos, &scene->lookAt);

	scene->upVec.x = 0.0f;
	scene->upVec.y = 1.0f;
	scene->upVec.z = 0.0f;
	scene->upVec.w = 1.0f;

	/* define N(into the scene) first */
	vec4_sub(&scene->N, &scene->lookAt, &scene->eyePos);
	vec4_normalize(&scene->N);
	/* define V(up vector) second */
	vec4_set(&scene->V, (float *)&scene->upVec);
	/* define U(right vector) third */
	vec4_cross(&scene->U, &scene->N, &scene->V);

/*
	mat4_viewv(&scene->viewMatrix,
		&scene->eyePos,
		&scene->lookAt,
		&scene->upVec);
*/
#if defined(_DEBUG)
	#if _DEBUG > 2
		printf("Main sizeof(scene_t): %d\n", sizeof(scene_t));
	#endif
	#if _DEBUG > 1
		printf("Main Scene:\n\tnObjects:\t%d\n\tnLights:\t%d\n\tbgColor:\t(%f, %f, %f,)\n\tU:\t(%f, %f, %f)\n\tV:\t(%f, %f, %f)\n\tN:\t(%f, %f, %f)\n",
			scene->nObjects, scene->nLights,
			scene->bgColor.r, scene->bgColor.g, scene->bgColor.b,
			scene->U.x, scene->U.y, scene->U.z,
			scene->V.x, scene->V.y, scene->V.z,
			scene->N.x, scene->N.y, scene->N.z);
	#endif
#endif

	return 1;
}

/* cleanup dynamic memory from creating scene */
void free_scene(scene_t *scene)
{
	unsigned int i = 0;
	
	/* free all lights */
#if defined(__SPU__) || defined(__PPU__)
	_free_align(scene->lights);
#else
	free(scene->lights);
#endif

	/* loop over every object */
	for(; i < scene->nObjects; ++i)
	{	/* if object is a polygon, free all of the verticies */
		if(scene->objects[i].geometryType == GEOMETRY_POLYGON)
#if defined(__SPU__) || defined (__PPU__)
			_free_align(scene->objects[i].poly_obj.vertex);
#else
			free(scene->objects[i].poly_obj.vertex);
#endif
	}

	/* free all objects */
#if defined(__SPU__) || defined (__PPU__)
	_free_align(scene->objects);
#else
	free(scene->objects);
#endif

}


