#ifndef _PRIVATE_H_INCLUDED_
#define _PRIVATE_H_INCLUDED_

#include <stdint.h>
#include <stddef.h>

typedef unsigned int pvr_list_t;
typedef void * pvr_ptr_t;

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

/* Useful memory locations */
#define PVR_TA_INPUT        0x10000000  /**< \brief TA command input */
#define PVR_RAM_BASE        0xa5000000  /**< \brief PVR RAM (raw) */
#define PVR_RAM_INT_BASE    0xa4000000  /**< \brief PVR RAM (interleaved) */

#define PVR_RAM_SIZE        (8*1024*1024)   /**< \brief RAM size in bytes */

#define PVR_RAM_TOP         (PVR_RAM_BASE + PVR_RAM_SIZE)       /**< \brief Top of raw PVR RAM */
#define PVR_RAM_INT_TOP     (PVR_RAM_INT_BASE + PVR_RAM_SIZE)   /**< \brief Top of int PVR RAM */

#define PVR_CLRFMT_ARGBPACKED       0   /**< \brief 32-bit integer ARGB */
#define PVR_CLRFMT_4FLOATS          1   /**< \brief 4 floating point values */
#define PVR_CLRFMT_INTENSITY        2   /**< \brief Intensity color */
#define PVR_CLRFMT_INTENSITY_PREV   3   /**< \brief Use last intensity */

#define PVR_UVFMT_32BIT         0   /**< \brief 32-bit floating point U/V */
#define PVR_UVFMT_16BIT         1   /**< \brief 16-bit floating point U/V */

#define PVR_MODIFIER_DISABLE    0   /**< \brief Disable modifier effects */
#define PVR_MODIFIER_ENABLE     1   /**< \brief Enable modifier effects */

#define PVR_MODIFIER_CHEAP_SHADOW   0
#define PVR_MODIFIER_NORMAL     1

#define PVR_MODIFIER_OTHER_POLY         0   /**< \brief Not the last polygon in the volume */
#define PVR_MODIFIER_INCLUDE_LAST_POLY  1   /**< \brief Last polygon, inclusion volume */
#define PVR_MODIFIER_EXCLUDE_LAST_POLY  2   /**< \brief Last polygon, exclusion volume */


