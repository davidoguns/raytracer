/* David Oguns
 * Computer Graphics II
 * Warren Carithers
 * Cell Ray Tracer Project
 * spu_raytrace.c
 * May 14, 2008
 *
 * This file defines the main functions and algorithm that will be used for 
 * running the ray tracer on the SPUs.  It handles the DMA of reading in rays
 * (if any) and DMA of writing out pixels back to main memory.
 *
 */

#include <spu_mfcio.h>
#include <malloc_align.h>
#include <free_align.h>
#include <simdmath/powf4.h>
#include <simdmath/sqrtf4.h>

#include <libvector.h>

#include <dot_product3.h>
#include <normalize3.h>

#include "spu_raytrace.h"
#include "ray.h"

#define COLORVALUE_MAX		0xFF

extern unsigned int speid;
unsigned int MAX_DEPTH = 4;

/* returns intersection of ray with any object type */
/* two output parameters - v and distance
 * p - point of intersection
 * distance - distance to intersection point from origin of ray */
int ray_intersect_object(const ray_t* ray, const object3d_t *obj,
						 point_t *p, float *distance)
{
	switch(obj->geometryType)
	{
		case GEOMETRY_POLYGON:
			return ray_intersect_polygon(ray, &obj->poly_obj, p, distance);
		case GEOMETRY_SPHERE:
			return ray_intersect_sphere(ray, &obj->sphr_obj, p, distance);
		default:
			return 0;
	};
}

/* gets the first object this ray intersects
 * also returns the point of intersection through first paramemter
 * and distance to the point through second parameter 
 * returns null if there is no object intersected in the scene */
object3d_t *get_object3d_intersect(point_t *intersect, float *d,
					const ray_t *ray, const scene_t *scene)
{
	object3d_t		*obj = 0;
	unsigned int	i = 0;		/* counting variable iterating over objects in scene */
	float			tmpD;		/* temporary distance to last intersected object */
	point_t			tmpInt;		/* temporary intersection point to last intersected obj */

	/* first find what object ray intersects first if any */
	/* iterate over every object in the scene */
	for(i = 0; i < scene->nObjects; ++i)
	{
		if(ray_intersect_object(ray, &scene->objects[i], &tmpInt, &tmpD))
		{	/* if objects intersect, compare distance to intersection */
			/* if there is no intersection object yet, then
			 * there being an intersection at all sets this as the 
			 * current closest object */
			if(obj == 0)
			{
				*d = tmpD;
				obj = &scene->objects[i];
				vec4_set(intersect, (float *)&tmpInt);
			}	/* otherwise, we want to make sure new
			 * intersection is closer */
			else if(tmpD < *d)
			{
				*d = tmpD;
				obj = &scene->objects[i];
				vec4_set(intersect, (float *)&tmpInt);
			}
		}
	}

	return obj;
}


/* gets the first object this ray intersects excluding supplied object
 * from consideration.  This version is used for casting shadow rays
 * to prevent returning intersection with object at the start of ray.
 *
 * Returns the point of intersection through first parameter
 * and distance to the point through second parameter 
 * returns null if there is no object intersected in the scene */
object3d_t *get_object3d_intersect_excl(point_t *intersect, float *d,
				const ray_t *ray, const scene_t *scene, const object3d_t *exc)
{
	object3d_t	*obj = 0;	/* return value */
	unsigned int	i = 0;		/* counting variable iterating over objects in scene */
	float		tmpD;		/* temporary distance to last intersected object */
	point_t		tmpInt;		/* temporary intersection point */

	/* first find what object ray intersects first if any */
	/* iterate over every object in the scene */
	for(i = 0; i < scene->nObjects; ++i)
	{
		/* if this is the object we want to exclude, skip over */
		if(&scene->objects[i] == exc)
			continue;

		if(ray_intersect_object(ray, &scene->objects[i], &tmpInt, &tmpD))
		{
			/* if there is no intersection object yet, then
			 * there being an intersection at all sets this as the 
			 * current closest object */
			if(tmpD < ray->magnitude && tmpD > 0.0f)
			{
				if(obj == 0)
				{
					*d = tmpD;
					obj = &scene->objects[i];
					vec4_set(intersect, (float *)&tmpInt);
				}	/* otherwise, we want to make sure new
					 * intersection is closer */
				else if(tmpD < *d)
				{
					*d = tmpD;
					obj = &scene->objects[i];
					vec4_set(intersect, (float *)&tmpInt);
				}
			}
		}
	}

	return obj;
}


