#ifndef NPVR_VERTEX
#define NPVR_VERTEX

typedef unsigned int pvr_uv16;
typedef unsigned int pvr_argb;

//Packed color textured vertex
//Vertex types 0, 2, 3, 4, 7, 8, 
typedef struct {
	int cmd;
	float x, y, z;
	union {
		pvr_uv16 uv;
		float u;
	};
	float v;
	union {
		pvr_argb argb;
		float intensity;
	};
	union {
		pvr_argb argbo;
		float intensityo;
	};
} pvr_vertex32;

//Floating color untextured vertex
//Vertex type 1
typedef struct {
	int cmd;
	float x, y, z;
	float a, r, g, b;
} pvr_vertex32f;

//Floating color textured vertex
//Vertex types 5, 6
typedef struct {
	int cmd;
	float x, y, z;
	union {
		pvr_uv16 uv;
		float u;
	};
	float v;
	int pad0, pad1;
	float a, r, g, b;
	float ao, ro, go, bo;
} pvr_vertex64f;

//Modifiable untextured vertex
//Vertex types 9, 10
typedef struct {
	int cmd;
	float x, y, z;
	union {
		pvr_argb argb;
		float intensity;
	};
	union {
		pvr_argb argbm;
		float intensitym;
	};
} pvr_vertex32m;

//Modifiable textured vertex
//Vertex types 11, 12, 13, 14
typedef struct {
	int cmd;
	float x, y, z;
	//Outside modifier
	union {
		pvr_uv16 uv;
		float u;
	};
	float v;
	union {
		pvr_argb argb;
		float intensity;
	};
	union {
		pvr_argb argbo;
		float intensityo;
	};
	//Inside modifier
	union {
		pvr_uv16 uvm;
		float um;
	};
	float vm;
	union {
		pvr_argb argbm;
		float intensitym;
	};
	union {
		pvr_argb argbom;
		float intensityom;
	};
	int pad0, pad1, pad2, pad3;
} pvr_vertex64m;

//Modifier, untextured sprite, textured sprite
//Vertex types 15, 16, 17
typedef struct {
	int cmd;
	float x0, y0, z0;
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3;
	int pad0;
	pvr_uv16 uv0;
	pvr_uv16 uv1;
	pvr_uv16 uv2;
} pvr_vertex64s;

static inline pvr_argb pv_pack_argb(float a, float r, float g, float b) {
	return ( ((unsigned char)(a * 255.0f) ) << 24 ) | 
		( ((unsigned char)(r * 255.0f) ) << 16 ) | 
		( ((unsigned char)(g * 255.0f) ) << 8 ) |
		( ((unsigned char)(b * 255.0f) ) << 0 );
}

static inline unsigned int pv_float_to_bits(float f) {
	union { unsigned int i; float f; } un;
	un.f = f;
	return un.i;
}
//Designed to allow optimizer to generate constants at compile time
static inline pvr_uv16 pv_pack_uv16_static(float u, float v) {
	return (pv_float_to_bits(u) & 0xffff0000) | ((pv_float_to_bits(v) >> 16) & 0xffff);
}

//Designed for fast runtime speed
static inline pvr_uv16 pv_pack_uv16_dynamic(float u, float v) {
	unsigned int ui = pv_float_to_bits(u) >> 16, vi = pv_float_to_bits(v);
	asm( "xtrct	%1,%0\n\t" : "=r" (vi) : "r" ( ui),"0" (vi) );
	return vi;
}

#define PV_CMD_VERTEX		(0xE0000000)
#define PV_CMD_VERTEX_EOS	(0xF0000000)