typedef struct {
    int     list_type;          /**< \brief Primitive list
                                     \see   pvr_lists */
    struct {
        int     alpha;          /**< \brief Enable or disable alpha outside modifier
                                     \see   pvr_alpha_switch */
        int     shading;        /**< \brief Shading type
                                     \see   pvr_shading_types */
        int     fog_type;       /**< \brief Fog type outside modifier
                                     \see   pvr_fog_types */
        int     culling;        /**< \brief Culling mode
                                     \see   pvr_cull_modes */
        int     color_clamp;    /**< \brief Color clamp enable/disable outside modifier
                                     \see   pvr_colclamp_switch */
        int     clip_mode;      /**< \brief Clipping mode
                                     \see   pvr_clip_modes */
        int     modifier_mode;  /**< \brief Modifier mode */
        int     specular;       /**< \brief Offset color enable/disable outside modifier
                                     \see   pvr_offset_switch */
        int     alpha2;         /**< \brief Enable/disable alpha inside modifier
                                     \see   pvr_alpha_switch */
        int     fog_type2;      /**< \brief Fog type inside modifier
                                     \see   pvr_fog_types */
        int     color_clamp2;   /**< \brief Color clamp enable/disable inside modifier
                                     \see   pvr_colclamp_switch */
    } gen;                      /**< \brief General parameters */
    struct {
        int     src;            /**< \brief Source blending mode outside modifier
                                     \see   pvr_blend_modes */
        int     dst;            /**< \brief Dest blending mode outside modifier
                                     \see   pvr_blend_modes */
        int     src_enable;     /**< \brief Source blending enable outside modifier
                                     \see   pvr_blend_switch */
        int     dst_enable;     /**< \brief Dest blending enable outside modifier
                                     \see   pvr_blend_switch */
        int     src2;           /**< \brief Source blending mode inside modifier
                                     \see   pvr_blend_modes */
        int     dst2;           /**< \brief Dest blending mode inside modifier
                                     \see   pvr_blend_modes */
        int     src_enable2;    /**< \brief Source blending mode inside modifier
                                     \see   pvr_blend_switch */
        int     dst_enable2;    /**< \brief Dest blending mode inside modifier
                                     \see   pvr_blend_switch */
    } blend;                    /**< \brief Blending parameters */
    struct {
        int     color;          /**< \brief Color format in vertex
                                     \see   pvr_color_fmts */
        int     uv;             /**< \brief U/V data format in vertex
                                     \see   pvr_uv_fmts */
        int     modifier;       /**< \brief Enable or disable modifier effect
                                     \see   pvr_mod_switch */
    } fmt;                      /**< \brief Format control */
    struct {
        int     comparison;     /**< \brief Depth comparison mode
                                     \see pvr_depth_modes */
        int     write;          /**< \brief Enable or disable depth writes
                                     \see pvr_depth_switch */
    } depth;                    /**< \brief Depth comparison/write modes */
    struct {
        int     enable;         /**< \brief Enable/disable texturing
                                     \see   pvr_txr_switch */
        int     filter;         /**< \brief Filtering mode
                                     \see   pvr_filter_modes */
        int     mipmap;         /**< \brief Enable/disable mipmaps
                                     \see   pvr_mip_switch */
        int     mipmap_bias;    /**< \brief Mipmap bias
                                     \see   pvr_mip_bias */
        int     uv_flip;        /**< \brief Enable/disable U/V flipping
                                     \see   pvr_uv_flip */
        int     uv_clamp;       /**< \brief Enable/disable U/V clamping
                                     \see   pvr_uv_clamp */
        int     alpha;          /**< \brief Enable/disable texture alpha
                                     \see   pvr_txralpha_switch */
        int     env;            /**< \brief Texture color contribution
                                     \see   pvr_txrenv_modes */
        int     width;          /**< \brief Texture width (requires a power of 2) */
        int     height;         /**< \brief Texture height (requires a power of 2) */
        int     format;         /**< \brief Texture format
                                     \see   pvr_txr_fmts */
        pvr_ptr_t base;         /**< \brief Texture pointer */
    } txr;                      /**< \brief Texturing params outside modifier */
    struct {
        int     enable;         /**< \brief Enable/disable texturing
                                     \see   pvr_txr_switch */
        int     filter;         /**< \brief Filtering mode
                                     \see   pvr_filter_modes */
        int     mipmap;         /**< \brief Enable/disable mipmaps
                                     \see   pvr_mip_switch */
        int     mipmap_bias;    /**< \brief Mipmap bias
                                     \see   pvr_mip_bias */
        int     uv_flip;        /**< \brief Enable/disable U/V flipping
                                     \see   pvr_uv_flip */
        int     uv_clamp;       /**< \brief Enable/disable U/V clamping
                                     \see   pvr_uv_clamp */
        int     alpha;          /**< \brief Enable/disable texture alpha
                                     \see   pvr_txralpha_switch */
        int     env;            /**< \brief Texture color contribution
                                     \see   pvr_txrenv_modes */
        int     width;          /**< \brief Texture width (requires a power of 2) */
        int     height;         /**< \brief Texture height (requires a power of 2) */
        int     format;         /**< \brief Texture format
                                     \see   pvr_txr_fmts */
        pvr_ptr_t base;         /**< \brief Texture pointer */
    } txr2;                     /**< \brief Texturing params inside modifier */
} pvr_poly_cxt_t;