/* calculates the color at a particular shading point on a specified object
 * we pass in the scene primary to use lights, but also for casting other
 * rays.
 * obj - object being intersected
 * eye - observer of this shading point
 * pt - point of intersection on the object
 * scene - entire scene
 */
color_t* get_shade_color_phong(color_t *colorout, const object3d_t *obj,
						 const ray_t *eye, const point_t *pt,
						 const scene_t *scene, unsigned int depth)
{
	unsigned int i = 0;				/* iterative variable over nLights */
	vector4_t	N, S, V, R;			/* vectors for lighting calculations */
	ray_t		shadow, reflRay, transRay;	/* reflected and transmitted rays*/
	object3d_t	*recurseObject;			/* object hit by spawned rays */
	point_t		recurseIntersect;		/* spawned ray intersection point */
	float		recurseDistance;		/* distance to spawn ray intersection */
	color_t		recurseColor;			/* color from spawned rays */
	float		tmpFloat;			/* used in various calculations */
	color_t		objColor;			/* color of object at intersection */
	color_t		diff;				/* diffuse term sum */
	color_t		spec;				/* specular term sum */
	float		nit;				/* index of refraction ratio */

	/* get color of object at intersection point */
	get_object_color(&objColor, obj, pt);

	/* get ambient light contribution first */
	color_mult(colorout, &objColor,
			&scene->ambientLightColor, 0);
	/* here we will clamp even though ambient lighting should never
	 * be enough to saturate a color channel */
	color_scale(colorout, colorout, obj->material.phong_ka, 0);

	/* initialize specular and diffuse components to 0 */
	color_init(&diff);
	color_init(&spec);

	/* calculate relevant lighting vectors that do not change for each
	 * light source */
	/* get normal vector */
	get_object_normal(&N, obj, pt);
	/* View vector is generated by subtracting intersection from eye pos */
	/* vec4_sub(&V, &eye->origin, pt); */
	V.v = spu_sub(eye->origin.v, pt->v);
	/* normalize V */
	/* vec4_normalize(&V); */
	V.v = _normalize3(V.v);
	
	/* for each light source - cast shadow ray towards light */
	for( ; i < scene->nLights; ++i)
	{
		/* generate a ray from intersection point to light */
		/* note - magnitude of ray created is the distance to the light */
		ray_create(&shadow, pt, &scene->lights[i].position);
		/* get first object that ray intersects NOT including this object */
		ray_tinypush(&shadow, &shadow);
		recurseObject = get_object3d_intersect_excl(&recurseIntersect,
						&recurseDistance, &shadow, scene, obj);
		if(recurseObject)
		{	/* there is an object between this surface and the light */
			/* continue to next light as this one has no contribution */
			continue;
		}

		/* this light has contribution, calculate it */
		/* get S and R vectors which change for each light source */
		/* Source vector is embedded in shadow ray direction */
		vec4_set(&S, (float *)&shadow.direction);
		/*
		vec4_sub(&S, &scene->lights[i].position, pt);
		vec4_normalize(&S);
		*/
		/* Reflected vector takes a bit more to calculate */
		/* S == Ri?  */
		/* tmpFloat = vec4_dot(&S, &N); */
		tmpFloat = _dot_product3(S.v, N.v);
		/* vec4_scale(&R, &N, 2.0f*tmpFloat); */
		R.v = spu_mul(N.v, spu_splats(2.0f*tmpFloat));
		/* vec4_sub(&R, &R, &S); */
		R.v = spu_sub(R.v, S.v);
		/* is this needed? */

		/* only apply specular and diffuse components from light if 
		 * surface normal points towards light
		 * i.e. not a back face of surface */
		if( tmpFloat <= 0.0f)
			continue;

		/* tmpFloat = vec4_dot(&S, &N);*/
		tmpFloat = _dot_product3(S.v, N.v);
		diff.r += scene->lights[i].color.r * tmpFloat;
		diff.g += scene->lights[i].color.g * tmpFloat;
		diff.b += scene->lights[i].color.b * tmpFloat;

		/* tmpFloat = vec4_dot(&R, &V); */
		tmpFloat = _dot_product3(R.v, V.v);
		if(tmpFloat < 0.0f)
			tmpFloat = 0.0f;
		tmpFloat = _powf4(spu_splats(tmpFloat), spu_splats(obj->material.phong_ke))[0];
		/* tmpFloat = pow(tmpFloat, obj->material.phong_ke); */
		spec.r += scene->lights[i].color.r * tmpFloat;
		spec.g += scene->lights[i].color.g * tmpFloat;
		spec.b += scene->lights[i].color.b * tmpFloat;
		
		/*
		colorout->r += scene->lights[i].color.r * 
			obj->material.colors[MATERIAL_DIFFUSECOLOR].r;
		colorout->g += scene->lights[i].color.g * 
			obj->material.colors[MATERIAL_DIFFUSECOLOR].g;
		colorout->b += scene->lights[i].color.b * 
			obj->material.colors[MATERIAL_DIFFUSECOLOR].b;
		*/
	}
/*
	color_clamp(&diff);
	color_clamp(&spec);
*/

	colorout->r += 	obj->material.phong_kd * 
		diff.r * objColor.r +
		obj->material.phong_ks *
		spec.r * obj->material.colors[MATERIAL_SPECULARCOLOR].r;
	colorout->g += 	obj->material.phong_kd * 
		diff.g * objColor.g +
		obj->material.phong_ks *
		spec.g * obj->material.colors[MATERIAL_SPECULARCOLOR].g;
	colorout->b += 	obj->material.phong_kd * 
		diff.b * objColor.b +
		obj->material.phong_ks *
		spec.b * obj->material.colors[MATERIAL_SPECULARCOLOR].b;


	/* add reflection stuff */
	if(obj->material.kr != 0.0f && depth != MAX_DEPTH)
	{	/* calculate reflection ray and get color at that point */
	
		/* origin of spawned ray is *this* intersection point */
		vec4_set(&reflRay.origin, (float *)pt);
		/* start with a magnitude of zero */
		reflRay.magnitude = 9999999.9f;

		/* find reflection of eye vector */
		/* tmpFloat = vec4_dot(&V, &N); */
		tmpFloat = _dot_product3(V.v, N.v);
		/* vec4_scale(&reflRay.direction, &N, 2.0f*tmpFloat); */
		reflRay.direction.v = spu_mul(N.v, spu_splats(2.0f*tmpFloat));
		/* vec4_sub(&reflRay.direction, &reflRay.direction, &V); */
		reflRay.direction.v = spu_sub(reflRay.direction.v, V.v);
		
		/* normalize the vector this way */
/*
		tmpFloat = vec4_magnitude(&reflRay.direction);
		vec4_scale(&reflRay.direction, &reflRay.direction, 1.0f / tmpFloat);
*/

		/* get recurse object */
		ray_tinypush(&reflRay, &reflRay);
/*
		recurseObject = get_object3d_intersect(&recurseIntersect,
						&recurseDistance, &reflRay, scene);
*/
		recurseObject = get_object3d_intersect_excl(&recurseIntersect,
						&recurseDistance, &reflRay, scene, 0);


		if(recurseObject)
		{
			/* get shade color */
			get_shade_color_phong(&recurseColor, recurseObject, &reflRay,
					&recurseIntersect, scene, depth+1);

			colorout->r += obj->material.kr * recurseColor.r;
			colorout->g += obj->material.kr * recurseColor.g;
			colorout->b += obj->material.kr * recurseColor.b;
		}
		else
		{	/* mix with background color */
			colorout->r += obj->material.kr * scene->bgColor.r;
			colorout->g += obj->material.kr * scene->bgColor.g;
			colorout->b += obj->material.kr * scene->bgColor.b;
		}

	}
	if(obj->material.kt != 0.0f && depth != MAX_DEPTH)
	{	/* calculate transmitted ray and get color at that point */
		/* assume indices of refraction ratio is heading into object */
		nit = 1.0f / obj->material.n;
		/* vec4_scale(&transRay.direction, &eye->direction, nit); */
		transRay.direction.v = spu_mul(eye->direction.v, spu_splats(nit));

		/* use transRay.origin as a tmp vector - (-D) */
		/* tmpFloat = vec4_dot(&V, &N); */
		tmpFloat = _dot_product3(V.v, N.v);
		if(tmpFloat < 0.0f)
		{	/* we are inside the object moving out */
			nit = 1.0f / nit;
			/* WARNING - N is being changed here */
			/* vec4_scale(&N, &N, -1.0f); */
			N.v = spu_mul(N.v, spu_splats(-1.0f));
		}

		/* use transRay.magnitude as tmpFloat2 */
		transRay.magnitude = 1 + nit * nit * 
				(tmpFloat*tmpFloat - 1);
		/* total internal reflection */
		if(transRay.magnitude < 0)
		{
			/* origin of spawned ray is *this* intersection point */
			vec4_set(&reflRay.origin, (float *)pt);
			/* start with a magnitude of zero */
			reflRay.magnitude = 999999.0f;
	
			/* find reflection of eye vector */
			/* tmpFloat = vec4_dot(&V, &N); */
			tmpFloat = _dot_product3(V.v, N.v);
			/*vec4_scale(&reflRay.direction, &N, 2.0f*tmpFloat);*/
			reflRay.direction.v = spu_mul(N.v, spu_splats(2.0f*tmpFloat));
			/*vec4_sub(&reflRay.direction, &reflRay.direction, &V);*/
			reflRay.direction.v = spu_sub(reflRay.direction.v, V.v);
			
			/* normalize the vector this way */
	/*
			tmpFloat = vec4_magnitude(&reflRay.direction);
			vec4_scale(&reflRay.direction, &reflRay.direction, 1.0f / tmpFloat);
	*/
	
			/* get recurse object */
			ray_tinypush(&reflRay, &reflRay);
/*
			recurseObject = get_object3d_intersect(&recurseIntersect,
							&recurseDistance, &reflRay, scene);
*/
			recurseObject = get_object3d_intersect_excl(&recurseIntersect,
							&recurseDistance, &reflRay, scene, 0);
	
			if(recurseObject)
			{
				/* get shade color */
				get_shade_color_phong(&recurseColor, recurseObject, &reflRay,
						&recurseIntersect, scene, depth+1);
	
				colorout->r += obj->material.kt * recurseColor.r;
				colorout->g += obj->material.kt * recurseColor.g;
				colorout->b += obj->material.kt * recurseColor.b;
			}
			else
			{	/* mix with background color */
				colorout->r += obj->material.kt * scene->bgColor.r;
				colorout->g += obj->material.kt * scene->bgColor.g;
				colorout->b += obj->material.kt * scene->bgColor.b;
			}
		}
		else
		{
			/* tmpFloat = (-D . N), transRay.magnitude = det */
			tmpFloat = 1.0f/obj->material.n * tmpFloat - 
					_sqrtf4(spu_splats(transRay.magnitude))[0];
			/*		sqrt(transRay.magnitude);*/
			/* used again as tmp vector */
			/*vec4_scale(&transRay.origin, &N, tmpFloat);*/
			transRay.origin.v = spu_mul(N.v, spu_splats(tmpFloat));
			/*vec4_add(&transRay.direction, &transRay.direction, &transRay.origin);*/
			transRay.direction.v = spu_add(transRay.direction.v, transRay.origin.v);

			/* now set origin appropriately */
			vec4_set(&transRay.origin, (float *)pt);
			transRay.magnitude = 99999.9f;
			/*
			printf("costheta(V, T):\t%f\n", vec4_dot(&V, &transRay.direction));
			printf("V.direction:\t(%f, %f, %f) - %f\n", V.x, V.y, V.z, vec4_magnitude(&V));
			printf("T.direction:\t(%f, %f, %f) - %f\n", transRay.direction.x, transRay.direction.y, transRay.direction.z, vec4_magnitude(&transRay.direction)); */


/*
			recurseObject = get_object3d_intersect(&recurseIntersect,
				&recurseDistance, &transRay, scene);
*/
			/* get recurse object */
			ray_tinypush(&transRay, &transRay);
			recurseObject = get_object3d_intersect_excl(&recurseIntersect,
				&recurseDistance, &transRay, scene, 0);
		
			if(recurseObject)
			{
				/* get shade color */
				get_shade_color_phong(&recurseColor, recurseObject, &transRay,
						&recurseIntersect, scene, depth+1);
	
				colorout->r += obj->material.kt * recurseColor.r;
				colorout->g += obj->material.kt * recurseColor.g;
				colorout->b += obj->material.kt * recurseColor.b;
			}
			else
			{	/* mix with background color */
				colorout->r += obj->material.kt * scene->bgColor.r;
				colorout->g += obj->material.kt * scene->bgColor.g;
				colorout->b += obj->material.kt * scene->bgColor.b;
			}
		}
		
	}

	/* ensure colors don't spill over max values on each channel */
	color_clamp(colorout);
	return colorout;
}