/*
	setr -> set struct reference
	set -> set pointer to struct
	set_submit -> set pointer to struct and submit first 32 bytes via store queue (prefetch)
	
	cmd -> ta command
	vertex -> ta command set to first/middle vertex of strip
	vertex_eos -> ta command set to last vertex of strip
	
	pos -> vertex position
	
	(no/outside modifier):
	uv -> 32bit floating point tex coords
	uv16 -> 16bit packed tex coords
	argb -> packed diffuse light
	argb_pack -> packed diffuse light from floats
	intensity -> floating point diffuse intensity
	argbf -> floating point diffuse light
	argbo -> packed specular light
	argbo_pack -> packed specular light from floats
	intensityo -> floating point specular intensity
	argbof -> floating point specular light
	
	(inside modifier):
	uvm -> 32bit floating point tex coords
	uv16m -> 16bit packed tex coords
	argbm -> packed diffuse light
	argbm_pack -> packed diffuse light from floats
	intensitym -> floating point diffuse intensity
	argbom -> packed specular light
	argbom_pack -> packed specular light from floats
	intensityom -> floating point specular intensity
*/

//pvr_vertex32, pvr_vertex32m, pvr_vertex32f, pvr_vertex64f, pvr_vertex64m, pvr_vertex64s
#define pv_setr_cmd(v, vcmd) do { v.cmd = (vcmd); } while(0)
#define pv_set_cmd(v, vcmd) do { (v)->cmd = (vcmd); } while(0)
#define pv_set_submit_cmd(v, vcmd) do { (v)->cmd = (vcmd);  sqSubmit32(v); } while(0)
#define pv_set_submit_cmd_next(v, vcmd) do { (v)->cmd = (vcmd); sqSubmit32(v); } while(0)

#define pv_setr_cmd_vertex_eos(v) pv_setr_cmd(v, PV_CMD_VERTEX_EOS)
#define pv_set_cmd_vertex_eos(v) pv_set_cmd(v, PV_CMD_VERTEX_EOS)
#define pv_set_cmd_submit_vertex_eos(v) pv_set_submit_cmd(v, PV_CMD_VERTEX_EOS)

//pvr_vertex32, pvr_vertex32m, pvr_vertex32f, pvr_vertex64f, pvr_vertex64m
#define pv_setr_cmd_vertex(v) pv_setr_cmd(v, PV_CMD_VERTEX)
#define pv_set_cmd_vertex(v) pv_set_cmd(v, PV_CMD_VERTEX)
#define pv_set_cmd_submit_vertex(v) pv_set_submit_cmd(v, PV_CMD_VERTEX)

#define pv_setr_pos(v, xp, yp, zp)  do { v.z = (zp); v.y = (yp); v.x = (xp); } while(0)
#define pv_set_pos(v, xp, yp, zp)  do { (v)->z = (zp); (v)->y = (yp); (v)->x = (xp); } while(0)

//pvr_vertex32, pvr_vertex64f, pvr_vertex64m (outside modifier)
#define pv_setr_uv(vert, tu, tv)  do { vert.v = (tv); vert.u = (tu); } while(0)
#define pv_set_uv(vert, tu, tv)  do { (vert)->v = (tv); (vert)->u = (tu); } while(0)
#define pv_setr_uv16(v, tuv)  do { v.uv = (tuv); } while(0)
#define pv_set_uv16(v, tuv)  do { (v)->uv = (tuv); } while(0)

//pvr_vertex32, pvr_vertex32m (outside modifier), pvr_vertex64m (outside modifier)
#define pv_setr_argb(v, packedcolor)  do { v.argb = (packedcolor); } while(0)
#define pv_set_argb(v, packedcolor)  do { (v)->argb = (packedcolor); } while(0)
#define pv_setr_argb_pack(v, af, rf, gf, bf)  do { pv_setr_argb(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_set_argb_pack(v, af, rf, gf, bf)  do { pv_set_argb(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_setr_intensity(v, intens)  do { v.intensity = (intens); } while(0)
#define pv_set_intensity(v, intens)  do { (v)->intensity = (intens); } while(0)

//pvr_vertex32, pvr_vertex64m (outside modifier)
#define pv_setr_argbo(v, packedcolor)  do { v.argbo = (packedcolor); } while(0)
#define pv_set_argbo(v, packedcolor)  do { (v)->argbo = (packedcolor); } while(0)
#define pv_setr_argbo_pack(v, af, rf, gf, bf)  do { pv_setr_argbo(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_set_argbo_pack(v, af, rf, gf, bf)  do { pv_set_argbo(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_setr_intensityo(v, intens)  do { v.intensityo = (intens); } while(0)
#define pv_set_intensityo(v, intens)  do { (v)->intensityo = (intens); } while(0)

