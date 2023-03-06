#ifndef PRIVATE_H_INCLUDED
#define PRIVATE_H_INCLUDED

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <kos.h>
#include "pvr_cxt.h"
#include "sh4_math.h"
#include "libsh4.h"
#include "libpspvram/valloc.h"

#define FABS(x) MATH_fabs(x)
#define SQRT(x) MATH_Fast_Sqrt(x)
#define INVERT(x) MATH_Fast_Invert(x)

#define PVR_SMALL_CULL          0x0078  // Minimum size of polygons for when culling is not PVR_CULLING_NONE

#define PVR_FGET(REG) (* ( (float*)( 0xa05f8000 + (REG) ) ) )
#define PVR_FSET(REG, VALUE) PVR_FGET(REG) = (VALUE)

void dc_init_hardware();

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

