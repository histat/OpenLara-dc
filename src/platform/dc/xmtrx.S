
.macro FSCHG_AND_8_ALIGN_STACK alignedstack
	mov	#-8, \alignedstack
	fschg
	
	and	r15, \alignedstack
.endm

.macro ALIGN_8_STACK alignedstack
	mov	#-8, \alignedstack
	
	and	r15, \alignedstack
.endm

.macro FADD3 a,b,dst
	fmov	\a, \dst
	fadd	\b, \dst
.endm
.macro FSUB3 a,b,dst
	fmov	\a, \dst
	fsub	\b, \dst
.endm
.macro FMUL3 a,b,dst
	fmov	\a, \dst
	fmul	\b, \dst
.endm
.macro FDIV3 a,b,dst
	fmov	\a, \dst
	fdiv	\b, \dst
.endm


#define SIZE_OF_MATRIX	(4*4*4)

#define fm00 fr0
#define fm01 fr1
#define fm02 fr2
#define fm03 fr3
#define fm10 fr4
#define fm11 fr5
#define fm12 fr6
#define fm13 fr7
#define fm20 fr8
#define fm21 fr9
#define fm22 fr10
#define fm23 fr11
#define fm30 fr12
#define fm31 fr13
#define fm32 fr14
#define fm33 fr15

/***************************************************************************
	Load/store xmtrx
 ***************************************************************************/
 //void xmtrxIdentity(void);
 //17 cycles
 .globl _xmtrxIdentity
_xmtrxIdentity:
		fschg
		fldi0	fr0
		fldi0	fr1
		
		fldi1	fr2
		fldi0	fr3
		
		fldi0	fr4
		fldi1	fr5

		fmov	dr2, xd0
		fmov	dr0, xd2
		fmov	dr4, xd4
		fmov	dr0, xd6
		fmov	dr0, xd8
		fmov	dr2, xd10
		fmov	dr0, xd12
		fmov	dr4, xd14

		rts
		 fschg
		 
 //void xmtrxIdentity(void);
 //15 cycles
 .globl _xmtrxIdentityOpt
_xmtrxIdentityOpt:
		frchg
		
		fschg
		fldi1	fr0
		fldi0	fr1
		
		fldi0	fr2
		fldi0	fr3
		
		fldi0	fr4
		fldi1	fr5

		fmov	dr0, dr10
		fmov	dr4, dr14
		fmov	dr2, dr6
		fmov	dr2, dr8
		fmov	dr2, dr12
		frchg

		rts
		 fschg
 
//void xmtrxZero(void);
//13 cycles
.globl _xmtrxZero
_xmtrxZero:
		fldi0	fr0
		fschg
		fldi0	fr1
		
		fmov	dr0, xd0
		fmov	dr0, xd2
		fmov	dr0, xd4
		fmov	dr0, xd6
		fmov	dr0, xd8
		fmov	dr0, xd10
		fmov	dr0, xd12
		fmov	dr0, xd14

		rts
		 fschg
 
// a faster way to load the identity matrix. current matrix cannot contain infinities or NaNs
// 12 cycles
.globl _xmtrxIdentityFast
_xmtrxIdentityFast:
		frchg
		
		fldi0	fr1
		fmul	fr1,fr2
		
		fldi1	fr0
		fmul	fr1,fr3
		
		fldi0	fr4
		fmul	fr1,fr6
		
		fldi1	fr5
		fschg
		
		fmul	fr1,fr7
		fmov	dr0,dr10
		
		fmul	fr1,fr8
		fmov	dr4,dr14
		
		fmul	fr1,fr9
		fmov	dr2,dr12
		
		fschg
		
		rts
		 frchg

.macro LOAD_MATRIX src
	fmov	@\src+,xd0
	fmov	@\src+,xd2
	fmov	@\src+,xd4
	fmov	@\src+,xd6
	fmov	@\src+,xd8
	fmov	@\src+,xd10
	fmov	@\src+,xd12
	fmov	@\src+,xd14
.endm

//dst MUST point to just past END of matrix!
.macro STORE_MATRIX dst
	fmov	xd14, @-\dst
	fmov	xd12, @-\dst
	fmov	xd10, @-\dst
	fmov	xd8, @-\dst
	fmov	xd6, @-\dst
	fmov	xd4, @-\dst
	fmov	xd2, @-\dst
	fmov	xd0, @-\dst
.endm

