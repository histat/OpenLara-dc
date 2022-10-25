#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "xmtrx.h"

//Private to xmtrx library
xMatrix * _xmtrxStackPtr;

void xmtrxSetStackPointer(xMatrix *stack, size_t size) {
#ifndef	XMTRX_USE_GBR_STACK
	_xmtrxStackPtr = ((void*)stack) + size;
#else
	#error No gbr support yet
#endif
}

void xmtrxPrint() {
	xMatrix __attribute__((aligned(8))) tempmat;
	xmtrxStore(&tempmat);
	
	printf("[ %10.3f %10.3f %10.3f %10.3f ]\n", tempmat[0][0],tempmat[1][0],tempmat[2][0],tempmat[3][0]); 
	printf("[ %10.3f %10.3f %10.3f %10.3f ]\n", tempmat[0][1],tempmat[1][1],tempmat[2][1],tempmat[3][1]); 
	printf("[ %10.3f %10.3f %10.3f %10.3f ]\n", tempmat[0][2],tempmat[1][2],tempmat[2][2],tempmat[3][2]); 
	printf("[ %10.3f %10.3f %10.3f %10.3f ]\n", tempmat[0][3],tempmat[1][3],tempmat[2][3],tempmat[3][3]); 
}
void xmtrxPrintLn() {
	xmtrxPrint();
	printf("\n");
}

int xmtrxEqualsDelta(xMatrix *other, float delta) {
	xMatrix __attribute__((aligned(8))) tempmat;
	xmtrxStore(&tempmat);
	
	int x,y;
	for(x = 0; x < 4; x++) {
		for(y = 0; y < 4; y++) {
			if (fabsf(tempmat[y][x] - (*other)[y][x]) > delta)
				return 0;
		}
	}
	return 1;
}

void mat_perspective_no_viewport(float cot_fovy_2, float znear, float zfar) {
	//From mat_perspective
	
	xMatrix __attribute__((aligned(8))) fr_mat;
	/* Setup the frustum matrix */
	assert((znear - zfar) != 0);
	fr_mat[0][0] = fr_mat[1][1] = cot_fovy_2;
	fr_mat[2][2] = (zfar + znear) / (znear - zfar);
	fr_mat[3][2] = 2 * zfar * znear / (znear - zfar);
	xmtrxMultiply(&fr_mat);
}


void xmtrxInverseTranspose()
{
	float src[16];
	float __attribute__((aligned(8))) dst[16];
	int j;
	xmtrxStoreTranspose((xMatrix*)&src);
	
	float det, tmp[12];
	
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	
	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
	
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];
	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];
	
	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
	
	/* calculate determinant */
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
	
	/* calculate matrix inverse */
	det = 1/det;
	for (j = 0; j < 16; j++)
		dst[j] *= det;
	
	xmtrxLoad((xMatrix*)&dst);
}

void xmtrxFrustum(float left, float right, float bottom, float top, float znear, float zfar) {
	//UNTESTED
	xMatrix __attribute__((aligned(8))) matrix;
	float nearx2, rightminusleft, topminusbottom, farminusnear;
	nearx2 = 2.0 * znear;
	rightminusleft = right - left;
	topminusbottom = top - bottom;
	farminusnear = zfar - znear;
	
	matrix[0][0] = nearx2 / rightminusleft;
	matrix[0][1] = 0.0;
	matrix[0][2] = 0.0;
	matrix[0][3] = 0.0;
	
	matrix[1][0] = 0.0;
	matrix[1][1] = nearx2 / topminusbottom;
	matrix[1][2] = 0.0;
	matrix[1][3] = 0.0;
	
	matrix[2][0] = (right + left) / rightminusleft;
	matrix[2][1] = (top + bottom) / topminusbottom;
	//matrix[2][2] = (-zfar - znear) / farminusnear;
	matrix[2][2] = zfar/farminusnear;
	matrix[2][3] = -1.0;
	
	matrix[3][0] = 0.0;
	matrix[3][1] = 0.0;
	//matrix[3][2] = (-nearx2 * zfar) / farminusnear;
	matrix[3][2] = -(zfar * znear) / farminusnear;
	matrix[3][3] = 0.0;
	
	xmtrxMultiply(&matrix);
}

/*
void glFrustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal) {
	dcglNotInBegin();
	static __attribute__((aligned(32))) matrix_t mat = {
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0,-1},
		{0, 0, 0, 0}};
	
	assert(nearVal > 0);
	assert(farVal > 0);
	assert(left != right);
	assert(top != bottom);
	assert(nearVal != farVal);
	
	mat[0][0] = 2*nearVal/(right-left);
	mat[1][1] = 2*nearVal/(top-bottom);
	mat[2][0] = (right+left)/(right-left);
	mat[2][1] = (top+bottom)/(top-bottom);
	//fru[2][2] = -(farVal+nearVal)/(farVal-nearVal);
	//fru[3][2] = -(2*farVal*nearVal)/(farVal-nearVal);
	mat[2][2] = farVal/(farVal-nearVal);
	mat[3][2] = -(farVal*nearVal)/(farVal-nearVal);
	
	xmtrxMultiply(&mat);
}
*/

