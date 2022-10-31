#include <stdio.h>
#include <string.h>

#include <sys/dirent.h>

#include <kos.h>
#include "private.h"

#include "libsh4.h"
#include "primitive/primitive.h"
#include "libpspvram/valloc.h"
#include "xmtrx.h"
#include "watchdog.h"
#include "pvr_cxt.h"
#include "npvr_vertex.h"
#include <../hardware/pvr/pvr_internal.h>
#include "banner_data.h"


extern void _audio_init();
extern void _audio_free();

static pvr_ptr_t pvr_mem_base;

extern void LaunchMenu();



void *kospvrvramGetAddr()
{
  return (void *)pvr_mem_base;
}

pvr_ptr_t kos_mem_malloc(size_t size)
{
  void *ret = psp_valloc(size);

#ifndef NOSERIAL
  printf("[%s] tex = %p size 0x%x\n", __func__, ret, size);
#endif

  return (pvr_ptr_t)(ret);
}

void kos_mem_free(pvr_ptr_t chunk)
{
#ifndef NOSERIAL
  printf("[%s] tex = %p\n", __func__, chunk);
#endif
  
  psp_vfree(chunk);
}

void kos_poly_compile(pvr_poly_hdr_t *dst, pvr_poly_cxt_t *src)
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

int  arch_auto_init() {
    /* Initialize memory management */
    mm_init();

    /* Do this immediately so we can receive exceptions for init code
       and use ints for dbgio receive. */
    irq_init();         /* IRQs */
    irq_disable();          /* Turn on exceptions */

    if(!(__kos_init_flags & INIT_NO_DCLOAD))
        fs_dcload_init_console();   /* Init dc-load console, if applicable */

    /* Init SCIF for debug stuff (maybe) */
    scif_init();

    /* Init debug IO */
    dbgio_init();

    /* Print a banner */
    #ifdef NOSERIAL
    dbgio_disable();
    #endif

    timer_init();           /* Timers */
    hardware_sys_init();        /* DC low-level hardware init */

    /* Initialize our timer */
    timer_ms_enable();
    rtc_init();

    /* Threads */
    if(__kos_init_flags & INIT_THD_PREEMPT)
        thd_init(THD_MODE_PREEMPT);
    else
        thd_init(THD_MODE_COOP);

    nmmgr_init();

    fs_init();          /* VFS */
    //fs_ramdisk_init();      /* Ramdisk */
    //fs_romdisk_init();      /* Romdisk */

    hardware_periph_init();     /* DC peripheral init */

    if(!(__kos_init_flags & INIT_NO_DCLOAD) && *DCLOADMAGICADDR == DCLOADMAGICVALUE) {
        dbglog(DBG_INFO, "dc-load console support enabled\n");
        fs_dcload_init();
    }

    fs_iso9660_init();

    vmufs_init();
    //fs_vmu_init();

    /* Now comes the optional stuff */
    if(__kos_init_flags & INIT_IRQ) {
        irq_enable();       /* Turn on IRQs */
        maple_wait_scan();  /* Wait for the maple scan to complete */
    }


    return 0;
}

void  arch_auto_shutdown() {
    fs_dclsocket_shutdown();

    irq_disable();
    timer_shutdown();
    hardware_shutdown();
    pvr_shutdown();
    //library_shutdown();
    fs_dcload_shutdown();
    //fs_vmu_shutdown();
    vmufs_shutdown();
    fs_iso9660_shutdown();
    //fs_ramdisk_shutdown();
    //fs_romdisk_shutdown();
    fs_shutdown();
    thd_shutdown();
    rtc_shutdown();
}

/* Primitive buffer */
unsigned int prim_buffer[256 * 1024 * 4] __attribute__((aligned(32)));

xMatrix mtrx_stack[4];

