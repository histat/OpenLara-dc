#include <stdio.h>
#include <string.h>

#include "private.h"

#include "libsh4.h"
#include "primitive/primitive.h"
#include "libpspvram/valloc.h"
#include "pvr_cxt.h"
#include "npvr_vertex.h"
#include <ronin/ronin.h>
#include <ronin/gddrive.h>
#include <ronin/soundcommon.h>
#include "banner_data.h"

extern void LaunchMenu();

#ifdef NOSERIAL
void dcExit()
{
  (*(void(**)(int))0x8c0000e0)(0);
}
#endif

void pvr_set_bg_color(float r, float g, float b) {
    int ir, ig, ib;
    unsigned int color;

    ir = (int)(255 * r);
    ig = (int)(255 * g);
    ib = (int)(255 * b);

    color = (ir << 16) | (ig << 8) | (ib << 0);

    ta_bg_list.color1 = color; 
    ta_bg_list.color2 = color;
    ta_bg_list.color3 = color;

#ifndef NOSERIAL
    printf("[%s] color =0x%x\n", __func__, color);
#endif
}

pvr_ptr_t pvr_mem_malloc(size_t size)
{
  void *ret = psp_valloc(size);

#ifndef NOSERIAL
  printf("[%s] tex = %p size 0x%x\n", __func__, ret, size);
#endif
  return (pvr_ptr_t)ret;
}

void pvr_mem_free(pvr_ptr_t chunk)
{
#ifndef NOSERIAL
  printf("[%s] tex = %p\n", __func__, chunk);
#endif
  
  psp_vfree(chunk);
}

void pvr_poly_compile(pvr_poly_hdr_t *dst, pvr_poly_cxt_t *src)
{
  int u,v;
  pc_boolean vq, twid;
  int format;
  
  pvr_context *cxt = (pvr_context *)dst;
  
  cxt->cmd = 0;
  cxt->mode1 = 0;
  pc_no_mod(cxt)->mode2 = 0;
  pc_no_mod(cxt)->tex = 0;
  
  pc_set_command(cxt, PC_CMD_POLYGON);
  
  /* Or in the list type, shading type, color and UV formats */
  pc_set_list(cxt, src->list_type);
  pc_set_max_strip_length(cxt, PC_STRIP6);
  pc_set_color_format(cxt, src->fmt.color);
  pc_set_small_uv(cxt, src->fmt.uv);
  pc_set_clip_mode(cxt, src->gen.clip_mode);
  pc_set_gouraud(cxt, src->gen.shading);
  pc_set_specular(cxt, src->gen.specular);
	
  /* Polygon mode 1 */
  pc_set_depth_compare(cxt, src->depth.comparison);
  pc_set_cull_mode(cxt, src->gen.culling);
  pc_set_depth_write_disable(cxt, src->depth.write);
  
  /* Polygon mode 2 */
  pc_set_blend_modes(cxt, src->blend.src, src->blend.dst);
  pc_set_color_source(cxt, src->blend.src_enable);
  pc_set_color_destination(cxt, src->blend.dst_enable);
  pc_set_fog_mode(cxt, src->gen.fog_type);
  pc_set_color_clamp(cxt, src->gen.color_clamp);
  pc_set_enable_alpha(cxt, src->gen.alpha);
  pc_set_texenv(cxt, src->txr.env);
  
  cxt->bf.a = cxt->bf.r = cxt->bf.g = cxt->bf.b = 1;
  
  if(src->txr.enable == PVR_TEXTURE_ENABLE)
    {
      u = pc_convert_size(src->txr.width);
      v = pc_convert_size(src->txr.height);
      vq = (src->txr.format>>30)&0x1;
      twid = !((src->txr.format>>26)&0x1);
      format = (src->txr.format>>27)&0x7;

      pc_set_texture(cxt, src->txr.base, u, v, format, twid, src->txr.mipmap, vq);
      
      pc_set_disable_texture_alpha(cxt, src->txr.alpha);
      pc_set_uv_flip(cxt, src->txr.uv_flip);
      pc_set_uv_clamp(cxt, src->txr.uv_clamp);
      int filter = src->txr.filter;
      pc_set_anisotropic(cxt, (filter & 1));
      pc_set_filter(cxt, (filter >> 1));
      pc_set_mipmap_bias(cxt, src->txr.mipmap_bias);
      
      if (format == 5) {
	      pc_set_palette_4bit(cxt, (src->txr.format>>21)&0x3f);
      } else if (format == 6) {
	      pc_set_palette_8bit(cxt, (src->txr.format>>25)&0x03);
      }
    }
}