#define PVR_LIST_OP_POLY        0   /**< \brief Opaque polygon list */
#define PVR_LIST_OP_MOD         1   /**< \brief Opaque modifier list */
#define PVR_LIST_TR_POLY        2   /**< \brief Translucent polygon list */
#define PVR_LIST_TR_MOD         3   /**< \brief Translucent modifier list*/
#define PVR_LIST_PT_POLY        4   /**< \brief Punch-thru polygon list */


typedef struct {
    uint32  cmd;                /**< \brief TA command */
    uint32  mode1;              /**< \brief Parameter word 1 */
    uint32  mode2;              /**< \brief Parameter word 2 */
    uint32  mode3;              /**< \brief Parameter word 3 */
    uint32  d1;                 /**< \brief Dummy value */
    uint32  d2;                 /**< \brief Dummy value */
    uint32  d3;                 /**< \brief Dummy value */
    uint32  d4;                 /**< \brief Dummy value */
} pvr_poly_hdr_t;

typedef struct {
    uint32  flags;              /**< \brief TA command (vertex flags) */
    float   x;                  /**< \brief X coordinate */
    float   y;                  /**< \brief Y coordinate */
    float   z;                  /**< \brief Z coordinate */
    float   u;                  /**< \brief Texture U coordinate */
    float   v;                  /**< \brief Texture V coordinate */
    uint32  argb;               /**< \brief Vertex color */
    uint32  oargb;              /**< \brief Vertex offset color */
} pvr_vertex_t;

typedef struct {
    uint32  cmd;                /**< \brief TA command */
    uint32  mode1;              /**< \brief Parameter word 1 */
    uint32  d1;                 /**< \brief Dummy value */
    uint32  d2;                 /**< \brief Dummy value */
    uint32  d3;                 /**< \brief Dummy value */
    uint32  d4;                 /**< \brief Dummy value */
    uint32  d5;                 /**< \brief Dummy value */
    uint32  d6;                 /**< \brief Dummy value */
} pvr_mod_hdr_t;

#define PVR_SHADE_FLAT          0   /**< \brief Use flat shading */
#define PVR_SHADE_GOURAUD       1   /**< \brief Use Gouraud shading */

#define PVR_DEPTHCMP_NEVER      0   /**< \brief Never pass */
#define PVR_DEPTHCMP_LESS       1   /**< \brief Less than */
#define PVR_DEPTHCMP_EQUAL      2   /**< \brief Equal to */
#define PVR_DEPTHCMP_LEQUAL     3   /**< \brief Less than or equal to */
#define PVR_DEPTHCMP_GREATER    4   /**< \brief Greater than */
#define PVR_DEPTHCMP_NOTEQUAL   5   /**< \brief Not equal to */
#define PVR_DEPTHCMP_GEQUAL     6   /**< \brief Greater than or equal to */
#define PVR_DEPTHCMP_ALWAYS     7   /**< \brief Always pass */

#define PVR_CMD_POLYHDR     0x80840000  /**< \brief PVR polygon header.Striplength set to 2 */
#define PVR_CMD_VERTEX      0xe0000000  /**< \brief PVR vertex data */
#define PVR_CMD_VERTEX_EOL  0xf0000000  /**< \brief PVR vertex, end of strip */
#define PVR_CMD_USERCLIP    0x20000000  /**< \brief PVR user clipping area */
#define PVR_CMD_MODIFIER    0x80000000  /**< \brief PVR modifier volume */
#define PVR_CMD_SPRITE      0xA0000000  /**< \brief PVR sprite header */

#define PVR_CULLING_NONE        0   /**< \brief Disable culling */
#define PVR_CULLING_SMALL       1   /**< \brief Cull if small */
#define PVR_CULLING_CCW         2   /**< \brief Cull if counterclockwise */
#define PVR_CULLING_CW          3   /**< \brief Cull if clockwise */

#define PVR_DEPTHWRITE_ENABLE   0   /**< \brief Update the Z value */
#define PVR_DEPTHWRITE_DISABLE  1   /**< \brief Do not update the Z value */

