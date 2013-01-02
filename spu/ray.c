/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 31, 2008
 *  ray.c
 *
 *  This file contains the ray_t data type as well as the prototypes
 *  for the associated math functions to do intersection tests against
 *  geometry.
 */

#if defined(__SPU__)
	#include <simdmath.h>
	#include <simdmath/acosf4.h>
	#include <simdmath/sqrtf4.h>
	#include <length_vec3.h>
	#include <reflect_vec3.h>
	#define M_PI 3.1415926535897932384
#else
	#define _USE_MATH_DEFINES
	#include <math.h>
#endif

#ifdef _DEBUG
	#include <stdio.h>
#endif
#include "ray.h"


/* create a ray from two points [p1 -> p2] (vector4_t under the hood) */
ray_t* ray_create(ray_t *rayout, const point_t *p1, const point_t *p2)
{
	/* origin of ray is point 1 */
	vec4_set(&rayout->origin, (float *)p1);
	/* get vector from point 1 to point 2 */
	/* vec4_sub(&rayout->direction, p2, p1); */
	rayout->direction.v = spu_sub(p2->v, p1->v);
	/* get magnitude of vector we just created */
	/*rayout->magnitude = vec4_magnitude(&rayout->direction);*/
	rayout->magnitude = _length_vec3(rayout->direction.v);
	/* normalize the vector this way to prevent duplicate work */
	/*vec4_scale(&rayout->direction, &rayout->direction, 1.0f / rayout->magnitude);*/
	rayout->direction.v = spu_mul(rayout->direction.v, spu_splats(1.0f/rayout->magnitude));

	return rayout;
}

/* copies a ray into another ray */
ray_t* ray_copy(ray_t *rayout, const ray_t *ray)
{
	vec4_set(&rayout->direction, (float *)&ray->direction);
	vec4_set(&rayout->origin, (float *)&ray->origin);
	rayout->magnitude = ray->magnitude;

	return rayout;
}

/* reverse the direction and origin of a ray */
ray_t* ray_reverse(ray_t *rayout, const ray_t *ray)
{
	ray_t tmpray;
	ray_copy(&tmpray, ray);
	/* scale original direction by magnitude of ray */
	/*vec4_scale(&tmpray.direction, &ray->direction, ray->magnitude);*/
	tmpray.direction.v = spu_mul(ray->direction.v, spu_splats(ray->magnitude));
	/* origin of new ray is destination of original ray */
	/*vec4_add(&rayout->origin, &tmpray.direction, &ray->origin);*/
	rayout->origin.v = spu_add(tmpray.direction.v, ray->origin.v);
	/* copy magnitude straight over */
	rayout->magnitude = ray->magnitude;
	/* direction of new ray is reverse that of old ray */
	/* vec4_scale(&rayout->direction, &tmpray.direction, -1.0f); */
	rayout->direction.v = spu_mul(tmpray.direction.v, spu_splats(-1.0f));

	return rayout;
}

/* this function is needed for a serious hack that I really
 * hate doing.  R.I.P. clean code R.I.P. */
ray_t* ray_tinypush(ray_t *rayout, const ray_t *ray)
{
	vector4_t v;
	vec4_set(&v, (float *)&ray->direction);
	/* make sure direction was normalized */
	vec4_normalize(&v);
	/* create a tiny displacement vector along direction */
	vec4_scale(&v, &v, 0.001f);
	/* copy the ray */
	ray_copy(rayout, ray);
	/* display the new rays origin */
	vec4_add(&rayout->origin, &rayout->origin, &v);

	return rayout;
}