//void xmtrxLoad(float *matrix);
.globl _xmtrxLoad
_xmtrxLoad:
		fschg
		

		LOAD_MATRIX	r4

		rts
		 fschg

//void xmtrxStore(float *matrix);
.globl _xmtrxStore
_xmtrxStore:
		fschg
		add	#SIZE_OF_MATRIX, r4

		STORE_MATRIX	r4

		rts
		 fschg

//void xmtrxLoadTranspose(float *matrix);
.globl _xmtrxLoadTranspose
_xmtrxLoadTranspose:
		frchg
		
		fmov	@r4+,fr0
		fmov	@r4+,fr4
		fmov	@r4+,fr8
		fmov	@r4+,fr12
		
		fmov	@r4+,fr1
		fmov	@r4+,fr5
		fmov	@r4+,fr9
		fmov	@r4+,fr13
		
		fmov	@r4+,fr2
		fmov	@r4+,fr6
		fmov	@r4+,fr10
		fmov	@r4+,fr14
		
		fmov	@r4+,fr3
		fmov	@r4+,fr7
		fmov	@r4+,fr11
		fmov	@r4+,fr15
		
		rts
		 frchg

//void xmtrxStoreTranspose(float *matrix);
 .globl _xmtrxStoreTranspose
_xmtrxStoreTranspose:
		add	#64,r4
		frchg
		
		fmov	fr15,@-r4
		fmov	fr11,@-r4
		fmov	fr7,@-r4
		fmov	fr3,@-r4
		
		fmov	fr14,@-r4
		fmov	fr10,@-r4
		fmov	fr6,@-r4
		fmov	fr2,@-r4
		
		fmov	fr13,@-r4
		fmov	fr9,@-r4
		fmov	fr5,@-r4
		fmov	fr1,@-r4
		
		fmov	fr12,@-r4
		fmov	fr8,@-r4
		fmov	fr4,@-r4
		fmov	fr0,@-r4
		
		rts
		 frchg
		 
//void xmtrxLoadUnaligned(float *matrix);
.globl _xmtrxLoadUnaligned
_xmtrxLoadUnaligned:
		frchg
		
		fmov	@r4+,fr0
		fmov	@r4+,fr1
		fmov	@r4+,fr2
		fmov	@r4+,fr3
		
		fmov	@r4+,fr4
		fmov	@r4+,fr5
		fmov	@r4+,fr6
		fmov	@r4+,fr7
		
		fmov	@r4+,fr8
		fmov	@r4+,fr9
		fmov	@r4+,fr10
		fmov	@r4+,fr11
		
		fmov	@r4+,fr12
		fmov	@r4+,fr13
		fmov	@r4+,fr14
		fmov	@r4+,fr15
		
		rts
		 frchg

//void xmtrxStoreUnaligned(float *matrix);
 .globl _xmtrxStoreUnaligned
_xmtrxStoreUnaligned:
		add	#64,r4
		frchg
		
		fmov	fr15,@-r4
		fmov	fr14,@-r4
		fmov	fr13,@-r4
		fmov	fr12,@-r4
		
		fmov	fr11,@-r4
		fmov	fr10,@-r4
		fmov	fr9,@-r4
		fmov	fr8,@-r4
		
		fmov	fr7,@-r4
		fmov	fr6,@-r4
		fmov	fr5,@-r4
		fmov	fr4,@-r4
		
		fmov	fr3,@-r4
		fmov	fr2,@-r4
		fmov	fr1,@-r4
		fmov	fr0,@-r4
		
		rts
		 frchg
		 
//void xmtrxLoadCol0(float x, float y, float z, float w);
.globl _xmtrxLoadCol0
_xmtrxLoadCol0:
		fschg
		fmov	dr4, xd0
		fmov	dr6, xd2
		rts
		 fschg
//void xmtrxLoadCol1(float x, float y, float z, float w);
.globl _xmtrxLoadCol1
_xmtrxLoadCol1:
		fschg
		fmov	dr4, xd4
		fmov	dr6, xd6
		rts
		 fschg
//void xmtrxLoadCol2(float x, float y, float z, float w);
.globl _xmtrxLoadCol2
_xmtrxLoadCol2:
		fschg
		fmov	dr4, xd8
		fmov	dr6, xd10
		rts
		 fschg
//void xmtrxLoadCol3(float x, float y, float z, float w);
.globl _xmtrxLoadCol3
_xmtrxLoadCol3:
		fschg
		fmov	dr4, xd12
		fmov	dr6, xd14
		rts
		 fschg