#define PVR_TEXTURE_DISABLE     0   /**< \brief Disable texturing */
#define PVR_TEXTURE_ENABLE      1   /**< \brief Enable texturing */

#define PVR_BLEND_ZERO          0   /**< \brief None of this color */
#define PVR_BLEND_ONE           1   /**< \brief All of this color */
#define PVR_BLEND_DESTCOLOR     2   /**< \brief Destination color */
#define PVR_BLEND_INVDESTCOLOR  3   /**< \brief Inverse of destination color */
#define PVR_BLEND_SRCALPHA      4   /**< \brief Blend with source alpha */
#define PVR_BLEND_INVSRCALPHA   5   /**< \brief Blend with inverse source alpha */
#define PVR_BLEND_DESTALPHA     6   /**< \brief Blend with destination alpha */
#define PVR_BLEND_INVDESTALPHA  7   /**< \brief Blend with inverse destination alpha */

#define PVR_BLEND_DISABLE       0   /**< \brief Disable blending */
#define PVR_BLEND_ENABLE        1   /**< \brief Enable blending */

#define PVR_ALPHA_DISABLE       0   /**< \brief Disable alpha blending */
#define PVR_ALPHA_ENABLE        1   /**< \brief Enable alpha blending */

#define PVR_FOG_TABLE           0   /**< \brief Table fog */
#define PVR_FOG_VERTEX          1   /**< \brief Vertex fog */
#define PVR_FOG_DISABLE         2   /**< \brief Disable fog */
#define PVR_FOG_TABLE2          3   /**< \brief Table fog mode 2 */

#define PVR_CLRCLAMP_DISABLE    0   /**< \brief Disable color clamping */
#define PVR_CLRCLAMP_ENABLE     1   /**< \brief Enable color clamping */

#define PVR_TXRALPHA_ENABLE     0   /**< \brief Enable alpha blending */
#define PVR_TXRALPHA_DISABLE    1   /**< \brief Disable alpha blending */

#define PVR_UVFLIP_NONE         0   /**< \brief No flipped coordinates */
#define PVR_UVFLIP_V            1   /**< \brief Flip V only */
#define PVR_UVFLIP_U            2   /**< \brief Flip U only */
#define PVR_UVFLIP_UV           3   /**< \brief Flip U and V */

#define PVR_UVCLAMP_NONE        0   /**< \brief Disable clamping */
#define PVR_UVCLAMP_V           1   /**< \brief Clamp V only */
#define PVR_UVCLAMP_U           2   /**< \brief Clamp U only */
#define PVR_UVCLAMP_UV          3   /**< \brief Clamp U and V */

#define PVR_FILTER_NONE         0   /**< \brief No filtering (point sample) */
#define PVR_FILTER_NEAREST      0   /**< \brief No filtering (point sample) */
#define PVR_FILTER_BILINEAR     2   /**< \brief Bilinear interpolation */
#define PVR_FILTER_TRILINEAR1   4   /**< \brief Trilinear interpolation pass 1 */
#define PVR_FILTER_TRILINEAR2   6   /**< \brief Trilinear interpolation pass 2 */

#define PVR_MIPBIAS_NORMAL      PVR_MIPBIAS_1_00    /* txr_mipmap_bias */
#define PVR_MIPBIAS_0_25        1
#define PVR_MIPBIAS_0_50        2
#define PVR_MIPBIAS_0_75        3
#define PVR_MIPBIAS_1_00        4
#define PVR_MIPBIAS_1_25        5
#define PVR_MIPBIAS_1_50        6
#define PVR_MIPBIAS_1_75        7
#define PVR_MIPBIAS_2_00        8
#define PVR_MIPBIAS_2_25        9
#define PVR_MIPBIAS_2_50        10
#define PVR_MIPBIAS_2_75        11
#define PVR_MIPBIAS_3_00        12
#define PVR_MIPBIAS_3_25        13
#define PVR_MIPBIAS_3_50        14
#define PVR_MIPBIAS_3_75        15

