/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 22, 2008
 *  vector4.c
 *
 *  This file contains the vector 4 math function definitions.
 */

#ifdef _DEBUG
#include <stdio.h>
#endif

#ifdef __SPU__
	#include <simdmath.h>
	#include <simdmath/sqrtf4.h>
#else
	#include <math.h>
#endif
#include "vector4.h"

#ifdef __SPU__
	#include <spu_intrinsics.h>
#elif defined(__PPU__)
	#include <altivec.h>
	#include <vec_types.h>
#endif



/* set dimensions of vector to 0 (w = 1) */
void vec4_clear(vector4_t *vec)
{
#if defined(__SPU__) || defined (__PPU__)
	vec->v = (vector float) {0.0f, 0.0f, 0.0f, 0.0f};
#else
	vec->c[0] = 0.0f;
	vec->c[1] = 0.0f;
	vec->c[2] = 0.0f;
	vec->c[3] = 0.0f;
#endif
}

/* initialize elements of vector based on array values 
 * returns vector that was set */
vector4_t* vec4_set(vector4_t *vec, const float *values)
{
#if defined (__PPU__)
	vec->v = *((vector float *)values);
#else
	vec->c[0] = values[0];
	vec->c[1] = values[1];
	vec->c[2] = values[2];
	vec->c[3] = values[3];
#endif
	return vec;
}

/* vector add */
vector4_t* vec4_add(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2)
{
#if defined (__PPU__)
	outvec->v = vec_add(vec1->v, vec2->v);
#elif defined(__SPU__)
	outvec->v = spu_add(vec1->v, vec2->v);
#else
	outvec->x = vec1->x + vec2->x;
	outvec->y = vec1->y + vec2->y;
	outvec->z = vec1->z + vec2->z;
	outvec->w = 0.0f;
#endif

	return outvec;
}

/* vector sub (leaves w coordinate in tact) */
vector4_t* vec4_sub(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2)
{
#if defined (__PPU__)
	outvec->v = vec_sub(vec1->v, vec2->v);
#elif defined(__SPU__)
	outvec->v = spu_sub(vec1->v, vec2->v);
#else
	outvec->x = vec1->x - vec2->x;
	outvec->y = vec1->y - vec2->y;
	outvec->z = vec1->z - vec2->z;
	outvec->w = 0.0f;
#endif

	return outvec;
}

/* vector scalar multiplication */
vector4_t* vec4_scale(vector4_t *outvec, const vector4_t *vec, float scale)
{
#if defined (__PPU__)
	vector float v;
	v[0] = v[1] = v[2] = v[3] = scale;
	vector float vz;
	vz[0] = vz[1] = vz[2] = vz[3] = 0.0f;
	outvec->v = vec_madd(vec->v, v, vz);
#elif defined(__SPU__)
	vector float v;
	v[0] = v[1] = v[2] = v[3] = scale;
	vector float vz;
	vz[0] = vz[1] = vz[2] = vz[3] = 0.0f;
	outvec->v = spu_madd(vec->v, v, vz);
#else
	outvec->x = vec->x * scale;
	outvec->y = vec->y * scale;
	outvec->z = vec->z * scale;
	outvec->w = 0.0f;
#endif

	return outvec;
}

/* dot product */
float vec4_dot(const vector4_t *vec1, const vector4_t *vec2)
{
/*
#if defined(__SPU__) || defined (__PPU__)
	vector float tmp;
	vector float tmp2;
	vector float tmp3 = {0.0f, 0.0f, 0.0f, 0.0f};
#endif

#if defined(__SPU__)
	tmp = spu_add(vec1->v, vec2->v);
#elif defined(__PPU__)
	tmp = vec_madd(vec1->v, vec2->v, tmp3);
	tmp[3] = 0.0f;
	tmp2 = vec_slo(tmp, VECPATTERN_SHIFTLEFT(8));
	tmp = vec_add(tmp, tmp2);
	tmp2 = vec_slo(tmp, VECPATTERN_SHIFTLEFT(4));
	return vec_add(tmp, tmp2)[0];
#else
*/
	return vec1->x * vec2->x +
		vec1->y * vec2->y +
		vec1->z * vec2->z;
}