//void xmtrxLoadRow0(float x, float y, float z, float w);
.globl _xmtrxLoadRow0
_xmtrxLoadRow0:
		fmov	fr4, @-r15
		fmov	fr5, @-r15
		fmov	fr6, @-r15
		fmov	fr7, @-r15
		frchg
		fmov	@r15+, fr0
		fmov	@r15+, fr4
		fmov	@r15+, fr8
		fmov	@r15+, fr12
		rts
		 frchg
		 
//void xmtrxLoadRow1(float x, float y, float z, float w);
.globl _xmtrxLoadRow1
_xmtrxLoadRow1:
		fmov	fr4, @-r15
		fmov	fr5, @-r15
		fmov	fr6, @-r15
		fmov	fr7, @-r15
		frchg
		fmov	@r15+, fr1
		fmov	@r15+, fr5
		fmov	@r15+, fr9
		fmov	@r15+, fr13
		rts
		 frchg
		 
//void xmtrxLoadRow2(float x, float y, float z, float w);
.globl _xmtrxLoadRow2
_xmtrxLoadRow2:
		fmov	fr4, @-r15
		fmov	fr5, @-r15
		fmov	fr6, @-r15
		fmov	fr7, @-r15
		frchg
		fmov	@r15+, fr2
		fmov	@r15+, fr6
		fmov	@r15+, fr10
		fmov	@r15+, fr14
		rts
		 frchg
		 
//void xmtrxLoadRow3(float x, float y, float z, float w);
.globl _xmtrxLoadRow3
_xmtrxLoadRow3:
		fmov	fr4, @-r15
		fmov	fr5, @-r15
		fmov	fr6, @-r15
		fmov	fr7, @-r15
		frchg
		fmov	@r15+, fr3
		fmov	@r15+, fr7
		fmov	@r15+, fr11
		fmov	@r15+, fr15
		rts
		 frchg
		 

/* 	GET_CUR_STACK_PTR_R0_R1 loads a pointer to the top of stack into r0,
	and saves a pointer to where the top of stack is saved
	UPDATE_CUR_STACK_PTR_R0_R1 saves the value in r0 to memory, using the
	value in R1
*/
#ifndef	XMTRX_USE_GBR_STACK
	.macro GET_CUR_STACK_PTR_R0_R1
		mov.l	.Lstackptr, r1
		mov.l	@r1, r0
	.endm
	.macro UPDATE_CUR_STACK_PTR_R0_R1
		mov.l	r0, @r1
	.endm
#else
	.macro GET_CUR_STACK_PTR_R0_R1
		mov.l	@(GBR_MAT_STACK, gbr), r0
	.endm
	.macro UPDATE_CUR_STACK_PTR_R0_R1
		mov.l	r0, @(GBR_MAT_STACK, gbr)
	.endm
#endif

.globl _xmtrxTest
_xmtrxTest:
		GET_CUR_STACK_PTR_R0_R1
		mov	#0, r0
		UPDATE_CUR_STACK_PTR_R0_R1
		rts
		 nop

.globl _xmtrxPop
_xmtrxPop:
		fschg
		GET_CUR_STACK_PTR_R0_R1
		LOAD_MATRIX	r0
		UPDATE_CUR_STACK_PTR_R0_R1
		rts
		 fschg
		 
.globl _xmtrxPeek
_xmtrxPeek:
		fschg
		GET_CUR_STACK_PTR_R0_R1
		LOAD_MATRIX	r0
		rts
		 fschg
		 
.globl _xmtrxPush
_xmtrxPush:
		fschg
		GET_CUR_STACK_PTR_R0_R1
		STORE_MATRIX	r0
		UPDATE_CUR_STACK_PTR_R0_R1
		rts
		 fschg

.globl _xmtrxDrop
_xmtrxDrop:
		GET_CUR_STACK_PTR_R0_R1
		add	#SIZE_OF_MATRIX, r0
		rts
		 UPDATE_CUR_STACK_PTR_R0_R1
		 
#ifndef	XMTRX_USE_GBR_STACK
	.align 2
	.Lstackptr:
		.long __xmtrxStackPtr
#endif
/***************************************************************************
	Regular multiplies
 ***************************************************************************/
 //void xmtrxLoadMultiply(float *left_matrix, float *right_matrix);