//pvr_vertex32f, pvr_vertex64f
#define pv_setr_argbf(v, af, rf, gf, bf)  do { v.a = (af); v.r = (rf); v.g = (gf); v.b = (bf);  } while(0)
#define pv_set_argbf(v, af, rf, gf, bf)  do { (v)->a = (af); (v)->r = (rf); (v)->g = (gf); (v)->b = (bf);  } while(0)

//pvr_vertex64f
#define pv_setr_argbof(v, af, rf, gf, bf)  do { v.ao = (af); v.ro = (rf); v.go = (gf); v.bo = (bf);  } while(0)
#define pv_set_argbof(v, af, rf, gf, bf)  do { (v)->ao = (af); (v)->ro = (rf); (v)->go = (gf); (v)->bo = (bf);  } while(0)

//pvr_vertex64m (inside modifier)
#define pv_setr_uvm(v, tu, tv)  do { v.um = (tu); v.vm = (tv); } while(0)
#define pv_set_uvm(v, tu, tv)  do { (v)->um = (tu); (v)->vm = (tv); } while(0)
#define pv_setr_uv16m(v, tuv)  do { v.uvm = (tuv); } while(0)
#define pv_set_uv16m(v, tuv)  do { (v)->uvm = (tuv); } while(0)

#define pv_setr_argbom(v, packedcolor)  do { v.argbom = (packedcolor); } while(0)
#define pv_set_argbom(v, packedcolor)  do { (v)->argbom = (packedcolor); } while(0)
#define pv_setr_argbom_pack(v, af, rf, gf, bf)  do { pv_setr_argbom(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_set_argbom_pack(v, af, rf, gf, bf)  do { pv_set_argbom(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_setr_intensityom(v, intens)  do { v.intensityom = (intens); } while(0)
#define pv_set_intensityom(v, intens)  do { (v)->intensityom = (intens); } while(0)

//pvr_vertex32m, pvr_vertex64m (inside modifier)
#define pv_setr_argbm(v, packedcolor)  do { v.argbm = (packedcolor); } while(0)
#define pv_set_argbm(v, packedcolor)  do { (v)->argbm = (packedcolor); } while(0)
#define pv_setr_argbm_pack(v, af, rf, gf, bf)  do { pv_setr_argbm(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_set_argbm_pack(v, af, rf, gf, bf)  do { pv_set_argbm(v, pv_pack_argb(af, rf, gf, bf)); } while(0)
#define pv_setr_intensitym(v, intens)  do { v.intensitym = (intens); } while(0)
#define pv_set_intensitym(v, intens)  do { (v)->intensitym = (intens); } while(0)

//pvr_vertex64s (modifier triangle/sprite)
#define pv_setr_pos0(v, xp, yp, zp)  do { v.x0 = (xp); v.y0 = (yp); v.z0 = (zp); } while(0)
#define pv_set_pos0(v, xp, yp, zp)  do { (v)->x0 = (xp); (v)->y0 = (yp); (v)->z0 = (zp); } while(0)
#define pv_setr_pos1(v, xp, yp, zp)  do { v.x1 = (xp); v.y1 = (yp); v.z1 = (zp); } while(0)
#define pv_set_pos1(v, xp, yp, zp)  do { (v)->x1 = (xp); (v)->y1 = (yp); (v)->z1 = (zp); } while(0)
#define pv_setr_pos2(v, xp, yp, zp)  do { v.x2 = (xp); v.y2 = (yp); v.z2 = (zp); } while(0)
#define pv_set_pos2(v, xp, yp, zp)  do { (v)->x2 = (xp); (v)->y2 = (yp); (v)->z2 = (zp); } while(0)
#define pv_set_submit_pos2(v, xp, yp, zp)  do { (v)->x2 = (xp); sqSubmit64A(v); (v)->y2 = (yp); (v)->z2 = (zp); } while(0)

//pvr_vertex64s (sprite only)
#define pv_setr_pos3(v, xp, yp, zp)  do { v.x3 = (xp); v.y3 = (yp); } while(0)
#define pv_set_pos3(v, xp, yp, zp)  do { (v)->x3 = (xp); (v)->y3 = (yp); } while(0)

