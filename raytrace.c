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
#include <stdlib.h>
#include "raytrace.h"
#include "ray.h"

/* max value of a single color channel */
#define COLORVALUE_MAX		0xFF

/* max depth set by caller of ray trace */
int MAX_DEPTH	=	4;

/* returns pointer to this buffer */
ray_t** create_raybuffer(unsigned int width, unsigned int height,
						 unsigned int samplesPerPixelSq)
{
	/* initial ray buffer */
	ray_t	**rays = (ray_t **)malloc(sizeof(ray_t *)*width*height);

	/* counting variables that are reused */
	unsigned int i = 0;
	unsigned int j = 0;
	
	/* create space for the set number of rays in each ray group */
	for(; j < height; ++j)
	{	/* for each row */
		for(i = 0; i < width; ++i)
		{	/* for each ray group in row */
			/* create a list of rays by spp^2 */
			rays[i + j*width] = (ray_t *)malloc(sizeof(ray_t)
				*samplesPerPixelSq*samplesPerPixelSq);
		}
	}

	return rays;
}

/* frees the memory associated with initial rays in the ray buffer */
void free_raybuffer(ray_t** buffer, unsigned int width, unsigned int height)
{
	unsigned int i;
	unsigned int j = 0;
	/* free space for the set number of rays in each ray group */
	for(; j < height; ++j)
	{	/* for each row */
		for(i = 0; i < width; ++i)
		{	/* for each ray group in row */
			/* free list of rays */
			free(buffer[i+j*width]);
		}
	}
	/* free entire buffer */
	free(buffer);
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

void init_raybuffer(ray_t **raybuffer, float fovY, float aspectRatio, 
		float nearZ, float farZ, unsigned int width, unsigned int height,
		unsigned int samplesPerPixelSq, const scene_t *scene)
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

	vector4_t tmp;			/* used for intermediate values for translation */
	ray_t	  *raygroup;	/* place holder for ray group we are accessing */
	/* counting variables that are reused */
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int m = 0;
	
	float uBaseDistance;	/* x offset from center of view plane for each pixel*/
	float vBaseDistance;	/* y offset from center of view plane for each pixel*/
	float uDistance;		/* final u position of ray origin on view place */
	float vDistance;		/* final v position of ray origin on view plane */ 
	float uSampleOffset;	/* offset on (x)u due to super sampling */
	float vSampleOffset;	/* offset on (y)v due to super sampling */
	
	/* create space for the set number of rays in each ray group */
	for(; j < height; ++j)
	{	/* for each row */
		/* calculate the v shift */
		vBaseDistance = (((height/2.0f) - j)/(height/2.0f)) * (float)viewPlaneHalfHeight;
		for(i = 0; i < width; ++i)
		{	/* for each ray group in row */			
			raygroup = raybuffer[i + j*width];
			/* calculate the u shift */
			uBaseDistance = ((i-(width/2.0f))/(width/2.0f)) * (float)viewPlaneHalfWidth;
			
			for(m = 0; m < spp; ++m)
			{	/* for each row in ray group */
				vSampleOffset = samplePixelWidth/2.0f + m*samplePixelWidth;
				for(k = 0; k < spp; ++k)
				{	/* for each ray in row */
					uSampleOffset = samplePixelWidth/2.0f + k*samplePixelWidth;

					uDistance = uBaseDistance + uSampleOffset;
					vDistance = vBaseDistance - vSampleOffset;

					/* move into the scene from eye point along the N vector */
					vec4_scale(&tmp, (vector4_t *)&scene->N, viewD);
					vec4_add(&raygroup[k + m*spp].origin, &scene->eyePos, &tmp);
					/* now move along the U vector */
					vec4_scale(&tmp, (vector4_t *)&scene->U, uDistance);
					vec4_add(&raygroup[k + m*spp].origin, &raygroup[k + m*spp].origin, &tmp);
					/* now move along V vector */
					vec4_scale(&tmp, (vector4_t *)&scene->V, vDistance);
					vec4_add(&raygroup[k + m*spp].origin, &raygroup[k + m*spp].origin, &tmp);
					
					/* now origin of ray is properly placed in the 3D world */
					/* we can calcuate the direction of the ray by subtracting
						the eye position from the origin point on the view plane */
					vec4_sub(&raygroup[k + m*spp].direction, &raygroup[k + m*spp].origin,
						&scene->eyePos);

					raygroup[k + m*spp].magnitude = vec4_magnitude(&raygroup[k + m*spp].direction);
					/* use the fact that we already have the magnitude for speed up 
						of normalization */
					vec4_scale(&raygroup[k + m*spp].direction, &raygroup[k + m*spp].direction,
						1.0f / raygroup[k + m*spp].magnitude);
				}
			}
		}
	}
}

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
	vec4_sub(&V, &eye->origin, pt);
	/* normalize V */
	vec4_normalize(&V);
