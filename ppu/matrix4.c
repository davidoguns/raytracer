/* David Oguns
 * Computer Graphics II
 * Ray Tracer
 * March 25, 2008
 * matrix4.c
 *
 * This file contains the function definitions for all of the matrix4
 * math operations and operations that occur between matrix4 and vector4.
 */

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include "matrix4.h"

/* Loads the identity matrix */
matrix4_t* mat4_identity(matrix4_t *mat)
{
	unsigned int i = 0;
	unsigned int j = 0;
	for( ; i < 4; ++i)
	{
		for(j = 0 ; j < 4; ++j)
		{
			mat->m[i][j] = 0.0f;
		}
	}
	mat->m[0][0] = 1.0f;
	mat->m[1][1] = 1.0f;
	mat->m[2][2] = 1.0f;
	mat->m[3][3] = 1.0f;

	return mat;
}

/* Loads the matrix with a set of values */
/* This implementation might speed up if we did a straight memcpy.
 * For now, we will keep it like this for clarity.
 */
matrix4_t* mat4_set(matrix4_t *mat, float *values)
{
	unsigned int i = 0;
	for( ; i < 16; ++i)
	{	/* access 2D array as a flat one */
		((float *)mat->m)[i] = values[i];
	}
	return mat;
}

/* Matrix add (perhaps will never be used in this project) */
matrix4_t* mat4_add(matrix4_t *outmat,
	const matrix4_t *mat1, const matrix4_t *mat2)
{
	unsigned int i = 0;
	for( ; i < 16; ++i)
	{	/* access 2D array as a flat one */
		((float *)outmat->m)[i] = 
			((float *)mat1->m)[i] + ((float *)mat2->m)[i];
	}
	return outmat;
}

/* Matrix subtract (also will probably never be used) */
matrix4_t* mat4_sub(matrix4_t *outmat,
	const matrix4_t *mat1, const matrix4_t *mat2)
{

	unsigned int i = 0;
	for( ; i < 16; ++i)
	{	/* access 2D array as a flat one */
		((float *)outmat->m)[i] = 
			((float *)mat1->m)[i] - ((float *)mat2->m)[i];
	}
	return outmat;
}

/* Matrix multiply */
/* there are perhaps many ways to optimize this function
 * for scalar processors.  But seeing as this will be ported
 * to run on a super scalar processor, I'll not bother
 */
matrix4_t* mat4_mul(matrix4_t *outmat,
	const matrix4_t *mat1, const matrix4_t *mat2)
{
	matrix4_t tmp;		/* hold temporary copy of matrix1 */
	vector4_t tVec;		/* temporary vector to hold columns of B */
	unsigned int i = 0;	/* counter variable */
	unsigned int j = 0;

	mat4_set(&tmp, (float *)mat1->m); /* copy matrix1 */

	/* go across both matrices' columns first */
	for( ; j < 4; ++j)
	{	//grab column from b and put it in a vector
		tVec.c[0] = mat2->m[0][i];
		tVec.c[1] = mat2->m[1][j];
		tVec.c[2] = mat2->m[2][j];
		tVec.c[3] = mat2->m[3][j];
		/* iterate over the rows filling in selected column value*/
		for(i = 0 ; i < 4; ++i)
		{	//use dot product of A's row on B's column
			outmat->m[i][j] = vec4_dot4(&tVec, 
				(vector4_t *)mat1->m[i]);
		}
	}

	return outmat;
}

/* multiply a vector4_t by matrix - or transform */
vector4_t* mat4_transform(vector4_t *outvec,
	const vector4_t *vec, const matrix4_t *mat)
{
	vector4_t tmp;	/* in case output vector is same as input parameter */
	vec4_set(&tmp, (float *)vec);

	outvec->x = vec4_dot4((vector4_t *)mat->m[0], &tmp);
	outvec->y = vec4_dot4((vector4_t *)mat->m[1], &tmp);
	outvec->z = vec4_dot4((vector4_t *)mat->m[2], &tmp);
	outvec->w = vec4_dot4((vector4_t *)mat->m[3], &tmp);

	return outvec;
}