.globl _xmtrxLoadMultiply
_xmtrxLoadMultiply:
		FSCHG_AND_8_ALIGN_STACK	r0
		
		LOAD_MATRIX	r4

		bra	.Lmultiply_entry
		 frchg
 
 //void xmtrxMultiplyPair(float *right_matrixa, float *right_matrixb);
.globl _xmtrxMultiplyPair
_xmtrxMultiplyPair:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fmov	dr14, @-r0
		
		fmov	@r4+, dr0
		fmov	@r4+, dr2
		fmov	@r4+, dr4
		fmov	@r4+, dr6
		ftrv	xmtrx, fv0
		fmov	@r4+, dr8
		fmov	@r4+, dr10
		ftrv	xmtrx, fv4
		fmov	@r4+, dr12
		fmov	@r4+, dr14
		ftrv	xmtrx, fv8
		mov	r5, r4
		ftrv	xmtrx, fv12
		bra	.Lmultiply_entry
		 frchg
		 
//void xmtrxMultiply(float *right_matrix);
.globl _xmtrxMultiply
_xmtrxMultiply:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fmov	dr14, @-r0
		
	.Lmultiply_entry:
		fmov	@r4+, dr0
		fmov	@r4+, dr2
		fmov	@r4+, dr4
		fmov	@r4+, dr6
		ftrv	xmtrx, fv0
		fmov	@r4+, dr8
		fmov	@r4+, dr10
		ftrv	xmtrx, fv4
		fmov	@r4+, dr12
		fmov	@r4+, dr14
		ftrv	xmtrx, fv8
		ftrv	xmtrx, fv12
		
		frchg	//TODO would it be faster to restore registers into xd14,xd12, then do frchg?
		fmov	@r0+,dr14
		fmov	@r0+,dr12
		rts
		 fschg

//void xmtrxMultiplyUnaligned(float *right_matrix);
.globl _xmtrxMultiplyUnaligned
_xmtrxMultiplyUnaligned:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fmov	dr14, @-r0
		fschg
	
		fmov	@r4+, fr0
		fmov	@r4+, fr1
		fmov	@r4+, fr2
		fmov	@r4+, fr3
		fmov	@r4+, fr4
		fmov	@r4+, fr5
		ftrv	xmtrx, fv0
		fmov	@r4+, fr6
		fmov	@r4+, fr7
		fmov	@r4+, fr8
		fmov	@r4+, fr9
		ftrv	xmtrx, fv4
		fmov	@r4+, fr10
		fmov	@r4+, fr11
		fmov	@r4+, fr12
		fmov	@r4+, fr13
		ftrv	xmtrx, fv8
		fmov	@r4+, fr14
		fmov	@r4+, fr15
		ftrv	xmtrx, fv12
		
		fschg
		fmov	@r0+,xd14
		frchg
		fmov	@r0+,dr12
		rts
		 fschg
		 
//void xmtrxReverseMultiply(float *left_matrix);
.globl _xmtrxReverseMultiply
_xmtrxReverseMultiply:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fmov	dr14, @-r0

		fmov	@r4+, dr0
		fmov	@r4+, dr2
		fmov	@r4+, dr4
		fmov	@r4+, dr6
		fmov	@r4+, dr8
		fmov	@r4+, dr10
		fmov	@r4+, dr12
		fmov	@r4+, dr14
		frchg
		ftrv	xmtrx, fv0
		ftrv	xmtrx, fv4
		ftrv	xmtrx, fv8
		ftrv	xmtrx, fv12
		frchg

		fmov	@r0+, dr14
		fmov	@r0+, dr12
		rts
		 fschg
		 
//void xmtrxMultiplyScalar(float scalar);
.globl _xmtrxMultiplyScalar
_xmtrxMultiplyScalar:
		fschg
		fmov	xd0,dr0
		
		fmul	fr4,fr0
		fmov	dr4,xd0
		
		fmul	fr4,fr1
		
		frchg
		
	.irp	cnt,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		fmul	fr0,fr\cnt
	.endr
		fmov	xd0,dr0
		
		fschg
		
		rts
		frchg
		
/***************************************************************************
	Modify matrix
 ***************************************************************************/
//void xmtrxTranspose();
//22 cycles
.globl _xmtrxTranspose
_xmtrxTranspose:
.macro fswap a, b
	flds	\a, fpul
	fmov	\b, \a
	fsts	fpul, \b