void pvr_poly_cxt_txr(pvr_poly_cxt_t *dst, pvr_list_t list,
                      int textureformat, int tw, int th, pvr_ptr_t textureaddr,
                      int filtering) {
    int alpha;

    /* Start off blank */
    memset(dst, 0, sizeof(pvr_poly_cxt_t));

    /* Fill in a few values */
    dst->list_type = list;
    alpha = list > PVR_LIST_OP_MOD;
    dst->fmt.color = PVR_CLRFMT_ARGBPACKED;
    dst->fmt.uv = PVR_UVFMT_32BIT;
    dst->gen.shading = PVR_SHADE_GOURAUD;
    dst->depth.comparison = PVR_DEPTHCMP_GREATER;
    dst->depth.write = PVR_DEPTHWRITE_ENABLE;
    dst->gen.culling = PVR_CULLING_CCW;
    dst->txr.enable = PVR_TEXTURE_ENABLE;

    if(!alpha) {
        dst->gen.alpha = PVR_ALPHA_DISABLE;
        dst->txr.alpha = PVR_TXRALPHA_ENABLE;
        dst->blend.src = PVR_BLEND_ONE;
        dst->blend.dst = PVR_BLEND_ZERO;
        dst->txr.env = PVR_TXRENV_MODULATE;
    }
    else {
        dst->gen.alpha = PVR_ALPHA_ENABLE;
        dst->txr.alpha = PVR_TXRALPHA_ENABLE;
        dst->blend.src = PVR_BLEND_SRCALPHA;
        dst->blend.dst = PVR_BLEND_INVSRCALPHA;
        dst->txr.env = PVR_TXRENV_MODULATEALPHA;
    }

    dst->blend.src_enable = PVR_BLEND_DISABLE;
    dst->blend.dst_enable = PVR_BLEND_DISABLE;
    dst->gen.fog_type = PVR_FOG_DISABLE;
    dst->gen.color_clamp = PVR_CLRCLAMP_DISABLE;
    dst->txr.uv_flip = PVR_UVFLIP_NONE;
    dst->txr.uv_clamp = PVR_UVCLAMP_NONE;
    dst->txr.filter = filtering;
    dst->txr.mipmap_bias = PVR_MIPBIAS_NORMAL;
    dst->txr.width = tw;
    dst->txr.height = th;
    dst->txr.base = textureaddr;
    dst->txr.format = textureformat;
}

#define TA_SQ_ADDR (unsigned int *)(void *) \
    (0xe0000000 | (((unsigned long)0x10000000) & 0x03ffffe0))

int pvr_list_begin(pvr_list_t list)
{
  pvr_poly_hdr_t poly =
    {
      (4 << 29)        /* Polygon */
      | (list << 24)  /* List */
      | (1 << 23)  /*  */
      | (1 << 18)  /* Strip Length */
      | (0 << 7)   /* Modifier enable */
      | (0 << 3)   /* Texture */
      | (0 << 2)   /* Offset color */
      | (0 << 1),  /* Gouraud Shading */
      (7 << 29)        /* Z sort */
      | (0 << 27)  /* CCW culling */
      | (0 << 20), /* MIPMAP */
      (32 << 24)       /* Blend Mode */
      | (2 << 22)  /* Fog */
      | (0 << 19)  /* Alpha */
      | (0 << 17)  /* UV flip */
      | (0 << 15)  /* UV clamp */
      | (0 << 13)  /* BiLinear Filter */
      | (0 << 8)   /* MIPMAP scale */
      | (0 << 6)   /* Shading mode */
      | (0 << 3)   /* Texture U size */
      | (0 << 0),  /* Texture V size */
      (0 << 31)        /* MIPMAP */
      | (0 << 30)  /* VQ*/
      | (0 << 27)  /* RGB565 */
      | (0 << 26)  /* Twiddled */
      | (0),
      0, 0, 0, 0};
  
  sqCopy32(TA_SQ_ADDR, &poly);
  return 0;
}