void dc_init_hardware()
{
  int n;

  pvr_init_params_t pvr_params = {
    { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16 },
    (int)(512 * 1024),
    0,	//dma
    0,	//fsaa
    0	//autosort disable
  };

  wdInit();

  xmtrxSetStackPointer(mtrx_stack, sizeof(mtrx_stack));

  _audio_init();

  pvr_mem_base = NULL;


  pvr_init (&pvr_params);
  
  pvr_mem_base = (pvr_ptr_t)(PVR_RAM_INT_BASE + pvr_state.texture_base);
  //pvr_mem_base = 0xa4000000;
#ifndef NOSERIAL
  printf("reset pvr_mem_base at 0x%4x ", pvr_mem_base);
#endif
  
  pvr_set_zclip(0.0f);
  
  pvr_set_bg_color(0.0, 0.0, 0.0);

  PVR_FSET(PVR_SMALL_CULL, 0.1f);

  PVR_FSET(0x11C, 0.5f);
  PVR_SET(0x0e4, (640 >> 5));

  for (n = 0; n < 256; n++) {
    pvr_set_pal_entry(n, (n*0x111)|0xff000000);
  }

 /* Init primitive buffer */
  primitive_buffer_init(0, 0, -1);
  primitive_buffer_init(2, &prim_buffer[256 * 1024 * 0], 256 * 1024 * 2);
  primitive_buffer_init(4, &prim_buffer[256 * 1024 * 1], 256 * 1024 * 2);

  wdPause();
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

void _bfont_draw(void *buffer, int bufwidth, int opaque, int c)
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

  buffer = kos_mem_malloc(256*256*2);
  uint16 *vram = (uint16 *)buffer;

  vram = (uint16 *)buffer;
  for (y=0; y<8; y++) {
    for (x=0; x<16; x++) {
	_bfont_draw(vram, 256, 0, y*16+x);
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

pvr_ptr_t tex_load_cd(char *filename, int *tex_w, int *tex_h, int *type)
{
    int file = 0;
    pvr_ptr_t buffer;
    tex_header_t header;

    file = open(filename, O_RDONLY);
    if (file < 0)
        return 0;

    /* load header */
    read(file, &header, 16);
    *tex_w = (int)header.width;
    *tex_h = (int)header.height;
    *type = header.type;

    buffer = kos_mem_malloc(header.size);
    read(file, buffer, header.size);
    close(file);

    return buffer;
}

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

    buffer = kos_mem_malloc(header.size);
    memcpy(buffer, in + 16, header.size);

    return buffer;
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
  int res = 0;
  int b;

  MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)

  b = st->buttons & 0x0fff;
  res |= b;
  
  MAPLE_FOREACH_END()
  
  return res;

}

static int check_CD(const char *path) {
  int fd;
  DIR *dirp;

  dirp = opendir(path);
  if (dirp == NULL) {
    return -1;
  }

  closedir(dirp);

  return 1;
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
  kos_poly_compile(&hdr0, &cxt);

  font_tex = setup_font_texture();

  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_NONTWIDDLED, 256, 256, font_tex, PVR_FILTER_NONE);
  cxt.gen.culling = PVR_CULLING_NONE;
  cxt.txr.env = PVR_TXRENV_MODULATE;
  kos_poly_compile(&hdr1, &cxt);
  //*((volatile unsigned int *)(void *)0xa05f8040) = 0xFFFFFF;
  
  int checked = 0;
  const char *path[] = {
    "/cd/level/1/",
    "/cd/data/",
    NULL
  };

  /*
  for (i = 0; path[i] != NULL; i++) {
    if(check_CD(path[i]) > 0) {
      checked = 1;
      break;
    }
  }
  */

  checked = 0;

  frame = 32;

  int lid = 0;
  while ( !checked ) {

    if(frame > 255)
      frame = 0;

    int res = poll_input();

    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_banner(&hdr0, w, h);
    pvr_list_finish();
    pvr_list_begin(PVR_LIST_TR_POLY);
    if (frame & 32) {
      draw_poly_strf(&hdr1, 0, 400, 1.0, 1.0, 1.0, 1.0, "INSERT GAME CD");
    }
    pvr_list_finish();
    pvr_scene_finish();

    frame++;

    if ((res & 0x60e) == 0x60e)
      break;

    timer_spin_sleep(17);
    for (i = 0; path[i] != NULL; i++) {
      if(check_CD(path[i]) > 0) {
	checked = 1;
      }
    }
  }

  pvr_wait_ready();
  pvr_scene_begin();
  pvr_list_begin(PVR_LIST_OP_POLY);
  draw_banner(&hdr0, w, h);
  pvr_list_finish();
  pvr_list_begin(PVR_LIST_TR_POLY);
  draw_poly_strf(&hdr1, 0, 400, 1.0, 1.0, 1.0, 1.0, "LOADING...");
  pvr_list_finish();
  pvr_scene_finish();

  pvr_wait_ready();
  //pvr_scene_begin();
  //pvr_scene_finish();
  
  kos_mem_free(banner_tex);
  kos_mem_free(font_tex);
  return;
}