/* tests if ray intersects a given polygon */
int ray_intersect_polygon(const ray_t *ray, const polygon_t* poly,
							point_t *pt, float *distance)
{
	float num = -1.0f * (
		vec4_dot((vector4_t *)&poly->plane, &ray->origin) +
		poly->plane.F);
	float den = vec4_dot((vector4_t *)&poly->plane, &ray->direction);
	float w;
	unsigned int i = 0;		/* counter variable for loop */
	double angleTotal = 0.0;	/* running total of angle */
	double angleTmp = 0.0;		/* angle between current two vectors */	
	vector4_t tmp;
	vector4_t tmp2;
#ifdef __SPU__
	vector float vTmp;
#endif

	/* if denomenator = 0, ray is parallel to plane */
	if(den == 0.0f)
	{	/* return no intersection */
		return 0;
	}

	w = num / den;		/* distance to intersection */
	/* if w < 0, intersection point is behind ray */
	if(w < 0.0f)
	{	/* return no intersection */
		return 0;
	}

	/* now w is least positive root */
	/* use it to calculate where intersection point is */
	/*vec4_add(pt, &ray->origin,
		vec4_scale(&tmp, &ray->direction, w));*/
	pt->v = spu_add(ray->origin.v, spu_mul(ray->direction.v, spu_splats(w)));
	*distance = w;		/* pass back distance to intersection */

	/* at this point we at least know the ray intersects the plane.
	 * let's figure out if the point is actually inside the confined
	 * polygonal area */
	for(i = 0; i < poly->nVerticies; ++i)
	{
		if(i == (poly->nVerticies - 1))
		{	/* last vertex, compare with first */
			/* calculate two vectors */
			/*vec4_sub(&tmp, &poly->vertex[i], pt);*/
			tmp.v = spu_sub(poly->vertex[i].v, pt->v);
			/*vec4_sub(&tmp2, &poly->vertex[0], pt);*/
			tmp2.v = spu_sub(poly->vertex[0].v, pt->v);
			/* find angle between them - between normal vectors, dot
			 * product is cos of angle between them */
#ifdef __SPU__
			vTmp[0] = vec4_costheta(&tmp, &tmp2);
			vTmp = _acosf4(vTmp);
			angleTotal += vTmp[0];
#else
			angleTmp = vec4_costheta(&tmp, &tmp2);
			/* arccos to get theta */
			angleTmp = acos(angleTmp);
			angleTotal += angleTmp;
#endif
		}
		else
		{
			/* calculate two vectors */
			/*vec4_sub(&tmp, &poly->vertex[i], pt);*/
			tmp.v = spu_sub(poly->vertex[i].v, pt->v);
			/*vec4_sub(&tmp2, &poly->vertex[i+1], pt);*/
			tmp2.v = spu_sub(poly->vertex[i+1].v, pt->v);
			/* find angle between them - between normal vectors, dot
			 * product is cos of angle between them */
#ifdef __SPU__
			vTmp[0] = vec4_costheta(&tmp, &tmp2);
			vTmp = _acosf4(vTmp);
			angleTotal += vTmp[0];
#else
			angleTmp = vec4_costheta(&tmp, &tmp2);
			/* arccos to get theta */
			angleTmp = acos(angleTmp);
			angleTotal += angleTmp;
#endif
		}
	}

	/* TODO: this comparison is weak, should be refined */
	if((angleTotal + .001) > (2.0 * M_PI))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


/* tests if ray intersects a given sphere */
int ray_intersect_sphere(const ray_t *ray, const sphere_t* sphere,
							point_t *pt, float *distance)
{
	vector4_t	tmp;	/* used to hold scale of ray direction */
	float A = 1;	/* since we know ray direction is normalized */
	float dx = ray->origin.x - sphere->center.x;
	float dy = ray->origin.y - sphere->center.y;
	float dz = ray->origin.z - sphere->center.z;
	float B = 2 * (
		ray->direction.x * (dx) +
		ray->direction.y * (dy) +
		ray->direction.z * (dz));
	float C = (dx * dx + dy * dy + dz * dz) 
		- (sphere->radius * sphere->radius);
	float det = (B*B) - (4 * A * C);
	float wOne;	/* distance to first intersection */
	float wTwo;	/* distance to second intersection */
	float w = 0.0f;	/* least positive w */
#ifdef __SPU__
	vector float vTmp;
	float	scalarTmp;
#endif

	if(det < 0.0f)
	{	/* intersection is behind ray so none at all*/
		return 0;
	}
	else
	{	/* no need to use A since it's 1 */
#ifdef __SPU__
		vTmp = spu_promote(det, 0);
		scalarTmp = spu_extract(
				_sqrtf4(vTmp), 0);
		wOne = (-B - scalarTmp) / (2.0f * A);
		wTwo = (-B + scalarTmp) / (2.0f * A);
#else
		wOne = (-B - sqrt(det)) / (2.0f * A);
		wTwo = (-B + sqrt(det)) / (2.0f * A);
#endif

		if(det == 0.0f)
		{	/* one root, wOne and wTwo should be equal */
			/*vec4_add(pt, &ray->origin, 
				vec4_scale(&tmp, &ray->direction, wOne));*/
			pt->v = spu_add(ray->origin.v, spu_mul(ray->direction.v,
							spu_splats(wOne)));
			*distance = wOne;	/* pass back distance to intersection */
			return 1;
		}
		else
		{
			if(wOne > 0.0f)
				w = wOne;
			if(wTwo > 0.0f && wTwo < w)
				w = wTwo;
			/* now w is least positive root */
			/* use it to calculate where intersection point is */
			/*vec4_add(pt, &ray->origin, 
				vec4_scale(&tmp, &ray->direction, w)); */
			pt->v = spu_add(ray->origin.v, spu_mul(ray->direction.v,
							spu_splats(w)));
			*distance = w;		/* pass back distance to intersection */
			return 1;
		}
	}

	/* all code paths above this point should have returned something
	 * but if we've gotten here anyways, assume no intersection occured */
#if defined(_DEBUG)
	printf("Ray-Sphere intersection test failed!!!\n");
#endif
	return 0;
}

