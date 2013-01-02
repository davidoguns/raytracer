/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 22, 2008
 *  vector4.h
 *
 *  This file contains the vector 4 data type as well as the prototypes
 *  for the associated math functions to do work with them
 */

#ifndef _VECTOR4_H_
#define _VECTOR4_H_

typedef struct
{
	union 
	{
		float c[4];		/* array access */
		struct
		{	/* dimension name access */
			float x, y, z, w;
		};
#if defined(__PPU__) || defined(__SPU__)
		vector float v;
	} __attribute__ ((aligned(16) ));	/* for cell processor */
#else
	};
#endif

#ifdef __PPU__
/* macro defines a bit pattern to shift elements in a vector
 * by a specified number of bytes to the left.
 * to be used mostly by vec_slo() AltiVec function */
#define VECPATTERN_SHIFTLEFT(n) (vector unsigned char) { \
					0, 0, 0, 0,	\
				 	0, 0, 0, 0,	\
					0, 0, 0, 0,	\
					0, 0, 0, (n << 3) }
#endif

} vector4_t, point_t;

/* set elements of vector to 0 */
void vec4_clear(vector4_t *vec);

/* initialize elements of vector based on array values 
 * returns vector that was set */
vector4_t* vec4_set(vector4_t *vec, const float *values);

/* vector add */
vector4_t* vec4_add(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2);

/* vector sub (leaves w coordinate in tact) */
vector4_t* vec4_sub(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2);

/* vector scalar multiplication */
vector4_t* vec4_scale(vector4_t *outvec, const vector4_t *vec, float scale);

/* dot product */
float vec4_dot(const vector4_t *vec1, const vector4_t *vec2);
/* dot product that is applied to the fourth component of vector as well */
float vec4_dot4(const vector4_t *vec1, const vector4_t *vec2);

/* cross product */
vector4_t* vec4_cross(vector4_t *outvec,
		const vector4_t *vec1, const vector4_t *vec2);

/* magnitude */
float vec4_magnitude(const vector4_t *vec);

/* magnitude squared of vector */
float vec4_magsquared(const vector4_t *vec);

/* distance between two points */
float point_distance(const point_t *p1, const point_t *p2);

/* distance squared between two points */
float point_distsquared(const point_t *p1, const point_t *p2);

/* normalize */
vector4_t* vec4_normalize(vector4_t *vec);

/* cosine of angle between vectors */
float vec4_costheta(const vector4_t *vec1, const vector4_t *vec2);

#ifdef _DEBUG
	/* debug output vector */
	void vec4_output(const vector4_t *vec);
#else
	#define vec4_output(x)
#endif

#endif