/* Calculates determinate */
float mat4_determinate(const matrix4_t* mat);

/* Calculates the inverse of a matrix */
matrix4_t* mat4_inverse(matrix4_t *outmat, const matrix4_t *mat);

/* Fills in a projection transformation matrix */
matrix4_t* mat4_projection(matrix4_t* outmat,
	float fovTheta,		/* how wide the viewing angle is */
	float aspectRatio,	/* aspect ratio of view plane (w/h) */
	float nearZ,		/* how far in front of camera is view plane */
	float farZ)			/* how far back is out of range */
{
	
	return outmat;
}

/* Fills in a view transformation matrix from float values */
matrix4_t* mat4_viewf(matrix4_t *outmat,
		float eyex, float eyey, float eyez,
		float lookatx, float lookaty, float lookatz,
		float upx, float upy, float upz)
{
	vector4_t tmp;
	tmp.x = eyex;
	tmp.y = eyey;
	tmp.z = eyez;
	tmp.w = 1.0f;
	
	/*n -  third row */
	outmat->m[2][0] = eyex - lookatx;
	outmat->m[2][1] = eyey - lookaty;
	outmat->m[2][2] = eyez - lookatz;
	outmat->m[2][3] = 1.0f;
	/* normalize vector */
	vec4_normalize((vector4_t *)outmat->m[2]);
	/* get negative eye vector */
	vec4_scale(&tmp, &tmp, -1.0f);
	outmat->m[2][3] = vec4_dot(&tmp, (vector4_t *)outmat->m[2]);

	tmp.x = upx;
	tmp.y = upy;
	tmp.z = upz;
	tmp.w = 1.0f;

	/*v second row */
	vec4_cross((vector4_t *)outmat->m[1], &tmp, (vector4_t *)outmat->m[2]);
	outmat->m[1][3] = vec4_dot(&tmp, (vector4_t *)outmat->m[1]);

	/*u -first row */
	vec4_cross((vector4_t *)outmat->m[0], 
		(vector4_t *)outmat->m[2], (vector4_t *)outmat->m[1]);
	outmat->m[0][3] = vec4_dot(&tmp, (vector4_t *)outmat->m[0]);
	
	/* fourth row looks like identity matrix */
	outmat->m[3][0] = 0.0f;
	outmat->m[3][1] = 0.0f;
	outmat->m[3][2] = 0.0f;
	outmat->m[3][3] = 1.0f;

	return outmat;
}

/* Fills in a view transformation matrix from 3 vectors */
matrix4_t* mat4_viewv(matrix4_t *outmat,
		const vector4_t *eyePos,
		const vector4_t *lookAt,
		const vector4_t *upVec)
{	
	vector4_t tmp;
	vector4_t U, V, N;
	vec4_scale(&tmp, eyePos, -1.0f);

	/*n - third row */
	vec4_sub(&N, eyePos, lookAt);			/* messes up N(Z axis) vector */
	/* vec4_sub(&N, lookAt, eyePos); */		/* messes up U(right) vector */
	outmat->_31 = N.x;
	outmat->_32 = N.y;
	outmat->_33 = N.z;
	outmat->_34 = vec4_dot(&tmp, &N);

	/*U (right) - first row */
	/* vec4_cross(&U, &N, upVec); */
	vec4_cross(&U, upVec, &N);
	outmat->_11 = U.x;
	outmat->_12 = U.y;
	outmat->_13 = U.z;
	outmat->_14 = vec4_dot(&tmp, &U);

	/*V (up) second row */
	/* vec4_cross(&V, &N, &U); */
	vec4_cross(&V, &U, &N);
	outmat->_21 = V.x;
	outmat->_22 = V.y;
	outmat->_23 = V.z;
	outmat->_24 = vec4_dot(&tmp, &V);

	/* fourth row looks like identity matrix */
	outmat->m[3][0] = 0.0f;
	outmat->m[3][1] = 0.0f;
	outmat->m[3][2] = 0.0f;
	outmat->m[3][3] = 1.0f;

	return outmat;
}