#define PVR_TXRENV_REPLACE          0   /**< \brief C = Ct, A = At */
#define PVR_TXRENV_MODULATE         1   /**< \brief  C = Cs * Ct, A = At */
#define PVR_TXRENV_DECAL            2   /**< \brief C = (Cs * At) + (Cs * (1-At)), A = As */
#define PVR_TXRENV_MODULATEALPHA    3   /**< \brief C = Cs * Ct, A = As * At */

#define PVR_MIPMAP_DISABLE      0   /**< \brief Disable mipmap processing */
#define PVR_MIPMAP_ENABLE       1   /**< \brief Enable mipmap processing */

#define PVR_TXRFMT_NONE         0           /**< \brief No texture */
#define PVR_TXRFMT_VQ_DISABLE   (0 << 30)   /**< \brief Not VQ encoded */
#define PVR_TXRFMT_VQ_ENABLE    (1 << 30)   /**< \brief VQ encoded */
#define PVR_TXRFMT_ARGB1555     (0 << 27)   /**< \brief 16-bit ARGB1555 */
#define PVR_TXRFMT_RGB565       (1 << 27)   /**< \brief 16-bit RGB565 */
#define PVR_TXRFMT_ARGB4444     (2 << 27)   /**< \brief 16-bit ARGB4444 */
#define PVR_TXRFMT_YUV422       (3 << 27)   /**< \brief YUV422 format */
#define PVR_TXRFMT_BUMP         (4 << 27)   /**< \brief Bumpmap format */
#define PVR_TXRFMT_PAL4BPP      (5 << 27)   /**< \brief 4BPP paletted format */
#define PVR_TXRFMT_PAL8BPP      (6 << 27)   /**< \brief 8BPP paletted format */
#define PVR_TXRFMT_TWIDDLED     (0 << 26)   /**< \brief Texture is twiddled */
#define PVR_TXRFMT_NONTWIDDLED  (1 << 26)   /**< \brief Texture is not twiddled */
#define PVR_TXRFMT_NOSTRIDE     (0 << 21)   /**< \brief Texture is not strided */
#define PVR_TXRFMT_STRIDE       (1 << 21)   /**< \brief Texture is strided */

/* OR one of these into your texture format if you need it. Note that
   these coincide with the twiddled/stride bits, so you can't have a
   non-twiddled/strided texture that's paletted! */
/** \brief  8BPP palette selector
    \param  x               The palette index */
#define PVR_TXRFMT_8BPP_PAL(x)  ((x) << 25)

/** \brief 4BPP palette selector
    \param  x               The palette index */
#define PVR_TXRFMT_4BPP_PAL(x)  ((x) << 21)


#define PVR_PACK_COLOR(a, r, g, b) ( \
                                     ( ((uint8)( a * 255 ) ) << 24 ) | \
                                     ( ((uint8)( r * 255 ) ) << 16 ) | \
                                     ( ((uint8)( g * 255 ) ) << 8 ) | \
                                     ( ((uint8)( b * 255 ) ) << 0 ) )


#define PVR_GET(REG) (* ( (uint32*)( 0xa05f8000 + (REG) ) ) )
#define PVR_SET(REG, VALUE) PVR_GET(REG) = (VALUE)

#define PVR_FGET(REG) (* ( (float*)( 0xa05f8000 + (REG) ) ) )
#define PVR_FSET(REG, VALUE) PVR_FGET(REG) = (VALUE)

#define PVR_SMALL_CULL          0x0078  // Minimum size of polygons for when culling is not PVR_CULLING_NONE
#define PVR_SCALER_CFG          0x00f4  /**< \brief Smoothing scaler */

/* Palette management ************************************************/

/* In addition to its 16-bit truecolor modes, the PVR also supports some
   nice paletted modes. These aren't useful for super high quality images
   most of the time, but they can be useful for doing some interesting
   special effects, like the old cheap "worm hole". */

