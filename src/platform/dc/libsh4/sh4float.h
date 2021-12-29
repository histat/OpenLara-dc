#ifndef SH4_FLOAT_ASM_H
#define SH4_FLOAT_ASM_H

	
static inline unsigned int sh4GetFPSCR() {
	int ret;
	__asm__( "sts	fpscr,%0\n\t" : "=r" (ret) );
	return ret;
}

static inline void sh4SetFPSCR(unsigned int newfpscr) {
	__asm__( "lds	%0,fpscr\n\t" : : "r" (newfpscr) );
}

static inline float sh4Fsrra(float a)
{
        __asm__( "fsrra	%0\n\t"
		: "=f" (a)
		: "0" (a)
		: );
	
        return a;
}

/**********************************************************************
		FSCA
***********************************************************************/
#define ANGLE_TO_RAD(r)		((r) / 10430.37835f)
#define RAD_TO_ANGLE(r)		((r) * 10430.37835f)

static inline void sh4FSCARadianF(float a, float *sine, float *cosine)
{
	register float __s __asm__("fr2");
	register float __c __asm__("fr3");
	a = RAD_TO_ANGLE(a);
	
        asm(    "ftrc	%2,fpul\n\t"
                "fsca	fpul,dr2\n\t"
		: "=f" (__s), "=f" (__c)
		: "f" (a)
		: "fpul");
	
        *sine = __s; *cosine = __c;
}
#define sh4FSCARadian(angle, sine, cosine) sh4FSCARadianF(angle,&sine,&cosine)
static inline float sh4FSCARadianSine(float angle) {
	float s, c;
	sh4FSCARadian(angle, s, c);
        return s;
}
static inline float sh4FSCARadianCosine(float angle) {
	float s, c;
	sh4FSCARadian(angle, s, c);
        return c;
}
static inline float sh4FSCARadianTangent(float angle) {
	float s, c;
	sh4FSCARadian(angle, s, c);
        return s / c;
}

static inline void sh4FSCAF(int angle, float *sine, float *cosine)
{
	register float __s __asm__("fr2");
	register float __c __asm__("fr3");
	
        asm(    "lds	%2,fpul\n\t"
                "fsca	fpul,dr2\n\t"
		: "=f" (__s), "=f" (__c)
		: "r" (angle)
		: "fpul");
	
        *sine = __s; *cosine = __c;
}
#define sh4FSCA(angle, sine, cosine) sh4FSCAF(angle, &sine, &cosine)

static inline float sh4FSCASine(int angle) {
	float s, c;
	sh4FSCA(angle, s, c);
        return s;
}
static inline float sh4FSCACosine(int angle) {
	float s, c;
	sh4FSCA(angle, s, c);
        return c;
}
static inline float sh4FSCATangent(int angle) {
	float s, c;
	sh4FSCA(angle, s, c);
        return s / c;
}

/**********************************************************************
		FTRV
***********************************************************************/
/* Transform vector, without any perspective division. */
/*
	4D
*/
#define sh4FtrvFV0(x, y, z, w) { \
	register float __x __asm__("fr0") = (x); \
	register float __y __asm__("fr1") = (y); \
	register float __z __asm__("fr2") = (z); \
	register float __w __asm__("fr3") = (w); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv0\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; w = __w; \
}
#define sh4FtrvFV4(x, y, z, w) { \
	register float __x __asm__("fr4") = (x); \
	register float __y __asm__("fr5") = (y); \
	register float __z __asm__("fr6") = (z); \
	register float __w __asm__("fr7") = (w); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv4\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; w = __w; \
}
#define sh4FtrvFV8(x, y, z, w) { \
	register float __x __asm__("fr8") = (x); \
	register float __y __asm__("fr9") = (y); \
	register float __z __asm__("fr10") = (z); \
	register float __w __asm__("fr11") = (w); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv8\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; w = __w; \
}
#define sh4FtrvFV12(x, y, z, w) { \
	register float __x __asm__("fr12") = (x); \
	register float __y __asm__("fr13") = (y); \
	register float __z __asm__("fr14") = (z); \
	register float __w __asm__("fr15") = (w); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv12\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; w = __w; \
}