/* Rotation matrix around X axis */
matrix4_t* mat4_rotationX(matrix4_t *outmat, float angle)
{
	outmat->m[0][0] = 1.0f;
	outmat->m[0][1] = 0.0f;
	outmat->m[0][2] = 0.0f;
	outmat->m[0][3] = 0.0f;

	outmat->m[1][0] = 0.0f;
	outmat->m[1][1] = cos(angle*(M_PI/180.0f));
	outmat->m[1][2] = -1.0f * sin(angle*(M_PI/180.0f));
	outmat->m[1][3] = 0.0f;

	outmat->m[2][0] = 0.0f;
	outmat->m[2][1] = sin(angle*(M_PI/180.0f));
	outmat->m[2][2] = outmat->m[1][1];
	outmat->m[2][3] = 0.0f;

	outmat->m[3][0] = 0.0f;
	outmat->m[3][1] = 0.0f;
	outmat->m[3][2] = 0.0f;
	outmat->m[3][3] = 1.0f;

	return outmat;
}

/* Rotation matrix around Y axis */
matrix4_t* mat4_rotationY(matrix4_t *outmat, float angle)
{
	outmat->m[0][0] = cos(angle*(M_PI/180.0f));
	outmat->m[0][1] = 0.0f;
	outmat->m[0][2] = sin(angle*(M_PI/180.0f));
	outmat->m[0][3] = 0.0f;

	outmat->m[1][0] = 0.0f;
	outmat->m[1][1] = 1.0f;
	outmat->m[1][2] = 0.0f;
	outmat->m[1][3] = 0.0f;

	outmat->m[2][0] = -1.0f * sin(angle*(M_PI/180.0f));
	outmat->m[2][1] = 0.0f;
	outmat->m[2][2] = outmat->m[0][0];
	outmat->m[2][3] = 0.0f;

	outmat->m[3][0] = 0.0f;
	outmat->m[3][1] = 0.0f;
	outmat->m[3][2] = 0.0f;
	outmat->m[3][3] = 1.0f;

	return outmat;
}

/* Rotation matrix around Z axis */
matrix4_t* mat4_rotationZ(matrix4_t *outmat, float angle)
{
	outmat->m[0][0] = cos(angle*(M_PI/180.0f));
	outmat->m[0][1] = 0.0f;
	outmat->m[0][2] = -1.0 * sin(angle*(M_PI/180.0f));
	outmat->m[0][3] = 0.0f;

	outmat->m[1][0] = sin(angle*(M_PI/180.0f));
	outmat->m[1][1] = cos(angle*(M_PI/180.0f));
	outmat->m[1][2] = 0.0f;
	outmat->m[1][3] = 0.0f;

	outmat->m[2][0] = 0.0f;
	outmat->m[2][1] = 0.0f;
	outmat->m[2][2] = 1.0f;
	outmat->m[2][3] = 0.0f;

	outmat->m[3][0] = 0.0f;
	outmat->m[3][1] = 0.0f;
	outmat->m[3][2] = 0.0f;
	outmat->m[3][3] = 1.0f;

	return outmat;
}

/* Generate translation matrix */
matrix4_t* mat4_translation(matrix4_t *outmat, float x, float y, float z)
{
	mat4_identity(outmat);

	outmat->_14 = x;
	outmat->_24 = y;
	outmat->_34 = z;

	return outmat;
}

/* Generate scale matrix */
matrix4_t* mat4_scale(matrix4_t *outmat, float sx, float sy, float sz)
{
	mat4_identity(outmat);
	
	outmat->_11 = sx;
	outmat->_22 = sy;
	outmat->_33 = sz;

	return outmat;
}

/* debug output of a matrix */
void mat4_output(const matrix4_t *mat)
{
	unsigned int i = 0;
	printf("[");
	for( ; i < 4; ++i)
	{
		printf("[%f %f %f %f]", 
			mat->m[i][0],
			mat->m[i][1],
			mat->m[i][2],
			mat->m[i][3]);
		if(i != 3)
			printf("\n");
	}
	printf("]\n");
}