float vec4_dot4(const vector4_t *vec1, const vector4_t *vec2)
{
#if defined(__SPU__) || defined (__PPU__)
	vector float tmp;
	vector float tmp2;
	vector float tmp3 = {0.0f, 0.0f, 0.0f, 0.0f};
#endif

#if defined(__SPU__)
	tmp = spu_add(vec1->v, vec2->v);
#elif defined(__PPU__)
	tmp = vec_madd(vec1->v, vec2->v, tmp3);
	tmp2 = vec_slo(tmp, VECPATTERN_SHIFTLEFT(8));
	tmp = vec_add(tmp, tmp2);
	tmp2 = vec_slo(tmp, VECPATTERN_SHIFTLEFT(4));
	return vec_add(tmp, tmp2)[0];
#else
	return vec1->x * vec2->x +
		vec1->y * vec2->y +
		vec1->z * vec2->z +
		vec1->w * vec2->w;
#endif
}


/* cross product 
 * This version works correctly if the output vector is the same
 * as one of the input vectors. i.e. A = A X B or B = A X B
 * It does this by instantiating temporary variables to hold
 * both input vectors
 */
vector4_t* _vec4_cross(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2)
{
/*
#if defined(__SPU__) || defined (__PPU__)
#else
*/
	/* if the caller decided to use vector1 also as the ouput
 	 * vector, then we want to make sure this function still
 	 * works.  For that reason, we need a temporary variable */
	vector4_t tmp;
	vector4_t tmp2;
	vec4_set(&tmp, (float *)vec1);	/* copy vector 1 into tmp */
	vec4_set(&tmp2, (float *)vec2);	/* copy vector 2 into tmp2 */
	
	/* now start writing to outvec as normal */
	outvec->x = tmp.y * tmp2.z - tmp.z * tmp2.y;
	outvec->y = tmp.z * tmp2.x - tmp.x * tmp2.z;
	outvec->z = tmp.x * tmp2.y - tmp.y * tmp2.x;
	outvec->w = 0.0f;
/* #endif */

	return outvec;	
}

/* cross product 
 * This version is not safe to use if the output vector is the
 * same as one of the input vectors.  The writes to the output
 * vector will corrupt the inputs in the middle of the calculation
 */
vector4_t* vec4_cross(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2)
{
	outvec->x = vec1->y * vec2->z - vec1->z * vec2->y;
	outvec->y = vec1->z * vec2->x - vec1->x * vec2->z;
	outvec->z = vec1->x * vec2->y - vec1->y * vec2->x;
	outvec->w = 0.0f;

	return outvec;	
}

/* magnitude */
float vec4_magnitude(const vector4_t *vec)
{
#ifdef __SPU__
	vector float v = spu_promote(
		vec->x * vec->x +
		vec->y * vec->y +
		vec->z * vec->z, 0);
	return _sqrtf4(v)[0];
#else
	return sqrt( 
		vec->x * vec->x +
		vec->y * vec->y +
		vec->z * vec->z );
#endif
}

/* magnitude squared of vector */
float vec4_magsquared(const vector4_t *vec)
{
	return 	vec->x * vec->x +
		vec->y * vec->y +
		vec->z * vec->z;
}

/* distance between two points */
float point_distance(const point_t *p1, const point_t *p2)
{
#ifdef __SPU__
	vector float v = spu_promote(
		(p1->x - p2->x) * (p1->x - p2->x) +
		(p1->y - p2->y) * (p1->y - p2->y) +
		(p1->z - p2->z) * (p1->z - p2->z) , 0);
	return _sqrtf4(v)[0];
#else
	return sqrt( (p1->x - p2->x) * (p1->x - p2->x) +
		(p1->y - p2->y) * (p1->y - p2->y) +
		(p1->z - p2->z) * (p1->z - p2->z) );
#endif
}

/* distance squared between two points */
float point_distsquared(const point_t *p1, const point_t *p2)
{
	return (p1->x - p2->x) * (p1->x - p2->x) +
		(p1->y - p2->y) * (p1->y - p2->y) +
		(p1->z - p2->z) * (p1->z - p2->z);
}

/* normalize */
vector4_t* vec4_normalize(vector4_t *vec)
{
	float mag = vec4_magnitude(vec);

	vec->x /= mag;
	vec->y /= mag;
	vec->z /= mag;
	vec->w = 0.0f;

	return vec;
}

/* cosine of angle between vectors */
float vec4_costheta(const vector4_t *vec1, const vector4_t *vec2)
{
	return vec4_dot(vec1, vec2) / 
		(vec4_magnitude(vec1) * vec4_magnitude(vec2));
}

#ifdef _DEBUG
/* debug output vector */
void vec4_output(const vector4_t *vec)
{
	printf("V<%f, %f, %f, %f>", vec->x, vec->y, vec->z, vec->w);
}
#endif