/* gets the color at the first shading point the ray intersects */
color_t* get_ray_color(color_t *colorout, const ray_t *ray,
					   const scene_t *scene)
{
	point_t			intersect;	/* intersection point if we find one */
	object3d_t		*obj = 0;	/* object being intersected if any */
	float			distance;	/* gets distance to intersection */

	/* get first object ray intersects */
	obj = get_object3d_intersect(&intersect, &distance, ray, scene);

	/* after we iterate over every object in the scene, let's
	 * examine the results */
	if(obj == 0)
	{	/* background, color of ray is background color */
		return color_copy(colorout, &scene->bgColor);
	}
	else
	{	/* there was an intersection with object */
		return get_shade_color_phong(colorout, obj, ray, &intersect, scene, 0);
	}
}

/* calculates the color of an individual pixel value */
color_t* get_pixel_color(color_t *colorout, unsigned int x, unsigned int y,
			const scene_t *scene)
{
	unsigned int i;
	unsigned int j = 0;
	float scale = 1.0f /(float)(scene->sqrtSpp * scene->sqrtSpp);
	color_t color;
	ray_t	ray;
	/* base target point on view plane */
	point_t target;	
	vector4_t shift;
	vector4_t tmp;

	/* trace all rays in ray group and average color values */
	for(; j < scene->sqrtSpp; ++j)
	{
		for(i = 0; i < scene->sqrtSpp; ++i)
		{
			/* N - move by viewDistance into scene - same for every pixel */
			/* vec4_scale(&tmp, &scene->N, scene->viewDistance); */
			tmp.v = spu_mul(scene->N.v, spu_splats(scene->viewDistance));
			vec4_set(&shift, (float *)&tmp);

			/* U - move left/right based on X pos and spp */
			/*vec4_scale(&tmp, &scene->U, 
				(((x-(scene->frameBufferWidth/2.0f))/(scene->frameBufferWidth/2.0f)) * scene->viewPlaneHalfWidth) 
				+ (scene->sppWidth/2.0f + i*scene->sppWidth)); */
			tmp.v = spu_mul(scene->U.v, spu_splats(
				(((x-(scene->frameBufferWidth/2.0f))/(scene->frameBufferWidth/2.0f)) * scene->viewPlaneHalfWidth) 
				+ (scene->sppWidth/2.0f + i*scene->sppWidth)));
			/*vec4_add(&shift, &shift, &tmp);*/
			shift.v = spu_add(shift.v, tmp.v);

			/* N - move up/down based on Y pos and spp */
			/*vec4_scale(&tmp, &scene->V, 
				((((scene->frameBufferHeight/2.0f) - y)/(scene->frameBufferHeight/2.0f)) * scene->viewPlaneHalfHeight)
				+ (scene->sppWidth/2.0f + j*scene->sppWidth)); */
			tmp.v = spu_mul(scene->V.v, spu_splats(
				((((scene->frameBufferHeight/2.0f) - y)/(scene->frameBufferHeight/2.0f)) * scene->viewPlaneHalfHeight)
				+ (scene->sppWidth/2.0f + j*scene->sppWidth)));
			/* vec4_add(&shift, &shift, &tmp); */
			shift.v = spu_add(shift.v, tmp.v);
		
			
			/* calculate target on view plane */
			/* vec4_add(&target, &scene->eyePos, &shift); */
			target.v = spu_add(scene->eyePos.v, shift.v);
			/* cast a ray from eye point to target on view plane */
			ray_create(&ray, &scene->eyePos, &target);
			/* get color of point this ray hits */
			get_ray_color(&color, &ray, scene);
			/* add color to accumulated color */
			color_add(colorout, colorout, &color, 0);
		}
	}

	/* divide values by number of points being sampled */
	return color_scale(colorout, colorout, scale, 0);
}