/*
	printf("V.direction:\t(%f, %f, %f) - %f\n", V.x, V.y, V.z, vec4_magnitude(&V));
	printf("E.direction:\t(%f, %f, %f) - %f\n", eye->direction.x, eye->direction.y, eye->direction.z, vec4_magnitude(&eye->direction));
*/

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
		tmpFloat = vec4_dot(&S, &N);
		vec4_scale(&R, &N, 2.0f*tmpFloat);
		vec4_sub(&R, &R, &S);
		/* is this needed? */

		/* only apply specular and diffuse components from light if 
		 * surface normal points towards light
		 * i.e. not a back face of surface */
		if( tmpFloat <= 0.0f)
			continue;

		tmpFloat = vec4_dot(&S, &N);
		diff.r += scene->lights[i].color.r * tmpFloat;
		diff.g += scene->lights[i].color.g * tmpFloat;
		diff.b += scene->lights[i].color.b * tmpFloat;

		tmpFloat = vec4_dot(&R, &V);
		if(tmpFloat < 0.0f)
			tmpFloat = 0.0f;
		tmpFloat = powf(tmpFloat, obj->material.phong_ke);
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
		tmpFloat = vec4_dot(&V, &N);
		vec4_scale(&reflRay.direction, &N, 2.0f*tmpFloat);
		vec4_sub(&reflRay.direction, &reflRay.direction, &V);
		
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
		vec4_scale(&transRay.direction, &eye->direction, nit);
		/* use transRay.origin as a tmp vector - (-D) */
		tmpFloat = vec4_dot(&V, &N);
		if(tmpFloat < 0.0f)
		{	/* we are inside the object moving out */
			nit = 1.0f / nit;
			/* WARNING - N is being changed here */
			vec4_scale(&N, &N, -1.0f);
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
			tmpFloat = vec4_dot(&V, &N);
			vec4_scale(&reflRay.direction, &N, 2.0f*tmpFloat);
			vec4_sub(&reflRay.direction, &reflRay.direction, &V);
			
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
					sqrt(transRay.magnitude);
			/* used again as tmp vector */
			vec4_scale(&transRay.origin, &N, tmpFloat);
			vec4_add(&transRay.direction, &transRay.direction, &transRay.origin);

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

/* calculates the color at a particular shading point on a specified object
 * we pass in the scene primary to use lights, but also for casting other
 * rays.
 * obj - object being intersected
 * eye - observer of this shading point
 * pt - point of intersection on the object
 * scene - entire scene
 */
color_t* get_shade_color_phongblinn(color_t *colorout, const object3d_t *obj,
						 const ray_t *eye, const point_t *pt,
						 const scene_t *scene)
{
	unsigned int i = 0;
	ray_t		shadow;				/* shadow ray */
	point_t		shadow_intersect;	/* point on object shadow ray first intersects */
	object3d_t	*shadow_int_object;	/* object shadow ray intersects */
	float		shadow_int_dist;	/* distance to shadow object intersection */
	vector4_t	N, S, V, H, tmp;		/* vectors for lighting calculations */
	float		tmpFloat;			/* used in various calculations */
	color_t		diff;				/* diffuse term sum */
	color_t		spec;				/* specular term sum */
	
	/* get ambient light contribution first */
	color_mult(colorout, &obj->material.colors[MATERIAL_DIFFUSECOLOR],
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
	vec4_sub(&V, &eye->origin, pt);
	/* normalize V */
	vec4_normalize(&V);

	/* for each light source - cast shadow ray towards light */
	for( ; i < scene->nLights; ++i)
	{
		/* generate a ray from intersection point to light */
		/* note - magnitude of ray created is the distance to the light */
		ray_create(&shadow, pt, &scene->lights[i].position);
		/* get first object that ray intersects NOT including this object */
		shadow_int_object = get_object3d_intersect_excl(&shadow_intersect,
									&shadow_int_dist, &shadow, scene, obj);
		if(shadow_int_object)
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
		/* halfway vector takes a bit more to calculate */
		vec4_add(&tmp, &V, &S);
		tmpFloat = 1.0 / vec4_magnitude(&tmp);
		vec4_scale(&H, &tmp, tmpFloat);

		tmpFloat = vec4_dot(&S, &N);
		/* only apply specular and diffuse components from light if 
		 * surface normal points towards light
		 * i.e. not a back face of surface */
		if( tmpFloat <= 0.0f)
			continue;

		diff.r += scene->lights[i].color.r * tmpFloat;
		diff.g += scene->lights[i].color.g * tmpFloat;
		diff.b += scene->lights[i].color.b * tmpFloat;

		tmpFloat = vec4_dot(&H, &N);
		if(tmpFloat < 0.0f)
			tmpFloat = 0.0f;
		tmpFloat = powf(tmpFloat, obj->material.phong_ke);
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
		diff.r * obj->material.colors[MATERIAL_DIFFUSECOLOR].r +
		obj->material.phong_ks *
		spec.r * obj->material.colors[MATERIAL_SPECULARCOLOR].r;
	colorout->g += 	obj->material.phong_kd * 
		diff.g * obj->material.colors[MATERIAL_DIFFUSECOLOR].g +
		obj->material.phong_ks *
		spec.g * obj->material.colors[MATERIAL_SPECULARCOLOR].g;
	colorout->b += 	obj->material.phong_kd * 
		diff.b * obj->material.colors[MATERIAL_DIFFUSECOLOR].b +
		obj->material.phong_ks *
		spec.b * obj->material.colors[MATERIAL_SPECULARCOLOR].b;

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
/* this version of the function relies on the raybuffer existing in memory -
 * it appears to work much faster but consumes far more memory */
color_t* old_get_pixel_color(color_t *colorout, const ray_t *rays,
						unsigned int nRays, const scene_t *scene)
{
	unsigned int i = 0;
	color_t color;

	/* trace all rays in ray group and average color values */
	for(; i < nRays; ++i)
	{
		/* get color of point this ray hits */
		get_ray_color(&color, &rays[i], scene);
		/* add color to accumulated color */
		color_add(colorout, colorout, &color, 0);
	}

	/* divide values by number of points being sampled */
	return color_scale(colorout, colorout, 1.0f/(float)nRays, 0);
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
			vec4_scale(&tmp, &scene->N, scene->viewDistance);
			vec4_set(&shift, (float *)&tmp);

			/* U - move left/right based on X pos and spp */
			vec4_scale(&tmp, &scene->U, 
				(((x-(scene->frameBufferWidth/2.0f))/(scene->frameBufferWidth/2.0f)) * scene->viewPlaneHalfWidth) 
				+ (scene->sppWidth/2.0f + i*scene->sppWidth));
			vec4_add(&shift, &shift, &tmp);

			/* N - move up/down based on Y pos and spp */
			vec4_scale(&tmp, &scene->V, 
				((((scene->frameBufferHeight/2.0f) - y)/(scene->frameBufferHeight/2.0f)) * scene->viewPlaneHalfHeight)
				+ (scene->sppWidth/2.0f + j*scene->sppWidth));	/* v shift based on X */
			vec4_add(&shift, &shift, &tmp);
		
			
			/* calculate target on view plane */
			vec4_add(&target, &scene->eyePos, &shift);
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

void ward_tone(color_t *colorbuffer, float *lbuffer, unsigned int nPixels,
		float ldMax, float totalLum, float totalLogLum)
{
	float logAvgLum = expf( (1.0f/(float)nPixels) * totalLogLum );
	float avgLum = totalLum / (float)nPixels;
	float sf = powf((1.219f + powf(ldMax/2.0f, 0.4f)) / 
			(1.219f + powf(logAvgLum, 0.4f)) , 2.5f);
	unsigned int i = 0;

	for(; i < nPixels; ++i)
	{
		/* apply scale factor to all channels */
		color_scale(&colorbuffer[i], &colorbuffer[i], sf, 0);
	}
}

void reinhard_tone(color_t *colorbuffer, float *lbuffer, unsigned int nPixels,
		float ldMax, float totalLum, float totalLogLum, int keyPix)
{
	float logAvgLum = expf( (1.0f/(float)nPixels) * totalLogLum );
	float avgLum = totalLum / (float)nPixels;
	unsigned int i = 0;
	float scale = 0.18f / logAvgLum;

	if(keyPix > -1)
	{
		scale = 0.18 / get_luminance(&colorbuffer[keyPix]);
	}

	for(; i < nPixels; ++i)
	{
		color_scale(&colorbuffer[i], &colorbuffer[i], scale, 0);
		colorbuffer[i].r = (colorbuffer[i].r * ldMax) / 
					(1.0f + colorbuffer[i].r);
		colorbuffer[i].g = (colorbuffer[i].g * ldMax) / 
					(1.0f + colorbuffer[i].g);
		colorbuffer[i].b = (colorbuffer[i].b * ldMax) / 
					(1.0f + colorbuffer[i].b);
	}
}

/* do the tone reproduction step */
void apply_tone(color_t *colorbuffer, unsigned int nPixels, int keyPix, const scene_t *scene)
{
	unsigned int i = 0;
	/* inverse of ldMax */
	float ldMaxInv = 1.0f / scene->ldMax;
	/* luminance buffer */
	float *lbuffer = malloc(sizeof(float) * nPixels);
	/* total luminance of scene to obtain avg */
	float totalLum = 0.0f;
	/* total log-average luminance */
	float totalLogLum = 0.0f;
	/* delta - used for log-average luminance */
	float delta = .00001f;

	printf("ldmax = %f; lMax = %f\n", scene->ldMax, scene->lMax);

	/* prepare HDR image -  multiply every pixel by lmax  */
	for(; i < nPixels; ++i)
	{
		color_scale(&colorbuffer[i], &colorbuffer[i], scene->lMax, 0);
		/* get target luminance of pixel from 0 to lMax */
		lbuffer[i] = get_luminance(&colorbuffer[i]);
		/* accumulate total luminance of all pixels for avg lum*/
		totalLum += lbuffer[i];
		totalLogLum += logf(delta + lbuffer[i]);
	}

	ward_tone(colorbuffer, lbuffer, nPixels, scene->ldMax, 
			totalLum, totalLogLum);

	/* back to display model */
	for(i = 0; i < nPixels; ++i)
	{
		color_scale(&colorbuffer[i], &colorbuffer[i], ldMaxInv, 1);
	}

	free(lbuffer);
}

/* called to start the ray tracing process */
void raytrace(unsigned int *buffer, scene_t *scene,
		float fovY, float aspectRatio, float nearZ, float farZ,
		unsigned int width, unsigned int height,
		unsigned int samplesPerPixelSq, unsigned int depth)
{
	unsigned int i;
	unsigned int j = 0;
	/* ray buffer is (9*spp^2) times bigger than frame buffer 
	 * for example - 1080p ray buffer at 16x super sampling is 
	 * 1200MB in size */
	/* ray_t **raybuffer = create_raybuffer(width, height, samplesPerPixelSq); */

	/* create color buffer */
	color_t *colorbuffer = malloc(sizeof(color_t) * width * height); 
	/* immediate color observed */
	color_t color;	
	/* assign to global variable */
	MAX_DEPTH = depth;

	/* generate some extra information in the scene to generate rays on the fly */
	prepare_scene(scene, width, height, samplesPerPixelSq, fovY, aspectRatio, nearZ);

	/* generate initial rays using view plane */
	/* init_raybuffer(raybuffer, fovY, aspectRatio, nearZ, farZ, width, height,
		samplesPerPixelSq, scene); */

	/* now start processing the pixels */

	/* iterate every initial pixel and start ray tracing!!! */
	for(; j < height; ++j)
	{
		for(i = 0; i < width; ++i)
		{	/* for every pixel pass in the ray group and get color value*/
			
			color_init(&colorbuffer[i+j*width]);
		 	get_pixel_color(&colorbuffer[i+j*width], i, j, scene);
		/*	old_get_pixel_color(&colorbuffer[i+j*width], raybuffer[i+j*width], 
				samplesPerPixelSq*samplesPerPixelSq, scene); */
		}
	}

	/* now that we have the raw colors, run the tone reproduction operation(s) */	
	/* with reinhard key value location */
	/*apply_tone(colorbuffer, width * height, 10 + 10 * width, scene);*/

	/* now copy colors to frame buffer */
	for(j = 0; j < height; ++j)
	{
		for(i = 0; i < width; ++i)
		{
			/* convert colors to 32 bit */
			write_color_32(&buffer[i+j*width], &colorbuffer[i+j*width]);
		}
	}

	/* free_raybuffer(raybuffer, width, height); */
	free(colorbuffer);
}