int pvr_list_finish() {

  unsigned int cmd[8] = { 0 };
  
  sqCopy32(TA_SQ_ADDR, &cmd);
  return 0;
}

int gettimeofday (struct timeval *tv, void *tz)
{
  static unsigned long last_tm = 0;
  static unsigned long tmhi = 0;
  unsigned long tmlo = Timer();
  if (tmlo < last_tm)
    tmhi++;

  unsigned long long usecs = 
    ((((unsigned long long)tmlo)<<11)|
     (((unsigned long long)tmhi)<<43))/100;

  tv->tv_usec = usecs % 1000000;
  tv->tv_sec = usecs / 1000000;

  last_tm = tmlo;
  return 0;
}


static unsigned int read_belong(unsigned int *l)
{
  unsigned int v = *l;
  return sh4ByteSwap(v);
}

static void write_belong(unsigned int *l, unsigned int v)
{
  *l = sh4ByteSwap(v);
}

int rumble_check_unit(int unit, struct rumbinfo *info)
{
  unsigned int func;
  unsigned char *res;

  info->port = unit/6;
  info->dev = unit - info->port*6;

  if(info->port<0 || info->port>3 || info->dev<0 || info->dev>5)
    return 0;

  if(info->dev != 1)
    return 0;

  //FIXME: Timer problem if docmd was done somewhere else within 15000s?
  res = maple_docmd(info->port, info->dev, MAPLE_COMMAND_DEVINFO, 0, NULL);

  if(res && res[0] == MAPLE_RESPONSE_DEVINFO && res[3]>=28 &&
     ((func=read_belong((unsigned int *)(res+4)))&MAPLE_FUNC_PURUPURU)) {
    info->func = func;
  }

  return 0;
}

int rumble_set(struct rumbinfo *info, int on)
{
  unsigned int param[2];
  unsigned char *res;
  int retr;

  if(!(info->func & MAPLE_FUNC_PURUPURU))
    return 0;

  for(retr = 0; retr < 5; retr++) {

    write_belong(&param[0], MAPLE_FUNC_PURUPURU);
    write_belong(&param[1], (on? 0x011a7010 : 0));

    if((res = maple_docmd(info->port, info->dev, MAPLE_COMMAND_SETCOND, 2,
                          param))
       && res[0] == MAPLE_RESPONSE_OK)
      return 1;
  }

  return 0;
}

void g2_memcpy4s(void *s1, const void *s2, unsigned int n)
{
  uint32 *p1a = s1;
  uint32 *p1b = (void*)(((char *)s1)+SAMPLES_TO_BYTES(STEREO_OFFSET));
  const uint32 *p2 = s2;
  n+=3;
  n>>=2;
  while(n--) {
    uint32 a = *p2++;
    uint32 b = *p2++;
    *p1a++ = (a & 0xffff) | ((b & 0xffff)<<16);
    *p1b++ = ((a & 0xffff0000)>>16) | (b & 0xffff0000);

    if(!(n & 3)) {
      uint32 i = 0x1800;
      // aica FIFO
      for (; i >=0; --i)
          if(!(*(volatile uint32*)0xa05f688c & 0x11)) break;
    }

  }
}



#define TA_LIST_ENABLE_DEFAULT (TA_LIST_OPAQUEPOLY|TA_LIST_TRANSPOLY|TA_LIST_PUNCHTHROUGH)
//#define TA_LIST_ENABLE_DEFAULT (TA_LIST_OPAQUEPOLY|TA_LIST_OPAQUEMOD|TA_LIST_TRANSPOLY)
//#define TA_LIST_ENABLE_DEFAULT (TA_LIST_OPAQUEPOLY|TA_LIST_TRANSPOLY)
//#define TA_LIST_ENABLE_DEFAULT (TA_LIST_OPAQUEPOLY|TA_LIST_OPAQUEMOD|TA_LIST_TRANSPOLY|TA_LIST_TRANSMOD)

static void dc_reset_target()
{
  int i;
  union {
    float f;
    unsigned int i;
  } zclip;
  
  for(i=0; i<2; i++) {
    struct ta_buffers *b = &ta_buffers[i];
#ifdef USE_AA
    b->ta_tilew = (640*2)/32;
#endif
    b->ta_lists = TA_LIST_ENABLE_DEFAULT;
  }
  
  ta_init_renderstate();

}

/* Primitive buffer */
unsigned int prim_buffer[256 * 1024 * 4] __attribute__((aligned(32)));


void dc_init_hardware()
{
#ifndef NOSERIAL
  serial_init(57600);
  usleep(20000);
  printf("Serial OK\n");
#endif
  
  cdfs_init();
  maple_init();
  dc_setup_ta();
  init_arm();

  sh4SetFPSCR(0x00040001);
  
#ifdef USE_AA
  if (fb_devconfig.dc_wid == 640) {
    *(volatile unsigned int *)(0xa05f80f4) = 0x10400;
  }
  else {
    *(volatile unsigned int *)(0xa05f80f4) = 0x10401;
  }
#endif

  dc_reset_target();

  pvr_set_bg_color(0.0, 0.0, 0.0);

  PVR_FSET(PVR_SMALL_CULL, 0.1f);

  PVR_FSET(0x11C, 0.5f);
  //PVR_FSET(0x108, PVR_PAL_ARGB4444);

  /* Init primitive buffer */
  primitive_buffer_init(0, 0, -1);
  //primitive_buffer_init(1, &prim_buffer[256 * 1024 * 0], 256 * 1024);
  primitive_buffer_init(2, &prim_buffer[256 * 1024 * 0], 256 * 1024 * 1);
  //primitive_buffer_init(2, &prim_buffer[256 * 1024 * 0], 256 * 1024 * 4);
  //primitive_buffer_init(3, &prim_buffer[256 * 1024 * 2], 256 * 1024);
  primitive_buffer_init(4, &prim_buffer[256 * 1024 * 2], 256 * 1024 * 3);


  LaunchMenu();

}

extern void *get_romfont_address();
__asm__("\
			\n\
.globl _romfont_address \n\
_get_romfont_address:	\n\
    mov.l 1f,r0		\n\
    mov.l @r0,r0	\n\
    jmp @r0		\n\
    mov #0,r1		\n\
    .align 2		\n\
1:  .long 0x8c0000b4	\n\
			\n\
");

void bfont_draw(void *buffer, int bufwidth, int opaque, int c)
{
  int i,j;
  uint8 *fa = (uint8 *)get_romfont_address();
  
  /* By default, map to a space */
  uint32 index = 72 << 2;
  
  /* 33-126 in ASCII are 1-94 in the font */
  if(c >= 33 && c <= 126)
    index = c - 32;
  
  /* 160-255 in ASCII are 96-161 in the font */
  else if(c >= 160 && c <= 255)
    index = c - (160 - 96);
  
  unsigned char *s = fa + index * (12*24/8);
  
  const int yalign = bufwidth;
  uint16 *dst = (uint16*)buffer;
  int bits;
  for (i=0; i<12; i++) {
    bits = *s++ <<16;
    bits |= *s++ << 8;
    bits |= *s++ << 0;
    for (j=0; j<12; j++, bits<<=1)
      if (bits & 0x800000) {
	dst[j] = 0xffff;
	dst[j+1] = 0xffff;
	dst[j+2] = 0xa108;
	dst[j+yalign] = 0xa108;
	dst[j+yalign+1] = 0xa108;
      }
    dst += yalign;
    for (j=0; j<12; j++, bits<<=1)
      if (bits & 0x800000) {
	dst[j] = 0xffff;
	dst[j+1] = 0xffff;
	dst[j+2] = 0xa108;
	dst[j+yalign] = 0xa108;
	dst[j+yalign+1] = 0xa108;
      }
    
    dst += yalign;
  }
}

pvr_ptr_t setup_font_texture() {
  int x, y;
  pvr_ptr_t buffer;

  buffer = pvr_mem_malloc(256*256*2);
  uint16 *vram = (uint16 *)buffer;

  vram = (uint16 *)buffer;
  for (y=0; y<8; y++) {
    for (x=0; x<16; x++) {
	bfont_draw(vram, 256, 0, y*16+x);
	vram += 16;
    }
    vram += 24*256;
  }
  
  return buffer;
}

void draw_poly_char(void *sq, float x1, float y1, float a, float r, float g, float b, int c) {
  pvr_vertex_t	vert;
  int ix = (c % 16) * 16;
  int iy = (c / 16) * 25;
  float u1 = ix * 1.0f / 256.0f;
  float v1 = iy * 1.0f / 256.0f;
  float u2 = (ix+14) * 1.0f / 256.0f;
  float v2 = (iy+25) * 1.0f / 256.0f;
  
  pvr_vertex32 *v = (pvr_vertex32*)sq;
  
  pv_set_pos(v, x1, y1 + 25.0f, 1.0);
  pv_set_uv(v, u1, v2);
  pv_set_argb_pack(v, a, r, g, b);
  pv_set_cmd_submit_vertex(v);
  v++;
  
  pv_set_pos(v, x1, y1, 1.0);
  pv_set_uv(v, u1, v1);
  pv_set_argb_pack(v, a, r, g, b);
  pv_set_cmd_submit_vertex(v);
  v++;
	
  pv_set_pos(v, x1 + 14.0f, y1 + 25.0f, 1.0);
  pv_set_uv(v, u2, v2);
  pv_set_argb_pack(v, a, r, g, b);
  pv_set_cmd_submit_vertex(v);
  v++;
	
  pv_set_pos(v, x1 + 14.0f, y1, 1.0);
  pv_set_uv(v, u2, v1);
  pv_set_argb_pack(v, a, r, g, b);
  pv_set_cmd_submit_vertex_eos(v);
  v++;
}

void draw_poly_strf(pvr_poly_hdr_t *hdr, float x1, float y1, float a, float r, float g, float b, char *str) {
  char *s;
  
  if (x1 == 0.0f) {
    int l = 0;
    int space = 0;
    int i;
    for (i=0; i<strlen(str); i++) {
      if(str[i] != ' ')
	l++;
      else
	space++;
    }

    float len = l*14.0f + space*10.0f;
    x1 = (640.0f - len) / 2;
  }
  
  void *sq = sqPrepare((void*)PVR_TA_INPUT);
  sqCopy32(sq, hdr);
  
  s = str;
  while (*s) {
    if (*s == ' ') {
      x1 += 10.0f; s++;
    } else
      draw_poly_char(sq, x1+=14.0f, y1, a, r, g, b, *s++);
  }
}

typedef struct
{
    char id[4]; // 'DTEX'
    short width;
    short height;
    int type;
    int size;
} tex_header_t;

#if 0
pvr_ptr_t tex_load_cd(char *filename, int *tex_w, int *tex_h, int *type)
{
    int file = 0;
    pvr_ptr_t buffer;
    tex_header_t header;

    file = open(filename, 0);
    if (file < 0)
        return 0;

    /* load header */
    read(file, &header, 16);
    *tex_w = (int)header.width;
    *tex_h = (int)header.height;
    *type = header.type;

    buffer = pvr_mem_malloc(header.size);
    read(file, buffer, header.size);
    close(file);

    return buffer;
}
#endif

pvr_ptr_t tex_load_ram(unsigned char *in, int *tex_w, int *tex_h, int *type)
{
    int file = 0;
    pvr_ptr_t buffer;
    tex_header_t header;

    /* load header */
    memcpy(&header, in, 16);
    *tex_w = (int)header.width;
    *tex_h = (int)header.height;
    *type = header.type;

    buffer = pvr_mem_malloc(header.size);
    memcpy(buffer, in + 16, header.size);

    return buffer;
}

int getCdState()
{
  unsigned int param[4];
  gdGdcGetDrvStat(param);
  return param[0];
}

static void draw_banner(pvr_poly_hdr_t *hdr, int w, int h)
{
    void *sq = sqPrepare((void*)PVR_TA_INPUT);
    sqCopy32(sq, hdr);

    float x1,y1;

    x1 = (640 - w) / 2;
    y1 = (480 - h) / 2;
    pvr_vertex32 *v = (pvr_vertex32*)sq + 32;
    
    pv_set_pos(v, x1, y1, 1.0);
    pv_set_uv(v, 0.0, 0.0);
    pv_set_cmd_submit_vertex(v);
    v++;
		
    pv_set_pos(v, x1, y1 + h, 1.0);
    pv_set_uv(v, 0.0, 1.0);
    pv_set_cmd_submit_vertex(v);
    v++;
		
    pv_set_pos(v, x1 + w, y1, 1.0);
    pv_set_uv(v, 1.0, 0.0);
    pv_set_cmd_submit_vertex(v);
    v++;

    pv_set_pos(v, x1 + w, y1 + h, 1.0);
    pv_set_uv(v, 1.0, 1.0);
    pv_set_cmd_submit_vertex_eos(v);
    v++;
}

static int poll_input()
{
  int i;
  int mask = getimask();
  setimask(15);
  
  int res = 0;
  int b;
  struct mapledev *pad = locked_get_pads();
  for (i = 0; i < 4; i++, pad++) {
    b = 0;
    if (pad->func & MAPLE_FUNC_CONTROLLER) {
      b = ~pad->cond.controller.buttons & 0x0fff;
    }
    res |= b;
  }
  setimask(mask);
  
  return res;

}

static int check_CD(const char *path) {
	int fd;
	DIR *dp;

	fd = open(path, O_RDONLY);
	if (fd >= 0) {
		close(fd);
		return 1;
	}
	dp = opendir(path);
	if (dp) {
		closedir(dp);
		return 2;
	}
	return -1;
}

void LaunchMenu() {
  int i, frame;
  int w, h, type;
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr0,hdr1;
  pvr_ptr_t banner_tex;
  pvr_ptr_t font_tex;

  banner_tex = tex_load_ram(banner_data, &w, &h, &type);

  pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, type /*PVR_TXRFMT_RGB565|PVR_TXRFMT_VQ_ENABLE|PVR_TXRFMT_TWIDDLED*/, w, h, banner_tex, PVR_FILTER_BILINEAR);
  cxt.gen.culling = PVR_CULLING_NONE;
  cxt.txr.env = PVR_TXRENV_REPLACE;
  pvr_poly_compile(&hdr0, &cxt);

  font_tex = setup_font_texture();
 
  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_NONTWIDDLED, 256, 256, font_tex, PVR_FILTER_NONE);
  cxt.gen.culling = PVR_CULLING_NONE;
  cxt.txr.env = PVR_TXRENV_MODULATE;
  pvr_poly_compile(&hdr1, &cxt);
  //*((volatile unsigned int *)(void *)0xa05f8040) = 0xFFFFFF;

  int checked = 0;
  const char *path[] = {
    "level/1/",
    "data/",
    NULL
  };

  for (i = 0; path[i] != NULL; i++) {
    if(check_CD(path[i]) >= 0) {
      checked = 1;
      break;
    }
  }

  frame = 32;

  int lid = 0;
  while ( !checked ) {
    usleep(20000);
    if(frame > 255)
      frame = 0;

    int res = poll_input();

    ta_begin_frame();
    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_banner(&hdr0, w, h);
    pvr_list_finish();
    pvr_list_begin(PVR_LIST_TR_POLY);
    if (frame & 32) {
      draw_poly_strf(&hdr1, 0, 400, 1.0, 1.0, 1.0, 1.0, "INSERT GAME CD");
    }
    pvr_list_finish();
    pvr_list_begin(PVR_LIST_PT_POLY);
    pvr_list_finish();
    ta_end_dlist();
    frame++;

    if (res == 0x0e)
      break;
     
    int s = getCdState();
    if (s >= 6) {
      lid = 1;
    }
    if (s > 0 && s < 6 && lid) {
      cdfs_reinit();
      chdir("/"); // failed
      chdir("/");

      break;
    }
  }

  ta_begin_frame();
  pvr_list_begin(PVR_LIST_OP_POLY);
  draw_banner(&hdr0, w, h);
  pvr_list_finish();
  pvr_list_begin(PVR_LIST_TR_POLY);
  draw_poly_strf(&hdr1, 0, 400, 1.0, 1.0, 1.0, 1.0, "LOADING...");
  pvr_list_finish();
  pvr_list_begin(PVR_LIST_PT_POLY);
  pvr_list_finish();
  ta_end_dlist();

  //usleep(1000);

  ta_sync();
  pvr_mem_free(banner_tex);
  pvr_mem_free(font_tex);
  return;
}

