#ifndef SH4_ASM_H
#define SH4_ASM_H

#define NONCACHED(a) (typeof (a))(((unsigned int)(a)) |  (1 << 29))
#define CACHED(a)    (typeof (a))(((unsigned int)(a)) & ~(1 << 29))
#define OCI_BANK0(a) (typeof (a))(((unsigned int)(a)) & ~(1 << 25))
#define OCI_BANK1(a) (typeof (&(a)[0]))(((unsigned int)(a)) |  (1 << 25))

#define _sh4_xstr(s) _sh4_str(s)
#define _sh4_str(s) #s

#if 0
#define xstr(s) str(s)
#define str(s) #s
#endif

static inline void sh4Nop() {
	__asm__ __volatile__("nop");
}

/**********************************************************************
		CACHE CONTROL
***********************************************************************/
//Normal data prefetch
static inline void sh4Prefetch(const void *addr) {
	//__asm__ __volatile__("pref @%0" : : "r" (addr));
	__builtin_prefetch(addr);
}
//Store queue submit "prefetch"
static inline void sh4PrefetchMem(const void *addr) {
	__asm__ __volatile__("pref @%0" : : "r" (addr) : "memory");
}
static inline void sh4Writeback(const void *addr) {
	__asm__ __volatile__("ocbwb @%0" : : "r" (addr) : "memory");
}
static inline void sh4Invalidate(const void *addr) {
	__asm__ __volatile__("ocbi @%0" : : "r" (addr) : "memory");
}
static inline void sh4Purge(const void *addr) {
	__asm__ __volatile__("ocbp @%0" : : "r" (addr) : "memory");
}
static inline void sh4CachelineAllocate(void *addr, int value) {
	register int __value  __asm__("r0") = value;
	__asm__ __volatile__ (
		"movca.l r0,@%0\n\t"
		:  
		: "r" (addr), "r" (__value)
		: "memory"  );
}

/**********************************************************************
		GBR
***********************************************************************/
static inline void * sh4GetGBR() {
	void * value;
	__asm__("stc gbr,%0\n\t"
		: "=r" (value)
		);
	return value;
}

static inline void sh4SetGBR(void * f) {
	__asm__("ldc %0,gbr\n\t"
		: 
		: "r" (f)
		);
}

//addr MUST be a compile-time constant literal
#define sh4LoadGBRB(addr) \
({ register int __value  __asm__("r0"); \
	int __value2; \
	__asm__ __volatile__ (	\
		"mov.b @( " _sh4_xstr(addr) " ,gbr),r0\n\t" \
	: "=r" (__value) ); \
	__value2 = __value; \
	__value2; })

//addr MUST be a compile-time constant literal	
#define sh4LoadGBRW(addr) \
({ register int __value  __asm__("r0"); \
	int __value2; \
	__asm__  __volatile__ (	\
		"mov.w @( " _sh4_xstr(addr) " ,gbr),r0\n\t" \
	: "=r" (__value) ); \
	__value2 = __value; \
	__value2; })

//addr MUST be a compile-time constant literal	
#define sh4LoadGBRL(addr) \
({ register int __value  __asm__("r0"); \
	int __value2; \
	__asm__ __volatile__ (	\
		"mov.l @( " _sh4_xstr(addr) " ,gbr),r0\n\t" \
	: "=r" (__value) ); \
	__value2 = __value; \
	__value2; })

//addr MUST be a compile-time constant literal
#define sh4StoreGBRB(addr,val) \
({ register int __value  __asm__("r0") = val; \
	__asm__ __volatile__ (	\
		"mov.b r0,@( " _sh4_xstr(addr) " ,gbr)\n\t" \
	:  \
	: "r" (__value) \
	: "memory"  ); \
	__value; })

//addr MUST be a compile-time constant literal
#define sh4StoreGBRW(addr,val) \
({ register int __value  __asm__("r0") = val; \
	__asm__ __volatile__ (	\
		"mov.w r0,@( " _sh4_xstr(addr) " ,gbr)\n\t" \
	:  \
	: "r" (__value) \
	: "memory"  ); \
	__value; })
	
//addr MUST be a compile-time constant literal
#define sh4StoreGBRL(addr,val) \
({ register int __value  __asm__("r0") = val; \
	__asm__ __volatile__ (	\
		"mov.l r0,@( " _sh4_xstr(addr) " ,gbr)\n\t" \
	:  \
	: "r" (__value) \
	: "memory"  ); \
	__value; })
	
/**********************************************************************
		SWAP/XTRCT
***********************************************************************/
static inline int sh4SwapB(int f) {
	int result;
	__asm__("swap.b	%1,%0\n\t"
		: "=r" (result)
		: "r" (f)
		);
	return result;
}

static inline int sh4SwapW(int f) {
	int result;
	__asm__("swap.w %1,%0\n\t"
		: "=r" (result)
		: "r" (f)
		);
	return result;
}

static inline int sh4ByteSwap(int f) {
	return sh4SwapB(sh4SwapW(sh4SwapB(f)));
}

static inline int sh4Xtrct(int l, int r) {
	int value = (r), arg = (l);
	__asm__("xtrct	%1,%0\n\t"
		: "=r" (value)
		: "r" (arg), "0" (value)
		);
	return value;
}

/**********************************************************************
		SHIFT
***********************************************************************/
static inline int sh4Rotl(int val) {
	__asm__("rotl	%0\n\t"
		: "=r" (val)
		: "0" (val)
		);
	return val;
}
static inline int sh4Rotr(int val) {
	__asm__("rotr	%0\n\t"
		: "=r" (val)
		: "0" (val)
		);
	return val;
}
/**********************************************************************
		TRAP
***********************************************************************/
//trapnum MUST be a compile-time constant literal
#define sh4Trap(trapnum) \
({ \
	__asm__ __volatile__ (	"trapa #" _sh4_xstr(trapnum) "\n\t" ); \
	})

#define sh4TrapMem(trapnum) \
({ \
	__asm__ __volatile__ (	\
		"trapa #" _sh4_xstr(trapnum) "\n\t" \
	:  \
	:  \
	: "memory"  ); \
	})

#endif
