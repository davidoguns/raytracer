/*  David Oguns
 *  Computer Graphics II
 *  Ray Tracing
 *  March 23, 2008
 *  matrix4.h
 *
 *  This file contains the matrix 4 data type as well as the prototypes
 *  for the associated math functions to do work with them
 */

#ifndef _MATRIX4_H_
#define _MATRIX4_H_

#include "vector4.h"

typedef struct
{
	union
	{
		float m[4][4];
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
	};
} matrix4_t;

/*  Functions related to handling matrices */

/* Loads the identity matrix */
matrix4_t* mat4_identity(matrix4_t *mat);

/* Loads the matrix with a set of values */
matrix4_t* mat4_set(matrix4_t *mat, float *values);

/* Matrix add (perhaps will never be used in this project) */
matrix4_t* mat4_add(matrix4_t *outmat,
	const matrix4_t *mat1, const matrix4_t *mat2);

/* Matrix subtract (also will probably never be used) */
matrix4_t* mat4_sub(matrix4_t *outmat,
	const matrix4_t *mat1, const matrix4_t *mat2);

/* Matrix multiply */
/* there are perhaps many ways to optimize this function
 * for scalar processors.  But seeing as this will be ported
 * to run on a super scalar processor, I'll not bother
 */
matrix4_t* mat4_mul(matrix4_t *outmat,
	const matrix4_t *mat1, const matrix4_t *mat2);

/* multiply a vector4_t by matrix - or transform */
vector4_t* mat4_transform(vector4_t *outvec,
	const vector4_t *vec, const matrix4_t *mat);

/* Calculates determinate */
float mat4_determinate(const matrix4_t* mat);

/* Calculates the inverse of a matrix */
matrix4_t* mat4_inverse(matrix4_t *outmat, const matrix4_t *mat);

/* Fills in a projection transformation matrix */
matrix4_t* mat4_projection(matrix4_t* outmat,
	float fovTheta,		/* how wide the viewing angle is */
	float aspectRatio,	/* aspect ratio of view plane (w/h) */
	float nearZ,		/* how far in front of camera is view plane */
	float farZ);		/* how far back is out of range */

/* Fills in a view transformation matrix from float values */
matrix4_t* mat4_viewf(matrix4_t *outmat,
		float eyex, float eyey, float eyez,
		float lookatx, float lookaty, float lookatz,
		float upx, float upy, float upz);

/* Fills in a view transformation matrix from 3 vectors */
matrix4_t* mat4_viewv(matrix4_t *outmat,
		const vector4_t *eyePos,
		const vector4_t *lookAt,
		const vector4_t *upVec);

/* Rotation matrix around X axis */
matrix4_t* mat4_rotationX(matrix4_t *outmat, float angle);

/* Rotation matrix around Y axis */
matrix4_t* mat4_rotationY(matrix4_t *outmat, float angle);

/* Rotation matrix around Z axis */
matrix4_t* mat4_rotationZ(matrix4_t *outmat, float angle);

/* Generate translation matrix */
matrix4_t* mat4_translation(matrix4_t *outmat, float x, float y, float z);

/* Generate scale matrix */
matrix4_t* mat4_scale(matrix4_t *outmat, float sx, float sy, float sz);

/* debug output of a matrix */
void mat4_output(const matrix4_t *mat);

#endif