.endm
 /*
 0  4  8 12  
 1  5  9 13
 2  6 10 14
 3  7 11 15 */
		frchg
		fswap	fr1, fr4
		fswap	fr2, fr8
		fswap	fr3, fr12
		fswap	fr6, fr9
		fswap	fr7, fr13
		fswap	fr11, fr14 
		rts
		 frchg
 
 
 //void xmtrxTransposeLS();
 .globl _xmtrxTransposeLS
 _xmtrxTransposeLS:
		FSCHG_AND_8_ALIGN_STACK	r0
		
		fmov	xd14, @-r0
		fmov	xd12, @-r0
		fmov	xd10, @-r0
		fmov	xd8, @-r0
		fmov	xd6, @-r0
		fmov	xd4, @-r0
		fmov	xd2, @-r0
		fmov	xd0, @-r0
		
		fschg
		frchg
		
		fmov	@r0, fm23	!0
		add	#8, r0
		fmov	@r0+, fm03	!8
		fmov	@r0, fm13	!12
		add	#8, r0
		fmov	@r0+, fm32	!20
		fmov	@r0+, fm02	!24
		fmov	@r0+, fm12	!28
		fmov	@r0+, fm21	!32
		fmov	@r0+, fm31	!36
		fmov	@r0, fm01	!40
		add	#8, r0
		fmov	@r0+, fm20	!48
		fmov	@r0, fm30	!52
		add	#8, r0
		fmov	@r0+, fm10	!60
		
		rts
		 frchg
		
/***************************************************************************
	Multiply with special matrix
 ***************************************************************************/
//void xmtrxTranslate(float x, float y, float z);
.globl _xmtrxTranslate
_xmtrxTranslate:
		FSCHG_AND_8_ALIGN_STACK	r0
		
		fmov	dr12,@-r0
		fmov	dr14,@-r0
		
		fmov	dr4,dr12
		fmov	dr6,dr14
		fldi1	fr15
		
		fldi1	fm00
		fldi0	fm01
		ftrv	xmtrx,fv12
		fldi0	fm02
		fldi0	fm03
		
		fldi0	fm10
		fldi1	fm11
		ftrv	xmtrx,fv0
		fldi0	fm12
		fldi0	fm13
		
		fldi0	fm20
		fldi0	fm21
		ftrv	xmtrx,fv4
		fldi1	fm22
		fldi0	fm23
		ftrv	xmtrx,fv8
		
		frchg
		
		fmov	@r0+,dr14
		fmov	@r0+,dr12
		rts
		 fschg

//void xmtrxScale(float x, float y, float z);		 
.globl _xmtrxScale
_xmtrxScale:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12,@-r0
		fmov	dr14,@-r0
		fschg
		
		fmov	fr4,fm00
		fldi0	fm01
		fldi0	fm02
		fldi0	fm03
		
		fldi0	fm10
		fmov 	fr6,fm22
		ftrv	xmtrx,fv0
		fldi0	fm12
		fldi0	fm13
		
		fldi0	fm20
		fldi0	fm21
		ftrv	xmtrx,fv4
		fldi0	fm23
		
		fldi0	fm30
		fldi0	fm31
		fldi0	fm32
		ftrv	xmtrx,fv8
		fldi1	fm33
		
		ftrv	xmtrx,fv12
		
		fschg
		frchg
		fmov	@r0+,dr14
		fmov	@r0+,dr12
		rts
		 fschg
		
//void xmtrxRotateIXYZ(BinAngle x, BinAngle y, BinAngle z);
.globl _xmtrxRotateIXYZ
_xmtrxRotateIXYZ:
		sts.l	pr,@-r15
		bsr	_xmtrxRotateIX
		 nop
		
		bsr	_xmtrxRotateIY
		 mov	r5,r4
		
		lds.l	@r15+,pr
		bra	_xmtrxRotateIZ
		 mov	r6,r4
	
//void xmtrxRotateIZYX(BinAngle z, BinAngle y, BinAngle x);
.globl _xmtrxRotateIZYX
_xmtrxRotateIZYX:
		sts.l	pr,@-r15
		bsr	_xmtrxRotateIZ
		 nop
		
		bsr	_xmtrxRotateIY
		 mov	r5,r4
		
		lds.l	@r15+,pr
		bra	_xmtrxRotateIX
		 mov	r6,r4