/* writes a 32 bit color value to memory in appropriate format */
void write_color_32(unsigned int *pixel, const color_t *color)
{
	/* max alpha channel */
	*pixel = 0xFF000000;
	*pixel |= ((unsigned char)(COLORVALUE_MAX * color->r)) << 16;
	*pixel |= ((unsigned char)(COLORVALUE_MAX * color->g)) << 8;
	*pixel |= (unsigned char)(COLORVALUE_MAX * color->b);
}

/* handles the ray tracing task handed to this spe and returns */
void spu_raytrace(spe_program_info_t *info, const scene_t *scene)
{
	unsigned int i = 0;
	unsigned int nPixel;	/* true index of pixel in main memory frame buffer */
	color_t	color;
	unsigned int pixel;
	unsigned long long write_ea;
	
	printf("SPE(%d) processing %d pixels...\n", speid, info->numPixels);
	/* assign to global */
	MAX_DEPTH = info->depth;

	for(; i < info->numPixels; ++i)
	{
		nPixel = i * info->numSpes + info->speId;
		color_init(&color);
		get_pixel_color(&color, nPixel % scene->frameBufferWidth,
					nPixel / scene->frameBufferWidth,
					scene);
		
		/* write color */
		write_color_32(&pixel, &color);		
		/* write to correct pixel address in main memory */
		write_ea = info->frame_buffer_ea + (nPixel * sizeof(unsigned int));
#if defined(_DEBUG) && _DEBUG > 1	
/*		printf("Spe(%d) writing pixel to (%08lX:%08lX) from (0x%08lX)...\n",
			speid, mfc_ea2h(write_ea), mfc_ea2l(write_ea), &pixel); */
#endif
		/* DMA pixel value to main memory */
/*
		spu_mfcdma64(&pixel,
			mfc_ea2h(write_ea),
			mfc_ea2l(write_ea),
			sizeof(unsigned int),
			SPUDMA_PIXELSOUT,
			MFC_PUT_CMD);
*/
	
		/* write to mail box - stalls until previous pixel is read by PPE*/	
		/* spu_write_out_mbox(pixel); */
#ifndef _PPE_NOWAIT
		spu_writech(SPU_WrOutMbox, pixel);
#endif
	}

}