/** \defgroup pvr_palfmts           PVR palette formats

    Entries in the PVR's palettes can be of any of these formats. Note that you
    can only have one format active at a time.

    @{
*/
#define PVR_PAL_ARGB1555    0   /**< \brief 16-bit ARGB1555 palette format */
#define PVR_PAL_RGB565      1   /**< \brief 16-bit RGB565 palette format */
#define PVR_PAL_ARGB4444    2   /**< \brief 16-bit ARGB4444 palette format */
#define PVR_PAL_ARGB8888    3   /**< \brief 32-bit ARGB8888 palette format */


/** \defgroup pvr_txrload_constants     Texture loading constants

    These are constants for the flags parameter to pvr_txr_load_ex() or
    pvr_txr_load_kimg().

    @{
*/
#define PVR_TXRLOAD_4BPP            0x01    /**< \brief 4BPP format */
#define PVR_TXRLOAD_8BPP            0x02    /**< \brief 8BPP format */
#define PVR_TXRLOAD_16BPP           0x03    /**< \brief 16BPP format */
#define PVR_TXRLOAD_FMT_MASK        0x0f    /**< \brief Bits used for basic formats */

#define PVR_TXRLOAD_VQ_LOAD         0x10    /**< \brief Do VQ encoding (not supported yet, if ever) */
#define PVR_TXRLOAD_INVERT_Y        0x20    /**< \brief Invert the Y axis while loading */
#define PVR_TXRLOAD_FMT_VQ          0x40    /**< \brief Texture is already VQ encoded */
#define PVR_TXRLOAD_FMT_TWIDDLED    0x80    /**< \brief Texture is already twiddled */
#define PVR_TXRLOAD_FMT_NOTWIDDLE   0x80    /**< \brief Don't twiddle the texture while loading */
#define PVR_TXRLOAD_DMA             0x8000  /**< \brief Use DMA to load the texture */
#define PVR_TXRLOAD_NONBLOCK        0x4000  /**< \brief Use non-blocking loads (only for DMA) */
#define PVR_TXRLOAD_SQ              0x2000  /**< \brief Use store queues to load */

/** PI constant (if you don't want full math.h) */
#define F_PI 3.1415926f

#define __ftan(x) \
    ({ float __value, __arg = (x), __scale = 10430.37835; \
        __asm__("fmul   %2,%1\n\t" \
                "ftrc   %1,fpul\n\t" \
                "fsca   fpul,dr0\n\t" \
                "fdiv   fr1, fr0\n\t" \
                "fmov   fr0,%0" \
                : "=f" (__value), "+&f" (__scale) \
                : "f" (__arg) \
                : "fpul", "fr0", "fr1"); \
        __value; })

#if __STDC_VERSION__ >= 199901L
#define __FMINLINE static inline
#elif __GNUC__
#define __FMINLINE extern inline
#else
/* Uhm... I guess this is the best we can do? */
#define __FMINLINE static
#endif

__FMINLINE float ftan(float r) {
    return __ftan(r);
}

typedef struct vec3f {
    float x, y, z;
} vec3f_t;

#define vec3f_dot(x1, y1, z1, x2, y2, z2, w) { \
        register float __x __asm__("fr0") = (x1); \
        register float __y __asm__("fr1") = (y1); \
        register float __z __asm__("fr2") = (z1); \
        register float __w __asm__("fr3"); \
        register float __a __asm__("fr4") = (x2); \
        register float __b __asm__("fr5") = (y2); \
        register float __c __asm__("fr6") = (z2); \
        register float __d __asm__("fr7"); \
        __asm__ __volatile__( \
                              "fldi0 fr3\n" \
                              "fldi0 fr7\n" \
                              "fipr    fv4,fv0" \
                              : "+f" (__w) \
                              : "f" (__x), "f" (__y), "f" (__z), "f" (__w), \
                              "f" (__a), "f" (__b), "f" (__c), "f" (__d) \
                            ); \
        w = __w; \
    }

