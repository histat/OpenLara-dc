#include <stdio.h>
#include <string.h>

#include <kos.h>
#include "private.h"

#include "primitive/primitive.h"
#include "watchdog.h"


extern void _audio_init();
extern void _audio_free();

extern void LaunchMenu();

/* Primitive buffer */
unsigned int __attribute__((aligned(32))) prim_buffer[256 * 1024 * 4];

xMatrix mtrx_stack[4];

KOS_INIT_FLAGS(INIT_DEFAULT);

pvr_init_params_t pvr_params = {
	{ PVR_BINSIZE_32, 0, PVR_BINSIZE_16, 0, 0 }, 
	(512 * 1024), 0, 0, 0, 3
};

void dc_init_hardware()
{
#ifndef NOSERIAL
  wdInit();
#endif

  xmtrxSetStackPointer(mtrx_stack, sizeof(mtrx_stack));

  _audio_init();

  pvr_init_defaults();

  pvr_set_zclip(0.0f);
  
  pvr_set_bg_color(0.0, 0.0, 0.0);

  PVR_FSET(PVR_SMALL_CULL, 0.01f);

  PVR_FSET(0x11C, 0.5f);
  PVR_SET(0x0e4, (640 >> 5));

#ifndef NOSERIAL
  wdPause();
#endif
  LaunchMenu();

  vid_set_enabled(0);
  vid_set_mode(DM_640x480, PM_RGB565);
  pvr_init(&pvr_params);
  vid_set_enabled(1);

  pvr_init (&pvr_params);

 /* Init primitive buffer */
  primitive_buffer_init(0, 0, -1);
  primitive_buffer_init(2, &prim_buffer[256 * 1024 * 0], 256 * 1024 * 4);

  //arch_set_exit_path(ARCH_EXIT_MENU);
}

void _bfont_draw(void *buffer, int bufwidth, int opaque, int c)
{
  int i,j;
  uint8_t *s = bfont_find_char(c);
  
  const int yalign = bufwidth;
  uint16_t *dst = (uint16_t *)buffer;
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

  buffer = pvr_mem_malloc( 256*256*2 );
  uint16_t *vram = (uint16_t *)buffer;

  vram = (uint16_t *)buffer;
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

void draw_poly_strf(pvr_context *hdr, float x1, float y1, float a, float r, float g, float b, char *str) {
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
  
  //void *sq = sqPrepare((void*)PVR_TA_INPUT);
  void *sq = sq_lock(PVR_TA_INPUT);
  sqCopy32(sq, hdr);
  
  s = str;
  while (*s) {
    if (*s == ' ') {
      x1 += 10.0f; s++;
    } else
      draw_poly_char(sq, x1+=14.0f, y1, a, r, g, b, *s++);
  }

  sq_unlock();
}

typedef struct
{
    char id[4]; // 'DTEX'
    short width;
    short height;
    int type;
    int size;
} tex_header_t;

pvr_ptr_t tex_load_rom(char *filename, int *tex_w, int *tex_h, int *type)
{
    file_t file = 0;
    pvr_ptr_t buffer;
    tex_header_t header;

    file = fs_open(filename, O_RDONLY);
    if (file < 0)
        return 0;

    /* load header */
    fs_read(file, &header, 16);
    *tex_w = (int)header.width;
    *tex_h = (int)header.height;
    *type = header.type;

    buffer = pvr_mem_malloc( header.size );
    fs_read(file, buffer, header.size);
    fs_close(file);

    return buffer;
}

#if 0
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

    buffer = pvr_mem_malloc( header.size );
    memcpy(buffer, in + 16, header.size);

    return buffer;
}
#endif

typedef struct {
  float a1,r1,g1,b1;
  float a2,r2,g2,b2;
  float a3,r3,g3,b3;
  float a4,r4,g4,b4;
} bgcolor;

static void draw_banner(pvr_context *hdr, int w, int h, bgcolor *bg)
{
    //void *sq = sqPrepare((void*)PVR_TA_INPUT);
    void *sq = sq_lock(PVR_TA_INPUT);
    sqCopy32(sq, hdr);

    float x1,y1;

    x1 = (640 - w) / 2;
    y1 = (480 - h) / 2;
    pvr_vertex32 *v = (pvr_vertex32*)sq + 32;
    
    pv_set_pos(v, x1, y1, 1.0);
    pv_set_uv(v, 0.0, 0.0);
    pv_set_argb_pack(v, bg->a1, bg->r1, bg->g1, bg->b1);
    pv_set_cmd_submit_vertex(v);
    v++;
		
    pv_set_pos(v, x1, y1 + h, 1.0);
    pv_set_uv(v, 0.0, 1.0);
    pv_set_argb_pack(v, bg->a2, bg->r2, bg->g2, bg->b2);
    pv_set_cmd_submit_vertex(v);
    v++;
		
    pv_set_pos(v, x1 + w, y1, 1.0);
    pv_set_uv(v, 1.0, 0.0);
    pv_set_argb_pack(v, bg->a3, bg->r3, bg->g3, bg->b3);
    pv_set_cmd_submit_vertex(v);
    v++;

    pv_set_pos(v, x1 + w, y1 + h, 1.0);
    pv_set_uv(v, 1.0, 1.0);
    pv_set_argb_pack(v, bg->a4, bg->r4, bg->g4, bg->b4);
    pv_set_cmd_submit_vertex_eos(v);
    v++;

    sq_unlock();
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

  file_t fd = fs_open(path, O_DIR);
  if (fd == -1) {
    return -1;
  }

  fs_close(fd);

  return 1;
}

void LaunchMenu() {
  int i, frame;
  int w, h, type;
  pvr_context hdr0,hdr1;
  pvr_ptr_t banner_tex;
  pvr_ptr_t font_tex;
  pvr_context *cxt0, *cxt1;

  banner_tex = tex_load_rom("/rd/lara.tex", &w, &h, &type);

  cxt0 = &hdr0;
  cxt1 = &hdr1;

  pc_set_default_polygon(cxt0);
  pc_set_color_format(cxt0, PC_PACKED);
  pc_set_cull_mode(cxt0, PC_CULL_DISABLE);
  pc_set_enable_alpha(cxt0, 0);
  pc_set_disable_texture_alpha(cxt0, 1);

  pc_copy(cxt0, cxt1);

  pc_set_texture(cxt0, banner_tex, pc_convert_size(w), pc_convert_size(h), (type>>27)&3, !((type>>16)&1), 0, ((type>>30)&1));

  font_tex = setup_font_texture();
  pc_set_enable_alpha(cxt1, 1);
  pc_set_disable_texture_alpha(cxt1, 0);
  pc_set_list(cxt1, PC_BLEND_POLY);
  pc_set_texture(cxt1, font_tex, pc_convert_size(256), pc_convert_size(256), PC_ARGB4444, 0, 0, 0);

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

  bgcolor b = {
     1.0, 1.0, 1.0, 1.0,
     1.0, 1.0, 1.0, 1.0,
     1.0, 1.0, 1.0, 1.0,
     1.0, 1.0, 1.0, 1.0
  };


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
    draw_banner(&hdr0, w, h, &b);
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

#if 1
    for (i = 0; path[i] != NULL; i++) {
      if(check_CD(path[i]) > 0) {
	checked = 1;
      }
    }
#endif
  }

  timer_spin_sleep(17*4);

  pvr_mem_free( (void*)banner_tex );
  pvr_mem_free( (void*)font_tex );

  return;
}

