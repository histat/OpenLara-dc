#ifndef _PRIM_H_INCLUDED_
#define _PRIM_H_INCLUDED_

#ifdef BUILD_LIB
#include <kos.h>
#else
#include "private.h"
#endif

typedef union {
    unsigned int color;
    unsigned char argb[4];
    float intensity;
} packed_color_t;

typedef struct {
    float x, y, z;
} vertex_3f_t;

typedef struct {
    unsigned int cmd;
    unsigned int mode1;
    unsigned int mode2;
    unsigned int mode3;
    unsigned int color[4];
} polygon_header_t;

typedef struct {
    unsigned int cmd;
    unsigned int mode1;
    unsigned int dummy[6];
} modifier_header_t;

typedef struct {
    unsigned int cmd;
    unsigned int mode1;
    unsigned int mode2;
    unsigned int mode3;
    packed_color_t base;
    packed_color_t offset;
    unsigned int dummy[2];
} sprite_header_t;

typedef struct {
    unsigned int flags;
    float x, y ,z;
    float u, v;
    packed_color_t base;
    packed_color_t offset;
} polygon_vertex_t;

typedef struct {
    unsigned int flags;
    vertex_3f_t a, b, c;
    unsigned int dummy[6];
} modifier_vertex_t;

typedef struct {
    unsigned int flags;
    float a[3], b[3], c[3], d[2];
    unsigned int dummy;
    unsigned int auv, buv, cuv;
} sprite_vertex_t;

/* Near Z clip value. */
/*#define NEAR_Z  0.0001f*/
#define NEAR_Z  0.001f

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
extern int primitive_buffer_init(int type, void *buffer, int size);
extern void primitive_buffer_begin(void);
extern void primitive_buffer_flush(void);

extern int primitive_header(void *header, int size);

extern int primitive_nclip_polygon(pvr_vertex_t *vertex_list, int *index_list, int index_size);
extern int primitive_nclip_polygon_strip(pvr_vertex_t *vertex_list, int *index_list, int index_size);
extern int primitive_nclip_modifier(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size);
extern int primitive_nclip_modifier_strip(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size);

extern int primitive_polygon(pvr_vertex_t *vertex_list, int *index_list, int index_size);
extern int primitive_polygon_strip(pvr_vertex_t *vertex_list, int *index_list, int index_size);
extern int primitive_modifier(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size);
extern int primitive_modifier_strip(pvr_mod_hdr_t *eol_header, float *vertex_list, int *index_list, int index_size);

extern int prim_commit_vert_ready(int size);
extern void prim_commit_poly_vert(polygon_vertex_t *p, int eos);
extern void prim_commit_poly_inter(polygon_vertex_t *p, polygon_vertex_t *q, int eos);
extern void prim_commit_modi_vert(modifier_header_t *eol_header, vertex_3f_t *a, vertex_3f_t *b, vertex_3f_t *c);
extern void prim_commit_spri_vert(sprite_vertex_t *p);
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