#define vec3f_normalize(x, y, z) { \
        register float __x __asm__("fr0") = x; \
        register float __y __asm__("fr1") = y; \
        register float __z __asm__("fr2") = z; \
        __asm__ __volatile__( \
                              "fldi0 fr3\n" \
                              "fipr  fv0,fv0\n" \
                              "fsrra fr3\n" \
                              "fmul  fr3, fr0\n" \
                              "fmul  fr3, fr1\n" \
                              "fmul  fr3, fr2\n" \
                              : "=f" (__x), "=f" (__y), "=f" (__z) \
                              : "0" (__x), "1" (__y), "2" (__z) \
                              : "fr3" ); \
        x = __x; y = __y; z = __z; \
    }

#define mat_trans_normal3_nomod(x, y, z, x2, y2, z2) { \
        register float __x __asm__("fr8") = (x); \
        register float __y __asm__("fr9") = (y); \
        register float __z __asm__("fr10") = (z); \
        __asm__ __volatile__( \
                              "fldi0 fr11\n" \
                              "ftrv  xmtrx, fv8\n" \
                              : "=f" (__x), "=f" (__y), "=f" (__z) \
                              : "0" (__x), "1" (__y), "2" (__z) \
                              : "fr11" ); \
        x2 = __x; y2 = __y; z2 = __z; \
    }


#define mat_trans_single3_nomod(x, y, z, x2, y2, z2) { \
        register float __x __asm__("fr12") = (x); \
        register float __y __asm__("fr13") = (y); \
        register float __z __asm__("fr14") = (z); \
        __asm__ __volatile__( \
                              "fldi1 fr15\n" \
                              "ftrv    xmtrx, fv12\n" \
                              "fldi1 fr14\n" \
                              "fdiv    fr15, fr14\n" \
                              "fmul    fr14, fr12\n" \
                              "fmul    fr14, fr13\n" \
                              : "=f" (__x), "=f" (__y), "=f" (__z) \
                              : "0" (__x), "1" (__y), "2" (__z) \
                              : "fr15" ); \
        x2 = __x; y2 = __y; z2 = __z; \
    }

#define mat_trans_single4(x, y, z, w) { \
        register float __x __asm__("fr0") = (x); \
        register float __y __asm__("fr1") = (y); \
        register float __z __asm__("fr2") = (z); \
        register float __w __asm__("fr3") = (w); \
        __asm__ __volatile__( \
                              "ftrv	xmtrx,fv0\n" \
                              "fdiv	fr3,fr0\n" \
                              "fdiv	fr3,fr1\n" \
                              "fdiv	fr3,fr2\n" \
                              "fldi1	fr4\n" \
                              "fdiv	fr3,fr4\n" \
                              "fmov	fr4,fr3\n" \
                              : "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
                              : "0" (__x), "1" (__y), "2" (__z), "3" (__w) \
                              : "fr4" ); \
        x = __x; y = __y; z = __z; w = __w; \
    }

typedef float matrix_t[4][4];

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

void pvr_set_pal_format(int fmt);
void pvr_set_shadow_scale(int enable, float scale_value);

pvr_ptr_t pvr_mem_malloc(size_t size);
void pvr_mem_free(pvr_ptr_t chunk);

void pvr_poly_cxt_col(pvr_poly_cxt_t *dst, pvr_list_t list);
void pvr_poly_cxt_txr(pvr_poly_cxt_t *dst, pvr_list_t list,
                      int textureformat, int tw, int th, pvr_ptr_t textureaddr,
                      int filtering);
void pvr_poly_compile(pvr_poly_hdr_t *dst, pvr_poly_cxt_t *src);
void pvr_set_bg_color(float r, float g, float b);
int pvr_list_begin(pvr_list_t list);
int pvr_list_finish();

  //#define USE_AA

void dc_init_hardware();

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif

