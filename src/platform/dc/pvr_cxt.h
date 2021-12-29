#ifndef PVR_CXT_GUARD
#define PVR_CXT_GUARD

static inline unsigned int replace_bits(unsigned int src, unsigned int value, unsigned int start, unsigned int width)
{
	//int mask = (~(-1 << start)) << width;
	int mask = 0xffffffff << start;
	mask = mask ^ (mask << width);
	value <<= start;
	
	//GCC generally combines multiple calls better with this, even though
	//it takes one more operation
	return (src & ~mask) | (value & mask);
	//return ((src ^ value) & mask) ^ src;
}

static inline unsigned int replace_bit(unsigned int src, unsigned int value, unsigned int start)
{
	return replace_bits(src, !!value, start, 1);
}

typedef struct {
	uint32  mode2;
	uint32  tex;
} pvr_context_submodes;

// Unified context
typedef struct {
    uint32  cmd;
    uint32  mode1;
    union {
	struct {	//basic intensity
		pvr_context_submodes m;
		float a,r,g,b;
	} bf;
	struct {	//basic packed/sprite/float
		pvr_context_submodes m;
		uint32 color;
		uint32 offset;
		uint32 pad0, pad1;
	} bp;
	pvr_context_submodes mod[2];	//full modifier
    };
} pvr_context;

// Long intensity modifier context
typedef struct {
    pvr_context cxt;
    float   a0, r0, g0, b0;
    float   a1, r1, g1, b1;
} pvr_context_ml;

//parameter types
//Either 0 (false) or 1 (true)
typedef int pc_boolean;

typedef enum {
	PC_CMD_POLYGON = 4,
	PC_CMD_MODIFIER = 4,
	PC_CMD_SPRITE = 5,
} pc_command;

typedef enum {
	PC_NEVER,
	PC_LESS,
	PC_EQUAL,
	PC_LEQUAL,
	PC_GREATER,
	PC_NOTEQUAL,
	PC_GEQUAL,
	PC_ALWAYS,
} pc_depth_compare;

typedef enum {
	PC_STRIP1,
	PC_STRIP2,
	PC_STRIP4,
	PC_STRIP6,
} pc_strip_length;

typedef enum {
	PC_NO_CLIP,
	PC_INSIDE_CLIP,
	PC_OUTSIDE_CLIP,
} pc_clip_mode;

typedef enum {
	PC_PACKED,
	PC_FLOAT,
	PC_INTENSITY,
	PC_CONSTANT
} pc_color_format;

typedef enum {
	PC_OPAQUE_POLY,
	PC_OPAQUE_MOD,
	PC_BLEND_POLY,
	PC_BLEND_MOD,
	PC_PUNCHTHROUGH,
} pc_list_type;

typedef enum {
	PC_CULL_DISABLE,
	PC_CULL_SMALL,
	PC_CULL_CCW,
	PC_CULL_CW,
	PC_CULL_ANTICW = PC_CULL_CCW,
} pc_culling;

typedef enum {
	PC_ZERO,
	PC_ONE,
	PC_OTHER_COLOR,
	PC_INV_OTHER_COLOR,
	PC_SRC_ALPHA,
	PC_INV_SRC_ALPHA,
	PC_DST_ALPHA,
	PC_INV_DST_ALPHA,
} pc_blend_mode;

typedef enum {
	PC_POLYGON,
	PC_ACCUMULATE_SRC,
} pc_color_source;

typedef enum {
	PC_FRAME_BUFFER,
	PC_ACCUMULATE_DST,
} pc_color_destination;

typedef enum {
	PC_UV_NONE,
	PC_UV_X,
	PC_UV_Y,
	PC_UV_XY,
} pc_uv_control;

typedef enum {
	PC_FOG_TABLE,
	PC_FOG_VERTEX,
	PC_FOG_DISABLE,
	PC_FOG_TABLE2,
} pc_fog_mode;

typedef enum {
	PC_POINT,
	PC_BILINEAR,
	PC_TRILINEAR_1ST,
	PC_TRILINEAR_2ND,
} pc_texture_filter;

typedef enum {
	PC_MIPMAP_BIAS_0_25 = 1,
	PC_MIPMAP_BIAS_0_50,
	PC_MIPMAP_BIAS_0_75,
	PC_MIPMAP_BIAS_1_00,
	PC_MIPMAP_BIAS_1_25,
	PC_MIPMAP_BIAS_1_50,
	PC_MIPMAP_BIAS_1_75,
	PC_MIPMAP_BIAS_2_00,
	PC_MIPMAP_BIAS_2_25,
	PC_MIPMAP_BIAS_2_50,
	PC_MIPMAP_BIAS_2_75,
	PC_MIPMAP_BIAS_3_00,
	PC_MIPMAP_BIAS_3_25,
	PC_MIPMAP_BIAS_3_50,
	PC_MIPMAP_BIAS_3_75,
} pc_mipmap_bias;

typedef enum {
	PC_TEXENV_REPLACE,
	PC_TEXENV_MODULATE_REPLACE_ALPHA,
	PC_TEXENV_DECAL,
	PC_TEXENV_MODULATE,
} pc_texture_environment;

typedef enum {
	PC_SIZE_8,
	PC_SIZE_16,
	PC_SIZE_32,
	PC_SIZE_64,
	PC_SIZE_128,
	PC_SIZE_256,
	PC_SIZE_512,
	PC_SIZE_1024,
} pc_texture_size;

static inline int pc_size_to_log2(pc_texture_size size) {
	return ((int)size)+3;
}

static inline int pc_unconvert_size(pc_texture_size size) {
	return 1 << pc_size_to_log2(size);
}

static inline pc_texture_size pc_convert_size(int size) {
	if (size >= 1024)
		return PC_SIZE_1024;
	else if (size >= 512)
		return PC_SIZE_512;
	else if (size >= 256)
		return PC_SIZE_256;
	else if (size >= 128)
		return PC_SIZE_128;
	else if (size >= 64)
		return PC_SIZE_64;
	else if (size >= 32)
		return PC_SIZE_32;
	else if (size >= 16)
		return PC_SIZE_16;
	else
		return PC_SIZE_8;
	
}

typedef enum {
	PC_ARGB1555,
	PC_RGB565,
	PC_ARGB4444,
	PC_YUV,
	PC_NORMAL,
	PC_PALETTE_4B,
	PC_PALETTE_8B,
} pc_texture_format;

typedef enum {
	PC_OUTSIDE,
	PC_INSIDE,
} pc_modifier_selection;

//get no-modifier specific settings
static inline pvr_context_submodes * pc_no_mod(pvr_context *src)
{
	return &src->bf.m;
}

//get sprite specific setting
static inline pvr_context_submodes * pc_spr_mode(pvr_context *src)
{
	return &src->bp.m;
}

//get modifier specific settings by index
static inline pvr_context_submodes * pc_idx_mod(pvr_context *src, pc_modifier_selection mode)
{
	return &src->mod[mode];
}

//get outside (default) modifier specific settings
static inline pvr_context_submodes * pc_out_mod(pvr_context *src)
{
	return pc_idx_mod(src, PC_OUTSIDE);
}

//get inside modifier specific settings
static inline pvr_context_submodes * pc_in_mod(pvr_context *src)
{
	return pc_idx_mod(src, PC_INSIDE);
}

//Sets command type
static inline void pc_set_command(pvr_context *dst, pc_command cmd) {
	dst->cmd = replace_bits(dst->cmd, cmd, 29, 3);
}

//Sets list type of command
static inline void pc_set_list(pvr_context *dst, pc_list_type list) {
	dst->cmd = replace_bits(dst->cmd, list, 24, 3);
}

//Listed in MAME source as "groupen", but MAME doesn't use it in any way
static inline void pc_set_group_enable(pvr_context *dst, pc_boolean enable) {
	dst->cmd = replace_bit(dst->cmd, enable, 23);
}

//Sets maximum strip length generated by TA
static inline void pc_set_max_strip_length(pvr_context *dst, pc_strip_length length) {
	dst->cmd |= 1 << 23;
	dst->cmd = replace_bits(dst->cmd, length, 18, 2);
}

//Sets clipping/scissor mode
static inline void pc_set_clip_mode(pvr_context *dst, pc_clip_mode mode) {
	dst->cmd = replace_bits(dst->cmd, mode, 16, 2);
}

//Set if affected by modifier volumes
static inline void pc_set_modified(pvr_context *dst, pc_boolean enable) {
	dst->cmd = replace_bit(dst->cmd, enable, 7);
}

//Set to enable full modifier volumes, or set to disabled to enable cheap modifier volumes
static inline void pc_set_modifier_full(pvr_context *dst, pc_boolean enable) {
	dst->cmd = replace_bit(dst->cmd, enable, 6);
}

//Sets color format of TA
static inline void pc_set_color_format(pvr_context *dst, pc_color_format format) {
	dst->cmd = replace_bits(dst->cmd, format, 4, 3);
}

//Enables texturing
static inline void pc_set_textured(pvr_context *dst, pc_boolean textured) {
	dst->cmd = replace_bit(dst->cmd, textured, 3);
}

//Enable specular/offset color
static inline void pc_set_specular(pvr_context *dst, pc_boolean specular) {
	dst->cmd = replace_bit(dst->cmd, specular, 2);
}

//Enable gouraud shading
static inline void pc_set_gouraud(pvr_context *dst, pc_boolean gouraud) {
	dst->cmd = replace_bit(dst->cmd, gouraud, 1);
}

//Enable 16-bit UV coords
static inline void pc_set_small_uv(pvr_context *dst, pc_boolean small) {
	dst->cmd = replace_bit(dst->cmd, small, 0);
}

//Sets depth compare mode
static inline void pc_set_depth_compare(pvr_context *dst, pc_depth_compare comparison) {
	dst->mode1 = replace_bits(dst->mode1, comparison, 29, 3);
}

//Sets backface culling
static inline void pc_set_cull_mode(pvr_context *dst, pc_culling cull) {
	dst->mode1 = replace_bits(dst->mode1, cull, 27, 2);
}

//Enables depth writes
static inline void pc_set_depth_write_disable(pvr_context *dst, pc_boolean disable) {
	dst->mode1 = replace_bit(dst->mode1, disable, 26);
}

//Enables improved mipmap level calcuation (slightly sharper textures) at a cost of reduced fillrate
static inline void pc_set_exact_mipmap(pvr_context *dst, pc_boolean dcalc) {
	dst->mode1 = replace_bit(dst->mode1, dcalc, 20);
}



//Sets source blend function
static inline void pc_set_src_blend_mode_mod(pvr_context_submodes *dst, pc_blend_mode bsrc) {
	dst->mode2 = replace_bits(dst->mode2, bsrc, 29, 3);
}
static inline void pc_set_src_blend_mode_in(pvr_context *dst, pc_blend_mode bsrc) {
	pc_set_src_blend_mode_mod(pc_in_mod(dst), bsrc);
}
static inline void pc_set_src_blend_mode(pvr_context *dst, pc_blend_mode bsrc) {
	pc_set_src_blend_mode_mod(pc_no_mod(dst), bsrc);
}

//Sets destination blend function
static inline void pc_set_dst_blend_mode_mod(pvr_context_submodes *dst, pc_blend_mode bdst) {
	dst->mode2 = replace_bits(dst->mode2, bdst, 26, 3);
}
static inline void pc_set_dst_blend_mode_in(pvr_context *dst, pc_blend_mode bdst) {
	pc_set_dst_blend_mode_mod(pc_in_mod(dst), bdst);
}
static inline void pc_set_dst_blend_mode(pvr_context *dst, pc_blend_mode bdst) {
	pc_set_dst_blend_mode_mod(pc_no_mod(dst), bdst);
}

//Sets both source and destination blend functions
static inline void pc_set_blend_modes_mod(pvr_context_submodes *dst, pc_blend_mode bsrc, pc_blend_mode bdst) {
	pc_set_src_blend_mode_mod(dst,bsrc);
	pc_set_dst_blend_mode_mod(dst,bdst);
}
static inline void pc_set_blend_modes_in(pvr_context *dst, pc_blend_mode bsrc, pc_blend_mode bdst) {
	pc_set_src_blend_mode_in(dst,bsrc);
	pc_set_dst_blend_mode_in(dst,bdst);
}
static inline void pc_set_blend_modes(pvr_context *dst, pc_blend_mode bsrc, pc_blend_mode bdst) {
	pc_set_src_blend_mode(dst,bsrc);
	pc_set_dst_blend_mode(dst,bdst);
}

//Sets source color source (regular polygon source color or multitexture accumulation buffer value)
static inline void pc_set_color_source_mod(pvr_context_submodes *dst, pc_color_source csrc) {
	dst->mode2 = replace_bit(dst->mode2, csrc, 25);
}
static inline void pc_set_color_source_in(pvr_context *dst, pc_color_source csrc) {
	pc_set_color_source_mod(pc_in_mod(dst), csrc);
}
static inline void pc_set_color_source(pvr_context *dst, pc_color_source csrc) {
	pc_set_color_source_mod(pc_no_mod(dst), csrc);
}

//Sets pixel destination (frame buffer or multitexture accumulation buffer)
static inline void pc_set_color_destination_mod(pvr_context_submodes *dst, pc_color_destination cdst) {
	dst->mode2 = replace_bit(dst->mode2, cdst, 24);
}
static inline void pc_set_color_destination_in(pvr_context *dst, pc_color_destination cdst) {
	pc_set_color_destination_mod(pc_in_mod(dst), cdst);
}
static inline void pc_set_color_destination(pvr_context *dst, pc_color_destination cdst) {
	pc_set_color_destination_mod(pc_no_mod(dst), cdst);
}

//Sets fog mode
static inline void pc_set_fog_mode_mod(pvr_context_submodes *dst, pc_fog_mode fog) {
	dst->mode2 = replace_bits(dst->mode2, fog, 22, 2);
}
static inline void pc_set_fog_mode_in(pvr_context *dst, pc_fog_mode fog) {
	pc_set_fog_mode_mod(pc_in_mod(dst), fog);
}
static inline void pc_set_fog_mode(pvr_context *dst, pc_fog_mode fog) {
	pc_set_fog_mode_mod(pc_no_mod(dst), fog);
}

//Sets color calcuation clamp enable
static inline void pc_set_color_clamp_mod(pvr_context_submodes *dst, pc_boolean clamp) {
	dst->mode2 = replace_bit(dst->mode2, clamp, 21);
}
static inline void pc_set_color_clamp_in(pvr_context *dst, pc_boolean clamp) {
	pc_set_color_clamp_mod(pc_in_mod(dst), clamp);
}
static inline void pc_set_color_clamp(pvr_context *dst, pc_boolean clamp) {
	pc_set_color_clamp_mod(pc_no_mod(dst), clamp);
}

//Enables vertex alpha value (replaces vertex alpha with 1.0 if disabled)
static inline void pc_set_enable_alpha_mod(pvr_context_submodes *dst, pc_boolean on) {
	dst->mode2 = replace_bit(dst->mode2, on, 20);
}
static inline void pc_set_enable_alpha_in(pvr_context *dst, pc_boolean on) {
	pc_set_enable_alpha_mod(pc_in_mod(dst), on);
}
static inline void pc_set_enable_alpha(pvr_context *dst, pc_boolean on) {
	pc_set_enable_alpha_mod(pc_no_mod(dst), on);
}

//Disables texture alpha (replaces texture alpha with 1.0 when "off" is set to 1)
static inline void pc_set_disable_texture_alpha_mod(pvr_context_submodes *dst, pc_boolean off) {
	dst->mode2 = replace_bit(dst->mode2, off, 19);
}
static inline void pc_set_disable_texture_alpha_in(pvr_context *dst, pc_boolean off) {
	pc_set_disable_texture_alpha_mod(pc_in_mod(dst), off);
}
static inline void pc_set_disable_texture_alpha(pvr_context *dst, pc_boolean off) {
	pc_set_disable_texture_alpha_mod(pc_no_mod(dst), off);
}

//Sets UV flip mode
static inline void pc_set_uv_flip_mod(pvr_context_submodes *dst, pc_uv_control flippage) {
	dst->mode2 = replace_bits(dst->mode2, flippage, 17, 2);
}
static inline void pc_set_uv_flip_in(pvr_context *dst, pc_uv_control flippage) {
	pc_set_uv_flip_mod(pc_in_mod(dst), flippage);
}
static inline void pc_set_uv_flip(pvr_context *dst, pc_uv_control flippage) {
	pc_set_uv_flip_mod(pc_no_mod(dst), flippage);
}

//Sets UV edge clamp
static inline void pc_set_uv_clamp_mod(pvr_context_submodes *dst, pc_uv_control clamppage) {
	dst->mode2 = replace_bits(dst->mode2, clamppage, 15, 2);
}
static inline void pc_set_uv_clamp_in(pvr_context *dst, pc_uv_control clamppage) {
	pc_set_uv_clamp_mod(pc_in_mod(dst), clamppage);
}
static inline void pc_set_uv_clamp(pvr_context *dst, pc_uv_control clamppage) {
	pc_set_uv_clamp_mod(pc_no_mod(dst), clamppage);
}

//Sets texture filter mode
static inline void pc_set_filter_mod(pvr_context_submodes *dst, pc_texture_filter filter) {
	dst->mode2 = replace_bits(dst->mode2, filter, 13, 2);
}
static inline void pc_set_filter_in(pvr_context *dst, pc_texture_filter filter) {
	pc_set_filter_mod(pc_in_mod(dst), filter);
}
static inline void pc_set_filter(pvr_context *dst, pc_texture_filter filter) {
	pc_set_filter_mod(pc_no_mod(dst), filter);
}

//Enable anisotropic filter
static inline void pc_set_anisotropic_mod(pvr_context_submodes *dst, pc_boolean aniso) {
	dst->mode2 = replace_bit(dst->mode2, aniso, 12);
}
static inline void pc_set_anisotropic_in(pvr_context *dst, pc_boolean aniso) {
	pc_set_anisotropic_mod(pc_in_mod(dst), aniso);
}
static inline void pc_set_anisotropic(pvr_context *dst, pc_boolean aniso) {
	pc_set_anisotropic_mod(pc_no_mod(dst), aniso);
}

//Sets mipmap bias, to prefer sharper or blurrier mipmap levels
static inline void pc_set_mipmap_bias_mod(pvr_context_submodes *dst, pc_mipmap_bias bias) {
	dst->mode2 = replace_bits(dst->mode2, bias, 8, 4);
}
static inline void pc_set_mipmap_bias_in(pvr_context *dst, pc_mipmap_bias bias) {
	pc_set_mipmap_bias_mod(pc_in_mod(dst), bias);
}
static inline void pc_set_mipmap_bias(pvr_context *dst, pc_mipmap_bias bias) {
	pc_set_mipmap_bias_mod(pc_no_mod(dst), bias);
}

//Sets opengl texture environment (texenv)
static inline void pc_set_texenv_mod(pvr_context_submodes *dst, pc_texture_environment env) {
	dst->mode2 = replace_bits(dst->mode2, env, 6, 2);
}
static inline void pc_set_texenv_in(pvr_context *dst, pc_texture_environment env) {
	pc_set_texenv_mod(pc_in_mod(dst), env);
}
static inline void pc_set_texenv(pvr_context *dst, pc_texture_environment env) {
	pc_set_texenv_mod(pc_no_mod(dst), env);
}

//Sets texture width
static inline void pc_set_u_size_mod(pvr_context_submodes *dst, pc_texture_size size) {
	dst->mode2 = replace_bits(dst->mode2,size, 3, 3);
}
static inline void pc_set_u_size_in(pvr_context *dst, pc_texture_size size) {
	pc_set_u_size_mod(pc_in_mod(dst), size);
}
static inline void pc_set_u_size(pvr_context *dst, pc_texture_size size) {
	pc_set_u_size_mod(pc_no_mod(dst), size);
}

//Sets texture height
static inline void pc_set_v_size_mod(pvr_context_submodes *dst, pc_texture_size size) {
	dst->mode2 = replace_bits(dst->mode2,size, 0, 3);
}
static inline void pc_set_v_size_in(pvr_context *dst, pc_texture_size size) {
	pc_set_v_size_mod(pc_in_mod(dst), size);
}
static inline void pc_set_v_size(pvr_context *dst, pc_texture_size size) {
	pc_set_v_size_mod(pc_no_mod(dst), size);
}

//Sets texture width and height
static inline void pc_set_uv_size_mod(pvr_context_submodes *dst, pc_texture_size u,pc_texture_size v) {
	pc_set_u_size_mod(dst,u);
	pc_set_v_size_mod(dst,v);
}
static inline void pc_set_uv_size_in(pvr_context *dst, pc_texture_size u,pc_texture_size v) {
	pc_set_uv_size_mod(pc_in_mod(dst), u, v);
}
static inline void pc_set_uv_size(pvr_context *dst, pc_texture_size u,pc_texture_size v) {
	pc_set_uv_size_mod(pc_no_mod(dst), u, v);
}

//Enables mipmapped texture
static inline void pc_set_mipmapped_mod(pvr_context_submodes *dst, pc_boolean mipped) {
	dst->tex = replace_bit(dst->tex, mipped, 31);
}
static inline void pc_set_mipmapped_in(pvr_context *dst, pc_boolean mipped) {
	pc_set_mipmapped_mod(pc_in_mod(dst), mipped);
}
static inline void pc_set_mipmapped(pvr_context *dst, pc_boolean mipped) {
	pc_set_mipmapped_mod(pc_no_mod(dst), mipped);
}

//Enables VQ compression
static inline void pc_set_compressed_mod(pvr_context_submodes *dst, pc_boolean vq) {
	dst->tex = replace_bit(dst->tex, vq, 30);
}
static inline void pc_set_compressed_in(pvr_context *dst, pc_boolean vq) {
	pc_set_compressed_mod(pc_in_mod(dst), vq);
}
static inline void pc_set_compressed(pvr_context *dst, pc_boolean vq) {
	pc_set_compressed_mod(pc_no_mod(dst), vq);
}

//Sets texture format
static inline void pc_set_texture_format_mod(pvr_context_submodes *dst, pc_texture_format format) {
	dst->tex = replace_bits(dst->tex, format, 27, 3);
}
static inline void pc_set_texture_format_in(pvr_context *dst, pc_texture_format format) {
	pc_set_texture_format_mod(pc_in_mod(dst), format);
}
static inline void pc_set_texture_format(pvr_context *dst, pc_texture_format format) {
	pc_set_texture_format_mod(pc_no_mod(dst), format);
}

//Sets twiddled style texture
static inline void pc_set_twiddled_mod(pvr_context_submodes *dst, pc_boolean twiddledee) {
	dst->tex = replace_bit(dst->tex, !twiddledee, 26);
}
static inline void pc_set_twiddled_in(pvr_context *dst, pc_boolean twiddledumb) {
	pc_set_twiddled_mod(pc_in_mod(dst), twiddledumb);
}
static inline void pc_set_twiddled(pvr_context *dst, pc_boolean twiddledumb) {
	pc_set_twiddled_mod(pc_no_mod(dst), twiddledumb);
}

//Sets strided (non-pow-2 width) texture
static inline void pc_set_strided_mod(pvr_context_submodes *dst, pc_boolean strider) {
	dst->tex = replace_bit(dst->tex, strider, 25);
}
static inline void pc_set_strided_in(pvr_context *dst, pc_boolean strider) {
	pc_set_strided_mod(pc_in_mod(dst), strider);
}
static inline void pc_set_strided(pvr_context *dst, pc_boolean strider) {
	pc_set_strided_mod(pc_no_mod(dst), strider);
}

//Sets the palette for a 4-bit texture
static inline void pc_set_palette_4bit_mod(pvr_context_submodes *dst, pc_boolean pal) {
	dst->tex = replace_bits(dst->tex, pal, 21, 6);
}
static inline void pc_set_palette_4bit_in(pvr_context *dst, pc_boolean pal) {
	pc_set_palette_4bit_mod(pc_in_mod(dst), pal);
}
static inline void pc_set_palette_4bit(pvr_context *dst, pc_boolean pal) {
	pc_set_palette_4bit_mod(pc_no_mod(dst), pal);
}

//Sets the palette for an 8-bit texture
static inline void pc_set_palette_8bit_mod(pvr_context_submodes *dst, pc_boolean pal) {
	dst->tex = replace_bits(dst->tex, pal<<4, 21, 2);
}
static inline void pc_set_palette_8bit_in(pvr_context *dst, pc_boolean pal) {
	pc_set_palette_8bit_mod(pc_in_mod(dst),  pal);
}
static inline void pc_set_palette_8bit(pvr_context *dst, pc_boolean pal) {
	pc_set_palette_8bit_mod(pc_no_mod(dst),  pal);
}

//Sets address of texture
static inline void pc_set_texture_address_mod(pvr_context_submodes *dst, pvr_ptr_t tex) {
	dst->tex = replace_bits(dst->tex, ((unsigned int)tex) >> 3, 0, 21);
}
static inline void pc_set_texture_address_in(pvr_context *dst, pvr_ptr_t tex) {
	pc_set_texture_address_mod(pc_in_mod(dst), tex);
}
static inline void pc_set_texture_address(pvr_context *dst, pvr_ptr_t tex) {
	pc_set_texture_address_mod(pc_no_mod(dst), tex);
}

//Returns pvr_ptr_t to texture
static inline pvr_ptr_t pc_get_texture_address_mod(pvr_context_submodes *dst) {
	return (pvr_ptr_t)(PVR_RAM_INT_BASE + ((dst->tex & 0xffffffff) << 3));
}
static inline pvr_ptr_t pc_get_texture_address_in(pvr_context *dst) {
	return pc_get_texture_address_mod(pc_in_mod(dst));
}
static inline pvr_ptr_t pc_get_texture_address(pvr_context *dst) {
	return pc_get_texture_address_mod(pc_no_mod(dst));
}

//Sets everything required for a non-stride non-palettized texture at once for a given mode (does not actually enable texturing)
static inline void pc_set_mode_texture_mod(pvr_context_submodes *dst, pvr_ptr_t tex, pc_texture_size u, pc_texture_size v,
	pc_texture_format format, pc_boolean twiddle, pc_boolean mipmap, pc_boolean vq)
{
	pc_set_strided_mod(dst, 0);
	pc_set_texture_address_mod(dst,tex);
	pc_set_uv_size_mod(dst,u,v);
	pc_set_texture_format_mod(dst,format);
	pc_set_twiddled_mod(dst,twiddle);
	pc_set_mipmapped_mod(dst,mipmap);
	pc_set_compressed_mod(dst,vq);
	pc_set_mipmap_bias_mod(dst, PC_MIPMAP_BIAS_1_00);
}
static inline void pc_set_mode_texture_in(pvr_context *dst, pvr_ptr_t tex, pc_texture_size u, pc_texture_size v,
	pc_texture_format format, pc_boolean twiddle, pc_boolean mipmap, pc_boolean vq)
{
	pc_set_mode_texture_mod(pc_in_mod(dst), tex, u, v, format, twiddle, mipmap, vq);
}
static inline void pc_set_mode_texture(pvr_context *dst, pvr_ptr_t tex, pc_texture_size u, pc_texture_size v,
	pc_texture_format format, pc_boolean twiddle, pc_boolean mipmap, pc_boolean vq)
{
	pc_set_mode_texture_mod(pc_no_mod(dst), tex, u, v, format, twiddle, mipmap, vq);
}

//Sets everything required for a non-stride texture at once for a given mode and enables texturing
static inline void pc_set_texture(pvr_context *dst, pvr_ptr_t tex, pc_texture_size u, pc_texture_size v,
	pc_texture_format format, pc_boolean twiddle, pc_boolean mipmap, pc_boolean vq)
{
	pc_set_textured(dst, 1);
	pc_set_mode_texture(dst, tex, u, v, format, twiddle, mipmap, vq);
}

