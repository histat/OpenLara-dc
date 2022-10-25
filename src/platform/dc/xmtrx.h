#ifndef XMTRX_H
#define XMTRX_H

#include <stddef.h>

typedef float __attribute__((aligned(8))) xMatrix[4][4];
typedef float __attribute__((aligned(32))) xMatrixA32[4][4];

/*
	Binary Angle
	A 16-bit value that is a ratio of 2*pi
	Declared as 32-bit value to improve C/C++ code generation
	
	0x0000 = 0 (0 deg)
	0x4000 = pi/2 (90 deg)
	0x8000 = pi (180 deg)
	0xc000 = 3/2*pi (270 deg)
	0xffff = almost 2*pi (almost 360 deg)
*/
typedef int BinAngle;

// #define BINANGLE_TO_DEG(angle) ((float)(angle) * )

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
//Sets XMTRX to the identity matrix
void xmtrxIdentity(void);
//Sets XMTRX to the zero matrix
void xmtrxZero(void);
//Sets XMTRX to the identity matrix. Existing matrix must not have any NaNs or infinties
void xmtrxIdentityFast(void);
//Loads a column major matrix from memory into XMTRX
void xmtrxLoad(xMatrix *matrix);
//Loads a row major matrix from memory into XMTRX
void xmtrxLoadTranspose(xMatrix *matrix);
//Saves XMTRX to memory (column major)
void xmtrxStore(xMatrix *matrix);
//Saves XMTRX to memory (row major)
void xmtrxStoreTranspose(xMatrix *matrix);

void xmtrxLoadCol0(float x, float y, float z, float w);
void xmtrxLoadCol1(float x, float y, float z, float w);
void xmtrxLoadCol2(float x, float y, float z, float w);
void xmtrxLoadCol3(float x, float y, float z, float w);
void xmtrxLoadRow0(float x, float y, float z, float w);
void xmtrxLoadRow1(float x, float y, float z, float w);
void xmtrxLoadRow2(float x, float y, float z, float w);
void xmtrxLoadRow3(float x, float y, float z, float w);

//Sets matrix stack pointer
//Correct size is required, but stack is not bounds checked
//Stack must be at least 8-byte aligned, ideally, stack is 32-byte aligned
void xmtrxSetStackPointer(xMatrix *stack, size_t size);
//Pop XMTRX from matrix stack
void xmtrxPop();
//Load top of matrix stack without popping
void xmtrxPeek();
//Push XMTRX to matrix stack
void xmtrxPush();
//Removes top item from matrix stack. XMTRX is unchanged
void xmtrxDrop();
void xmtrxTest();

//XMTRX * right_matrix -> XMTRX
void xmtrxMultiply(xMatrix *right_matrix);
//left_matrix * XMTRX -> XMTRX
void xmtrxReverseMultiply(xMatrix *left_matrix);
//XMTRX * scalar -> XMTRX
void xmtrxMultiplyScalar(float scalar);
//XMTRX * matrixa * matrixb -> XMTRX
void xmtrxMultiplyPair(xMatrix *right_matrixa, xMatrix *right_matrixb);
//left_matrix * right_matrix -> XMTRX
void xmtrxLoadMultiply(xMatrix *left_matrix, xMatrix *right_matrix);
//Transposed XMTRX -> XMTRX
void xmtrxTranspose();

//XMTRX * TranslationMatrix -> XMTRX
void xmtrxTranslate(float x, float y, float z);
//XMTRX * ScaleMatrix -> XMTRX
void xmtrxScale(float x, float y, float z);

//XMTRX * RotationMatrix -> XMTRX
//Rotate around arbitary axis, axis must be normalized
void xmtrxRotate(BinAngle angle, float axis_x, float axis_y, float axis_z);
//Roate around arbitary axis, axis will be normalized
void xmtrxRotateNormalize(BinAngle angle, float axis_x, float axis_y, float axis_z);
void xmtrxRotateIX(BinAngle x);	//Rotate around X axis
void xmtrxRotateIY(BinAngle y);	//Rotate around Y axis
void xmtrxRotateIZ(BinAngle z);	//Rotate around Z axis
//XMTRX * RotationX * RotationY * RotationZ -> XMTRX
void xmtrxRotateIXYZ(BinAngle x, BinAngle y, BinAngle z);
//XMTRX * RotationZ * RotationY * RotationX -> XMTRX
void xmtrxRotateIZYX(BinAngle z, BinAngle y, BinAngle x);

//Same as above rotation matrices, but in radians
void xmtrxRotateFX(float x);
void xmtrxRotateFY(float y);
void xmtrxRotateFZ(float z);
void xmtrxRotateFXYZ(float x, float y, float z);
void xmtrxRotateFZYX(float z, float y, float x);

//OpenGL rotate
void xmtrxRotateNormalizeDeg(float degrees, float axis_x, float axis_y, float axis_z);

//4-byte aligned operations
void xmtrxLoadUnaligned(float *matrix);
void xmtrxStoreUnaligned(float *matrix);
void xmtrxMultiplyUnaligned(float *right_matrix);

//Prints XMTRX to stdout
void xmtrxPrint();
//Prints XMTRX to stdout, followed by a blank line
void xmtrxPrintLn();


void mat_perspective_no_viewport(float cot_fovy_2, float znear, float zfar);
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