//void xmtrxRotateIX(BinAngle x);
.globl _xmtrxRotateIX
_xmtrxRotateIX:
		//TODO might be faster without fschgs?
		lds	r4, fpul
	.Lfxentry:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fsca	fpul, dr6
		fmov	dr14, @-r0
		fschg
		
		fldi1	fm00	!1st col
		fldi0	fm01
		fldi0	fm02
		fldi0	fm03
		
		fldi0	fm30	!4th col
		fldi0	fm31
		ftrv	xmtrx, fv0
		fldi0	fm32
		fldi1	fm33
		
		fldi0	fm20	!3rd col
		fmov	fm12, fm21
		fneg	fm21
		ftrv	xmtrx, fv12
		fmov	fm13, fm22
		fldi0	fm23
		
		fmov	fm13, fm11	!2nd col
		fldi0	fm10
		ftrv	xmtrx, fv8
		fldi0	fm13
		ftrv	xmtrx, fv4

		fschg
		frchg
		
		fmov	@r0+, dr14
		fmov	@r0+, dr12
		rts
		 fschg


//void xmtrxRotateIY(BinAngle y);
.globl _xmtrxRotateIY
_xmtrxRotateIY:
		//TODO might be faster without fschgs?
		lds	r4, fpul
	.Lfyentry:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fsca	fpul, dr2
		fmov	dr14, @-r0
		fschg
		
		fldi0	fm10	!2nd col
		fldi1	fm11
		fldi0	fm12
		fldi0	fm13
		
		fldi0	fm30	!4th col
		fldi0	fm31
		ftrv	xmtrx, fv4
		fldi0	fm32
		fldi1	fm33
		
		fmov	fm02, fm20	!1st col
		ftrv	xmtrx, fv12
		fldi0	fm21
		fmov	fm03, fm22
		fldi0	fm23
		
		fmov	fm03, fm00	!2nd col
		fldi0	fm01
		ftrv	xmtrx, fv8
		fneg	fm02
		fldi0	fm03
		ftrv	xmtrx, fv0
		
		fschg
		frchg
		
		fmov	@r0+, dr14
		fmov	@r0+, dr12
		rts
		 fschg
		
//void xmtrxRotateIZ(BinAngle z);
.globl _xmtrxRotateIZ
_xmtrxRotateIZ:
		//TODO might be faster without fschgs?
		lds r4,fpul
	.Lfzentry:
		FSCHG_AND_8_ALIGN_STACK	r0
		fmov	dr12, @-r0
		fsca	fpul, dr4	!sin -> fm10, cos -> fm11
		fmov	dr14, @-r0
		fschg

		fldi0	fm30	!4th col
		fldi0	fm31
		fldi0	fm32
		fldi1	fm33
		
		fldi0	fm20	!3rd col
		fldi0	fm21
		ftrv	xmtrx, fv12
		fldi1	fm22
		fldi0	fm23
		
		fmov	fm10, fm01	!1st col
		fmov	fm11, fm00
		ftrv	xmtrx, fv8
		fldi0	fm02
		fldi0	fm03
		
		fneg	fm10
		fldi0	fm12	!2nd col
		fldi0	fm13
		fschg
		ftrv	xmtrx,fv0
		ftrv	xmtrx,fv4
		
		frchg
		
		fmov	@r0+,dr14
		fmov	@r0+,dr12
		rts
		 fschg

//void xmtrxRotateNormalize(BinAngle angle, float x, float y, float z);
//UNTESTED
.globl _xmtrxRotateNormalize
_xmtrxRotateNormalize:
		fldi0	fr7
		fipr	fv4, fv4
		fsrra	fr7
		fmul	fr7, fr4
		fmul	fr7, fr5
		fmul	fr7, fr6
	//***FALLTHROUGH**