static inline void pc_set_sprite_color_packed(pvr_context *dst, int color) {
	dst->bp.color = color;
}
static inline void pc_set_sprite_color(pvr_context *dst, float a, float r, float g, float b) {
	dst->bp.color = PVR_PACK_COLOR(a,r,g,b);
}
static inline void pc_set_sprite_offset_packed(pvr_context *dst, unsigned int argb) {
	dst->bp.offset = argb;
}
static inline void pc_set_sprite_offset(pvr_context *dst, float a, float r, float g, float b) {
	dst->bp.offset = PVR_PACK_COLOR(a,r,g,b);
}

static inline void pc_set_intensity_color(pvr_context *dst, float a, float r, float g, float b) {
	dst->bf.a = a;
	dst->bf.r = r;
	dst->bf.g = g;
	dst->bf.b = b;
}

//untextured gouraud shaded opaque polyons with floating point color
static inline void pc_set_default_polygon(pvr_context *dst)
{
	dst->cmd = 0;
	dst->mode1 = 0;
	pc_no_mod(dst)->mode2 = 0;
	pc_no_mod(dst)->tex = 0;
	
	pc_set_command(dst, PC_CMD_POLYGON);
	pc_set_list(dst, PC_OPAQUE_POLY);
	pc_set_gouraud(dst, 1);
	pc_set_max_strip_length(dst, PC_STRIP6);
	pc_set_color_format(dst, PC_FLOAT);
	pc_set_depth_compare(dst, PC_GEQUAL);
	pc_set_cull_mode(dst, PC_CULL_SMALL);
	pc_set_enable_alpha(dst, 1);
	pc_set_blend_modes(dst, PC_ONE, PC_ZERO);
	pc_set_color_clamp(dst, 1);
	pc_set_fog_mode(dst, PC_FOG_DISABLE);
	pc_set_texenv(dst, PC_TEXENV_MODULATE);
	
	dst->bf.a = dst->bf.r = dst->bf.g = dst->bf.b = 1;
}

//transparent untextured sprite
static inline void pc_set_default_sprite(pvr_context *dst) {
	dst->cmd = 0;
	dst->mode1 = 0;
	pc_no_mod(dst)->mode2 = 0;
	pc_no_mod(dst)->tex = 0;
	dst->bp.pad0 = 0;
	dst->bp.pad1 = 0;
	
	pc_set_command(dst, PC_CMD_SPRITE);
	pc_set_list(dst, PC_BLEND_POLY);
	pc_set_gouraud(dst, 1);
	pc_set_max_strip_length(dst, PC_STRIP1);
	pc_set_color_format(dst, PC_PACKED);
	pc_set_depth_compare(dst, PC_GEQUAL);
	pc_set_cull_mode(dst, PC_CULL_SMALL);
	pc_set_enable_alpha(dst, 1);
	pc_set_blend_modes(dst, PC_SRC_ALPHA, PC_INV_SRC_ALPHA);
	pc_set_color_clamp(dst, 1);
	pc_set_fog_mode(dst, PC_FOG_DISABLE);
	pc_set_texenv(dst, PC_TEXENV_MODULATE);
	
	pc_set_sprite_color_packed(dst, 0xffffffff);
	pc_set_sprite_offset_packed(dst, 0);
	
}

static inline void pc_copy(const pvr_context *src, volatile pvr_context *dst) {
	dst->cmd = src->cmd;
	dst->mode1 = src->mode1;
	dst->bp.m.mode2 = src->bp.m.mode2;
	dst->bp.m.tex = src->bp.m.tex;
	dst->bp.color = src->bp.color;
	dst->bp.offset = src->bp.offset;
	dst->bp.pad0 = src->bp.pad0;
	dst->bp.pad1 = src->bp.pad1;
}

int pc_load_texture(const char *fname, pc_texture_format format, int mipmap, pvr_context *dst);

#endif