/*
	2D
*/
#define sh4Ftrv2DFV0(x, y) { \
	register float __x __asm__("fr0") = (x); \
	register float __y __asm__("fr1") = (y); \
	register float __z __asm__("fr2") = (0); \
	register float __w __asm__("fr3") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv0\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; \
}
#define sh4Ftrv2DFV4(x, y) { \
	register float __x __asm__("fr4") = (x); \
	register float __y __asm__("fr5") = (y); \
	register float __z __asm__("fr6") = (0); \
	register float __w __asm__("fr7") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv4\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; \
}
#define sh4Ftrv2DFV8(x, y) { \
	register float __x __asm__("fr8") = (x); \
	register float __y __asm__("fr9") = (y); \
	register float __z __asm__("fr10") = (0); \
	register float __w __asm__("fr11") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv8\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; \
}
#define sh4Ftrv2DFV12(x, y) { \
	register float __x __asm__("fr12") = (x); \
	register float __y __asm__("fr13") = (y); \
	register float __z __asm__("fr14") = (0); \
	register float __w __asm__("fr15") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv12\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; \
}

/*
	3D
*/
#define sh4Ftrv3DFV0(x, y, z) { \
	register float __x __asm__("fr0") = (x); \
	register float __y __asm__("fr1") = (y); \
	register float __z __asm__("fr2") = (z); \
	register float __w __asm__("fr3") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv0\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; \
}
#define sh4Ftrv3DFV4(x, y, z) { \
	register float __x __asm__("fr4") = (x); \
	register float __y __asm__("fr5") = (y); \
	register float __z __asm__("fr6") = (z); \
	register float __w __asm__("fr7") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv4\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; \
}
#define sh4Ftrv3DFV8(x, y, z) { \
	register float __x __asm__("fr8") = (x); \
	register float __y __asm__("fr9") = (y); \
	register float __z __asm__("fr10") = (z); \
	register float __w __asm__("fr11") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv8\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; \
}
#define sh4Ftrv3DFV12(x, y, z) { \
	register float __x __asm__("fr12") = (x); \
	register float __y __asm__("fr13") = (y); \
	register float __z __asm__("fr14") = (z); \
	register float __w __asm__("fr15") = (1); \
	__asm__ __volatile__( \
		"ftrv   xmtrx,fv12\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) ); \
	x = __x; y = __y; z = __z; \
}

/**************************************************************************
	XMTRX
 **************************************************************************/

#define _sh4_xstr(s) _sh4_str(s)
#define _sh4_str(s) #s

#define GET_XF_MACRO(name,register)	\
static inline float name()	\
{	\
	float result;	\
	asm(	\
		"frchg\n\t\
		flds	" _sh4_xstr(register) ",fpul\n\t\
		frchg\n\t\
		fsts	fpul,%0\n\t"	\
		: "=f" (result) \
		: \
		: "fpul"); \
	return result; \
} \
/* end */
 
GET_XF_MACRO(sh4GetXF0,fr0)
GET_XF_MACRO(sh4GetXF1,fr1)
GET_XF_MACRO(sh4GetXF2,fr2)
GET_XF_MACRO(sh4GetXF3,fr3)
GET_XF_MACRO(sh4GetXF4,fr4)
GET_XF_MACRO(sh4GetXF5,fr5)
GET_XF_MACRO(sh4GetXF6,fr6)
GET_XF_MACRO(sh4GetXF7,fr7)
GET_XF_MACRO(sh4GetXF8,fr8)
GET_XF_MACRO(sh4GetXF9,fr9)
GET_XF_MACRO(sh4GetXF10,fr10)
GET_XF_MACRO(sh4GetXF11,fr11)
GET_XF_MACRO(sh4GetXF12,fr12)
GET_XF_MACRO(sh4GetXF13,fr13)
GET_XF_MACRO(sh4GetXF14,fr14)
GET_XF_MACRO(sh4GetXF15,fr15)

#undef _sh4_xstr
#undef _sh4_str

#undef GET_XF_MACRO

#endif