//void xmtrxRotate(BinAngle angle, float x, float y, float z);
.globl _xmtrxRotate
_xmtrxRotate:
#define x	fr12
#define y	fr13
#define z	fr11
#define sin	fr0
#define cos	fr15
#define one_minus_cos fr7
		FSCHG_AND_8_ALIGN_STACK	r0
		lds r4,fpul
		fmov	dr12, @-r0
		fmov	dr14, @-r0
		fschg

		fsca	fpul, dr0
		fmov	fr4, x
		fmov	fr5, y
		fmov	fr6, z
		
	.Lrotate_entry:
		fldi1	one_minus_cos
		fmov	fr1, cos
		fsub	cos, one_minus_cos
		
		//Column 3
		FMUL3	x, z, fm20
		FMUL3	y, z, fm21
		FMUL3	z, z, fm22
		fmul	one_minus_cos, fm20
		fmul	one_minus_cos, fm21
		fmul	one_minus_cos, fm22
		fmac	sin, y, fm20
		fneg	sin
		fmac	sin, x, fm21
		fadd	cos, fm22
		
		//Column 2
		FMUL3	x, y, fm10
		FMUL3	y, y, fm11
		FMUL3	z, y, fm12
		fmul	one_minus_cos, fm10
		fmul	one_minus_cos, fm11
		fmul	one_minus_cos, fm12
		fmac	sin, z, fm10
		fneg	sin
		fadd	cos, fm11
		fmac	sin, x, fm12
		
		//Column 1
		fmov	sin, fr3
		FMUL3	x, x, fm00
		FMUL3	y, x, fm01
		FMUL3	z, x, fm02
		fmul	one_minus_cos, fm00
		fmul	one_minus_cos, fm01
		fmul	one_minus_cos, fm02
		fmul	fr3, z
		fmul	fr3, y
		fadd	cos, fm00
		fadd	z, fm01
		fsub	y, fm02
		
		//Row 4
		fldi0	fm03		
		fldi0	fm13
		fldi0	fm23
		ftrv	xmtrx, fv0
		//Column 4
		fldi0	fm30
		fldi0	fm31
		fldi0	fm32
		fldi1	fm33
		ftrv	xmtrx, fv4
		ftrv	xmtrx, fv8
		ftrv	xmtrx, fv12
		
		fschg
	#if 0
		fmov	@r0+,xd14
		frchg
	#else
		fmov	@r0+,dr14
		frchg
	#endif
		fmov	@r0+,dr12
		rts
		 fschg

//void xmtrxRotateNormalizeDeg(float degrees, float x, float y, float z);
.globl _xmtrxRotateNormalizeDeg
_xmtrxRotateNormalizeDeg:
		fmov	fr4, fr1
		mova	.Ldeg2binaryangle, r0
		
		fmov	@r0, fr0
		mov	#-8, r0
		
		fmov	fr7, fr4
		and	r15, r0
		
		fldi0	fr7
		fmul	fr0, fr1
		
		fschg
		
		fmov	dr12, @-r0
		
		fmov	dr14, @-r0
		fschg
		
		fipr	fv4, fv4
		
		//
		
		ftrc	fr1, fpul
		
		//
		
		//
		
		fsrra	fr7
		//
		//
		//
		fsca	fpul, dr0
		FMUL3	fr5, fr7, x
		FMUL3	fr6, fr7, y
		fmov	fr4, z
		bra	.Lrotate_entry
		 fmul	fr7, z
		
#undef x
#undef y
#undef z
#undef sin
#undef cos
#undef one_minus_cos
		
//void xmtrxRotateFXYZ(float x, float y, float z);
//UNTESTED
.globl _xmtrxRotateFXYZ
_xmtrxRotateFXYZ:
		sts.l	pr,@-r15
		
		mova	.Lrad2binaryangle, r0
		
		fmov	@r0, fr0
		
		fmov	fr12, @-r15
		
		fmov	fr13, @-r15
		fmul	fr0, fr4
		
		FMUL3	fr5, fr0, fr12
		
		FMUL3	fr6, fr0, fr13
		
		bsr	.Lfxentry
		 ftrc	fr4, fpul
		
		bsr	.Lfyentry
		 ftrc	fr12, fpul
		
		ftrc	fr13, fpul
		fmov	@r15+, fr13
		fmov	@r15+, fr12
		bra	.Lfzentry
		 lds.l	@r15+, pr
		 
//void xmtrxRotateFZYX(float z, float y, float x);
//UNTESTED
.globl _xmtrxRotateFZYX
_xmtrxRotateFZYX:
		sts.l	pr,@-r15
		
		mova	.Lrad2binaryangle, r0
		
		fmov	@r0, fr0
		
		fmov	fr12, @-r15
		
		fmov	fr13, @-r15
		fmul	fr0, fr4
		
		FMUL3	fr5, fr0, fr12
		
		FMUL3	fr6, fr0, fr13
		
		bsr	.Lfzentry
		 ftrc	fr4, fpul
		
		bsr	.Lfyentry
		 ftrc	fr12, fpul
		
		ftrc	fr13, fpul
		fmov	@r15+, fr13
		fmov	@r15+, fr12
		bra	.Lfxentry
		 lds.l	@r15+, pr
	