/*
	 CW       CCW
	0   1    0   3
	+---+    +---+
	|   |    |   |
	+---+    +---+
	3   2    1   2
*/
#define pv_setr_rect_cw(v, xa, ya, xb, yb, zp) \
	do { \
		pv_setr_pos0(v, xa, ya, zp); \
		pv_setr_pos1(v, xb, ya, zp); \
		pv_setr_pos2(v, xb, yb, zp); \
		pv_setr_pos3(v, xa, yb, zp); \
	} while(0)
#define pv_set_rect_cw(v, xa, ya, xb, yb, zp) \
	do { \
		pv_set_pos0(v, xa, ya, zp); \
		pv_set_pos1(v, xb, ya, zp); \
		pv_set_pos2(v, xb, yb, zp); \
		pv_set_pos3(v, xa, yb, zp); \
	} while(0)
#define pv_set_submit_rect_cw(v, xa, ya, xb, yb, zp) \
	do { \
		pv_set_pos0(v, xa, ya, zp); \
		pv_set_pos1(v, xb, ya, zp); \
		pv_set_submit_pos2(v, xb, yb, zp); \
		pv_set_pos3(v, xa, yb, zp); \
	} while(0)

#define pv_setr_rect_ccw(v, xa, ya, xb, yb, zp) \
	do { \
		pv_setr_pos0(v, xa, ya, zp); \
		pv_setr_pos1(v, xa, yb, zp); \
		pv_setr_pos2(v, xb, yb, zp); \
		pv_setr_pos3(v, xb, ya, zp); \
	} while(0)
#define pv_set_rect_ccw(v, xa, ya, xb, yb, zp) \
	do { \
		pv_set_pos0(v, xa, ya, zp); \
		pv_set_pos1(v, xa, yb, zp); \
		pv_set_pos2(v, xb, yb, zp); \
		pv_set_pos3(v, xb, ya, zp); \
	} while(0)
#define pv_set_submit_rect_ccw(v, xa, ya, xb, yb, zp) \
	do { \
		pv_set_pos0(v, xa, ya, zp); \
		pv_set_pos1(v, xa, yb, zp); \
		pv_set_submit_pos2(v, xb, yb, zp); \
		pv_set_pos3(v, xb, ya, zp); \
	} while(0)

#define pv_setr_rect(v, xa, ya, xb, yb, zp) pv_setr_rect_ccw(v, xa, ya, xb, yb, zp)
#define pv_set_rect(v, xa, ya, xb, yb, zp) pv_set_rect_ccw(v, xa, ya, xb, yb, zp)
#define pv_set_submit_rect(v, xa, ya, xb, yb, zp) pv_set_submit_rect_ccw(v, xa, ya, xb, yb, zp)

#define pv_setr_uv16_0(v, tuv)  do { v.uv0 = (tuv); } while(0)
//#define pv_setr_uv16_0_packd(vert, u, v)  do { pv_setr_uv160(vert, pv_pack_uv16_dynamic(u,v)); } while(0)
//#define pv_setr_uv16_0_packs(vert, u, v)  do { pv_setr_uv160(vert, pv_pack_uv16_static(u,v)); } while(0)
#define pv_set_uv16_0(v, tuv)  do { (v)->uv0 = (tuv); } while(0)
//#define pv_set_uv16_0_packd(vert, u, v)  do { pv_set_uv160(vert, pv_pack_uv16_dynamic(u,v)); } while(0)
//#define pv_set_uv16_0_packs(vert, u, v)  do { pv_set_uv160(vert, pv_pack_uv16_static(u,v)); } while(0)
#define pv_setr_uv16_1(v, tuv)  do { v.uv1 = (tuv); } while(0)
#define pv_set_uv16_1(v, tuv)  do { (v)->uv1 = (tuv); } while(0)
#define pv_setr_uv16_2(v, tuv)  do { v.uv2 = (tuv); } while(0)
#define pv_set_uv16_2(v, tuv)  do { (v)->uv2 = (tuv); } while(0)

#endif
