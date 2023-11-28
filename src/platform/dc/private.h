#ifndef PRIVATE_H_INCLUDED
#define PRIVATE_H_INCLUDED

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <kos.h>
#include "pvr_cxt.h"
#include "npvr_vertex.h"
#include "sh4.h"
#include "sq.h"
#include "xmtrx.h"
#include "libpspvram/valloc.h"

#define PVR_SMALL_CULL          0x0078  // Minimum size of polygons for when culling is not PVR_CULLING_NONE

#define PVR_FGET(REG) (* ( (float*)( 0xa05f8000 + (REG) ) ) )
#define PVR_FSET(REG, VALUE) PVR_FGET(REG) = (VALUE)

void dc_init_hardware();

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