//void xmtrxRotateFX(float x);
//UNTESTED
.globl _xmtrxRotateFX
_xmtrxRotateFX:
		mova	.Lrad2binaryangle, r0
		fmov	@r0, fr0
		fmul	fr0, fr4
		bra	.Lfxentry
		 ftrc	fr4, fpul

//void xmtrxRotateFY(float z);
//UNTESTED
.globl _xmtrxRotateFY
_xmtrxRotateFY:
		mova	.Lrad2binaryangle, r0
		fmov	@r0, fr0
		fmul	fr0, fr4
		bra	.Lfyentry
		 ftrc	fr4, fpul

//void xmtrxRotateFZ(float z);
//UNTESTED
.globl _xmtrxRotateFZ
_xmtrxRotateFZ:
		mova	.Lrad2binaryangle, r0
		fmov	@r0, fr0
		fmul	fr0, fr4
		bra	.Lfzentry
		 ftrc	fr4, fpul
		 

		

	.align 5
	.Lrad2binaryangle:
		.float	10430.37835
	.Ldeg2binaryangle:
		.float 182.044444

//void xmtrxViewport(float devicexcenter, float deviceycenter, float devicehalfwidth, float devicehalfheight)
/*
	

	halfwidth	0		0	xcenter
	0		halfheight	0	ycenter
	0		0		1	0
	0		0		0	1
*/
.globl _xmtrxViewport
_xmtrxViewport:
		fmov	fr6, fr0
		fldi0	fr1
		fldi0	fr2
		fldi0	fr3
		fmov	fr4, fr12
		fmov	fr5, fr13
		fmov	fr7, fr5
		ftrv	xmtrx, fv0
		fldi0	fr4
		fldi0	fr6
		fldi0	fr7
		
		fldi0	fr8
		fldi0	fr9
		fldi1	fr10
		ftrv	xmtrx, fv4
		fldi0	fr11
		
		fldi0	fr14
		fldi0	fr15
		ftrv	xmtrx, fv8
		ftrv	xmtrx, fv12
		
		rts
		 frchg
#if 0
//xmtrxFrustum(float left, float right, float bottom, float top, float near, float far)
/*
	2*near/(right-left)	0			(right+left)/(right-left)	0
	0			2*near/(top-bottom)	(top+bottom)/(top-bottom)	0
	0			0			-(far+near)/(far-near)		-2*far*near/(far-near)
	0			0			-1				0
	
	2*near/(right-left)	0			(right+left)/(right-left)	0
	0			2*near/(top-bottom)	(top+bottom)/(top-bottom)	0
	0			0			-(far+near)/(far-near)		-2*far*near/(far-near)
	0			0			-1				0
*/
.globl _xmtrxFrustum
_xmtrxFrustum:
		FSCHG_AND_8_ALIGN_STACK	r1
		fmov	@-r1, dr12
		fmov	@-r1, dr14
		fschg
		
	#define	neartimestwo	fr1
	#define	near	fr2
	#define	far	fr3
	#define	left	fr4
	#define	right	fr5
	#define	bottom	fr6
	#define	top	fr7
	#define rightminusleft	fr12
	#define topminusbottom	fr13
	#define farminusnear	fr15
		
		fmov	@r15+, near
		mova	.Lfloattwo, r0
		
		fmov	@r0, neartimestwo
		
		fmov	@r15, fr3
		add	#-4, far
		
		fmul	near, neartimestwo
		
		FSUB3	right, left, rightminusleft
		FSUB3	top, bottom, topminusbottom
		FSUB3	far, near, farminusnear
		
		FDIV3	neartimestwo, rightminusleft, fr0
		FADD3	right, left, fr8
		FADD3	top, bottom, fr9
		FADD3	far, near, fr10
		fldi1	fr11
		fneg	fr11
		FDIV3	neartimestwo, topminusbottom, fr5
		
		fmov
		
		
	
	#undef	neartimestwo
	#undef	near
	#undef	far
	#undef	left
	#undef	right
	#undef	bottom
	#undef	top
	#undef rightminusleft
	#undef topminusbottom
	#undef farminusnear
		 
		fschg
		fmov	@r1+, xd14
		fmov	@r1+, xd12
		rts
		 frchg
		 
	.align 5
	.Lfloattwo
		.float	2.0f
	.Lfloathalf
		.float	0.5f
#endif
//
