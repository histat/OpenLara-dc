#ifndef H_GAPI_TA
#define H_GAPI_TA

#include "core.h"

#include "private.h"
#include "primitive/primitive.h"

#define PROFILE_MARKER(title)
#define PROFILE_LABEL(id, name, label)
#define PROFILE_TIMING(time)

#define COLOR_16

#define ENABLE_FOG

#define INV_SHORT_HALF      (1.0 / 32767.0)

#ifdef COLOR_16
#define CONV_COLOR(r,g,b) (((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3) | (1 << 15))
#else
#define CONV_COLOR(r,g,b) ((r << 16) | (g << 8) | b | (255 << 24))
#endif

#ifdef COLOR_16
#define CONV_LUMINANCE(a) (((a>>3)*0x421)|0x8000)
#else
#define CONV_LUMINANCE(a) (((a)*0x111)|0xff000000)
#endif

#define SW_MAX_DIST  (20.0f * 1024.0f)
#define SW_FOG_START (12.0f * 1024.0f)
#define WATER_FOG_DIST (6.0f * 1024.0f)

namespace GAPI {

    using namespace Core;

    typedef ::Vertex Vertex;

    int cullMode, blendMode;
    vec3 clearColor;

    #ifdef COLOR_16
        typedef uint16 ColorSW;
    #else
        typedef uint32 ColorSW;
    #endif
    ColorSW swPaletteColor[256];

    mat4 svMatrix;
    xMatrix tmpMatrix;

    enum {
      PaletteColor,
      PaletteWater,
      PaletteGray,
    };
    int PaletteFlag;

    #ifdef ENABLE_FOG
    float FogParams;
    uint32 fogColor;
    #endif

    vec3 ambient;
    int32 lightsCount;

   struct LightSW {
     vec3 intensity;
     vec3  pos;
     float att;
     float radius;
   } lights[MAX_LIGHTS];

    pvr_ptr_t pvr_base_mem;

    pvr_ptr_t allocVRAM(unsigned int size) {
      pvr_base_mem = pvr_mem_malloc( size );
      return pvr_base_mem;
    }

    void freeVRAM() {
      if (pvr_base_mem != NULL) {
          pvr_mem_free(pvr_base_mem);
      }

      pvr_base_mem = NULL;
    }

    pvr_context m_PvrContext;

    bool AlphaTestEnable;
    bool AlphaBlendEnable;

    pc_culling CullMode;
    pc_blend_mode AlphaBlendSrc;
    pc_blend_mode AlphaBlendDst;

    enum {
        FALSE = false,
        TRUE = true
    };

    void ReCompileHeader(pvr_context *dst) {

      pc_copy(&m_PvrContext, dst);
      if (AlphaBlendEnable || AlphaTestEnable) {
          
          pc_set_enable_alpha(dst, TRUE);
          pc_set_disable_texture_alpha(dst, FALSE);

          pc_set_list(dst, PC_BLEND_POLY);
          pc_set_blend_modes(dst, AlphaBlendSrc, AlphaBlendDst);

      } else {
          pc_set_list(dst, PC_OPAQUE_POLY);
          pc_set_enable_alpha(dst, FALSE);
          pc_set_disable_texture_alpha(dst, TRUE);
          pc_set_blend_modes(dst, PC_ONE, PC_ZERO);
      }
    }



// Shader
    struct Shader {
        void init(Pass pass, int type, int *def, int defCount) {}
        void deinit() {}
        void bind() {}
        void setParam(UniformType uType, const vec4  &value, int count = 1) {}
        void setParam(UniformType uType, const mat4  &value, int count = 1) {}
    };

// Texture
    static const struct FormatDesc {
      int bpp;
      pc_texture_format textureFormat;
    } formats[FMT_MAX] = {
      {  8, PC_ARGB4444}, // LUMINANCE
      { 32, PC_ARGB1555}, // RGBA
      { 16, PC_RGB565}, // RGB16
      { 16, PC_ARGB1555}, // RGBA16
      { 32, PC_PALETTE_8B}, // RG_FLOAT
      { 32, PC_PALETTE_8B}, // RG_HALF
      { 32, PC_PALETTE_8B}, // DEPTH
      { 32, PC_PALETTE_8B}, // SHADOW
    };

    #define ARGB1555(r,g,b,a)	( (((r)>>3)<<10) | (((g)>>3)<<5) |((b)>>3) | (((a)&0x80)<<8) )
    #define ARGB8888(r,g,b,a)	( ((a) << 24) | ((r)<<16) | ((g)<<8) | (b) )
    #define LUMINANCE(a)	((((a)>>4)*0x111)|0xf0e0)


#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )

#define MIN(a, b) ( (a)<(b)? (a):(b) )

        void twiddle(uint8_t *out, const uint8_t *in, uint32_t w, uint32_t h, int bpp) {
	  int x,y,min,mask,yout;
        
	  min = MIN(w, h);
	  mask = min - 1;
        
	  switch (bpp) {
	  case 8: {
	    uint8_t * pixels;
	    uint16_t * vtex;
	    pixels = (uint8_t *)in;
	    vtex = (uint16_t *)out;
	    for (y=0; y<h; y += 2) {
	      yout = y;
	      for (x=0; x<w; x++) {
		vtex[TWIDOUT((yout&mask)/2, x&mask) + 
		     (x/min + yout/min)*min*min/2] = 
		  pixels[y*w+x] | (pixels[(y+1)*w+x]<<8);
	      }
	    }
	  }
	    break;
	  };
	}
#undef TWIDTAB
#undef TWIDOUT
#undef MIN

    void tex_memcpy(void *dest, void *src, uint32_t cnt) {
      uint32_t *s = (uint32_t *)src;
      //uint32 *d = (uint32 *)(void *)                      \
	//(0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));

      //volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
      //qacr[0] = qacr[1] = 0xa4;

      uint32_t *d = sq_lock(dest);

      cnt >>= 6;

      asm("pref @%0" : : "r" (s));

      while (cnt--) {
	d[0] = *s++;
	d[1] = *s++;
	d[2] = *s++;
	d[3] = *s++;
	d[4] = *s++;
	d[5] = *s++;
	d[6] = *s++;
	d[7] = *s++;
	asm("pref @%0" : : "r" (d));
	d += 8;
	d[0] = *s++;
	d[1] = *s++;
	d[2] = *s++;
	d[3] = *s++;
	d[4] = *s++;
	d[5] = *s++;
	d[6] = *s++;
	d[7] = *s++;
	asm("pref @%0" : : "r" (d));
	d += 8;
      }

      sq_unlock();
    }

    #define COPY8888TO16(n) do {	\
        tmp = ARGB1555(s[0],s[1],s[2],s[3]);    \
        tmp |= ARGB1555(s[4],s[5],s[6],s[7]) << 16;         \
        d[n] = tmp;                             \
        s += 8;                                 \
    } while(0)

    void tex_memcpy_pal(void *dest, void *src, uint32_t cnt) {
      unsigned char *s = (unsigned char *)src;
      //    unsigned int *d = (unsigned int *)(void *)		\
	//(0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));

      //volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
      //qacr[0] = qacr[1] = 0xa4;

      uint32_t *d = sq_lock(dest);

      uint32 tmp;
      cnt >>= 5;

      asm("pref @%0" : : "r" (s));

      while (cnt--) {
	COPY8888TO16(0);
	COPY8888TO16(1);
	COPY8888TO16(2);
	COPY8888TO16(3);
	COPY8888TO16(4);
	COPY8888TO16(5);
	COPY8888TO16(6);
	COPY8888TO16(7);
	asm("pref @%0" : : "r" (d));
	d += 8;
	COPY8888TO16(0);
	COPY8888TO16(1);
	COPY8888TO16(2);
	COPY8888TO16(3);
	COPY8888TO16(4);
	COPY8888TO16(5);
	COPY8888TO16(6);
	COPY8888TO16(7);
	asm("pref @%0" : : "r" (d));
	d += 8;
      }

      sq_unlock();
    }


    struct Texture {
        pvr_ptr_t      memory;
        int        width, height, origWidth, origHeight;
        TexFormat  fmt;
        uint32     opt;

        Texture(int width, int height, int depth, uint32 opt) : memory(0), width(width), height(height), origWidth(width), origHeight(height), fmt(FMT_RGBA), opt(opt) {}

        void init(void *data) {
            ASSERT((opt & OPT_PROXY) == 0);

            opt &= ~(OPT_CUBEMAP | OPT_MIPMAPS);

	    FormatDesc desc = formats[fmt];

	    if (origWidth == 640) {
	      width = 640;
	    }

	    LOG("%dx%d orig %dx%d fmt=%d \n",width, height, origWidth,origHeight, fmt);

	    int size = width * height * 2;

            memory = pvr_mem_malloc( size );
            if (memory == NULL) {
	          LOG("Unable to create %dx%dx 0x%x \n", width, height, size);
	    }

            if (data && memory) {
                update(data);
            }
        }

        void deinit() {
            if (memory) {
		pvr_mem_free( memory );
                memory = NULL;
            }
        }


        void generateMipMap() {}

        void update(void *data) {
            ASSERT(data);

            FormatDesc desc = formats[fmt];
            if (desc.bpp == 8) {
	      int n = origWidth * origHeight;
	      uint16_t *dst = (uint16_t *)memory;
	      uint8_t *src = (uint8_t *)data;
	      while(n--) {
		uint8_t c = *src++;
		*dst++ = LUMINANCE(c);
	      }
            } else if (desc.bpp == 16) {
                uint16_t *dst = (uint16_t *)memory;
                uint16_t *src = (uint16_t *)data;
                if (width != origWidth /*|| height != origHeight*/) {
                    for (int y = 0; y < origHeight; y++) {
		                int n = origWidth * 2;
		                tex_memcpy(dst, src, n);
		                dst += width;
                        src += origWidth;
                    }
                } else {
                    int n = origWidth * origHeight * 2;
                    tex_memcpy(dst, src, n);
                }
            }  else if (desc.bpp == 32) {

	            if (width != origWidth /*|| height != origHeight*/) {
                    uint16_t *dst = (uint16_t *)memory;
                    uint32_t *src = (uint32_t *)data;
                    for (int y = 0; y < origHeight; y++) {
		                int n = origWidth;
            	        uint16_t *d = dst;
		                tex_memcpy_pal(dst, src, n);
		                dst += width;
                        src += origWidth;
                    }
	            } else {
		            int n = origWidth * origHeight;
		            uint16_t *dst = (uint16_t *)memory;
		            uint32_t *src = (uint32_t *)data;
		            tex_memcpy_pal(dst, src, n);
	            }
	        }
        }

        void bind(int sampler) {
            Core::active.textures[sampler] = this;

            if (!this || (opt & OPT_PROXY)) return;
            ASSERT(memory);

            FormatDesc desc = formats[fmt];
	    pvr_context *dst = &m_PvrContext;

	        if (opt & OPT_REPEAT) {
		  pc_set_uv_clamp(dst, PC_UV_NONE);
	        } else {
		  pc_set_uv_clamp(dst, PC_UV_XY);
	        }

	        if (opt & OPT_NEAREST) {
		  pc_set_filter(dst, PC_POINT);
	        } else {
		  pc_set_anisotropic(dst, 1);
		  pc_set_filter(dst, PC_BILINEAR);
	        }

		pc_texture_size u,v;
		u = pc_convert_size(width);
		v = pc_convert_size(height);

		if (width == 640) {
		  pc_set_strided(dst, 1);
		}

		pc_set_texture(dst, memory, u, v, desc.textureFormat, 0, 0, 0);
	}

        void bindTileIndices(void *tile) {
	    pc_texture_size u,v;
	    pvr_context *dst = &m_PvrContext;

	    u = pc_convert_size(width);
	    v = pc_convert_size(height);
	    pc_set_palette_8bit(dst, 0);
	    pc_set_texture(dst, tile, u, v, PC_PALETTE_8B, 1, 0, 0);
        }

        void unbind(int sampler) {}

        void setFilterQuality(int value) {
            if (value > Settings::LOW)
                opt &= ~OPT_NEAREST;
            else
                opt |= OPT_NEAREST;
        }
    };

// Mesh
    struct Mesh {
        Index        *iBuffer;
        GAPI::Vertex *vBuffer;

        int          iCount;
        int          vCount;
        bool         dynamic;

        Mesh(bool dynamic) : iBuffer(NULL), vBuffer(NULL), dynamic(dynamic) {}

        void init(Index *indices, int iCount, ::Vertex *vertices, int vCount, int aCount) {
            this->iCount  = iCount;
            this->vCount  = vCount;

            iBuffer = new Index[iCount];
            vBuffer = new Vertex[vCount];

            update(indices, iCount, vertices, vCount);
        }

        void deinit() {
            delete[] iBuffer;
            delete[] vBuffer;
        }

        void update(Index *indices, int iCount, ::Vertex *vertices, int vCount) {
            if (indices) {
                memcpy(iBuffer, indices, iCount * sizeof(indices[0]));
            }
            if (vertices) {
                memcpy(vBuffer, vertices, vCount * sizeof(vertices[0]));
            }
        }

        void bind(const MeshRange &range) const {}

        void initNextRange(MeshRange &range, int &aIndex) const {
            range.aIndex = -1;
        }
    };

    void init() {
        LOG("Vendor   : %s\n", "SEGA");
        LOG("Renderer : %s\n", "PowerVR2DC");
        LOG("Version  : %s\n", "0.1");

        support.texMinSize  = 8;

#ifdef COLOR_16
	pvr_set_pal_format(PVR_PAL_ARGB1555);
#else
	pvr_set_pal_format(PVR_PAL_ARGB8888);
#endif

        pvr_base_mem = NULL;

    pc_set_default_polygon_packed(&m_PvrContext);

#ifdef ENABLE_FOG
	pc_set_specular(&m_PvrContext, 1);
	pc_set_fog_mode(&m_PvrContext, PC_FOG_VERTEX);
    FogParams = 0.0f;
#endif
        //swDepth = NULL;

    }

    void deinit() {
        //delete[] swDepth;
    }

    inline mat4::ProjRange getProjRange() {
        return mat4::PROJ_ZERO_POS;
    }

    mat4 ortho(float l, float r, float b, float t, float znear, float zfar) {
        mat4 m;
        m.ortho(getProjRange(), l, r, b, t, znear, zfar);
        return m;
    }

    mat4 perspective(float fov, float aspect, float znear, float zfar, float eye) {
        mat4 m;
        m.perspective(getProjRange(), fov, aspect, znear, zfar, eye);
        return m;
    }

    bool beginFrame() {
      pvr_wait_ready();
      pvr_scene_begin();
      primitive_buffer_begin();
      return true;
    }

    void endFrame() {
      primitive_buffer_flush();
      pvr_scene_finish();
    }

    void resetState() {}

    void bindTarget(Texture *texture, int face) {}

    void discardTarget(bool color, bool depth) {}

    void copyTarget(Texture *dst, int xOffset, int yOffset, int x, int y, int width, int height) {}

    void setVSync(bool enable) {}

    void waitVBlank() {}

    void clear(bool color, bool depth) {
      if (color) {
	pvr_set_bg_color(clearColor.x, clearColor.y, clearColor.z);
      }
    }

    void setClearColor(const vec4 &color) {
      clearColor.x = color.x;
      clearColor.y = color.y;
      clearColor.z = color.z;
    }

    void setViewport(const short4 &v) {
      svMatrix.viewport(0.0f, (float)Core::height, (float)Core::width, -(float)Core::height, 0.0f, 1.0f);
    }

    void setScissor(const short4 &s) {
    }

    void setDepthTest(bool enable) {

    pc_set_depth_compare(&m_PvrContext, enable ? PC_GEQUAL : PC_ALWAYS );
    }

    void setDepthWrite(bool enable) {

	pc_set_depth_write_disable(&m_PvrContext, enable ? FALSE : TRUE);
    }

    void setColorWrite(bool r, bool g, bool b, bool a) {}

    void setAlphaTest(bool enable) {
      AlphaTestEnable = enable;
    }

    void setCullMode(int rsMask) {
        cullMode = rsMask;
        switch (rsMask) {
	case RS_CULL_BACK:
	  pc_set_cull_mode(&m_PvrContext, PC_CULL_CW);
	  break;
	case RS_CULL_FRONT:
	  pc_set_cull_mode(&m_PvrContext, PC_CULL_CCW);
	  break;
	default:
        pc_set_cull_mode(&m_PvrContext, PC_CULL_DISABLE);
        }
    }

    void setBlendMode(int rsMask) {

        blendMode = rsMask;

        switch (rsMask) {
            case RS_BLEND_ALPHA: 
                AlphaBlendSrc = PC_SRC_ALPHA;
                AlphaBlendDst = PC_INV_SRC_ALPHA;
                break;
            case RS_BLEND_ADD:
                AlphaBlendSrc = PC_ONE;
                AlphaBlendDst = PC_ONE;
                break;
            case RS_BLEND_MULT:
                AlphaBlendSrc = PC_OTHER_COLOR;
                AlphaBlendDst = PC_ZERO;
                break;
            case RS_BLEND_PREMULT:
                AlphaBlendSrc = PC_ONE;
                AlphaBlendDst = PC_INV_SRC_ALPHA;
                break;
            default:
                AlphaBlendEnable = FALSE;
                return;
        }
        AlphaBlendEnable = TRUE;
    }

    void setViewProj(const mat4 &mView, const mat4 &mProj) {}

    void updateLights(vec4 *lightPos, vec4 *lightColor, int count) {

      ambient = vec3(Core::active.material.y);

      lightsCount = 0;
      for (int i = 0; i < count; i++) {
	if (lightColor[i].w >= 1.0f) {
	  continue;
	}
	LightSW &light = lights[lightsCount++];
	light.intensity = lightColor[i].xyz();
	light.pos   = lightPos[i].xyz();
	light.radius = lightColor[i].w;
      }
    }

    void setFog(const vec4 &params) {
        FogParams = params.w;

        uint32 fogColor = 0;
        if (params.w > 0.0f) {
	        fogColor = 0x00000000
                | (uint32(clamp(params.x * 255.0f, 0.0f, 255.0f)) << 0)
                | (uint32(clamp(params.y * 255.0f, 0.0f, 255.0f)) << 8)
                | (uint32(clamp(params.z * 255.0f, 0.0f, 255.0f)) << 16);
		PVR_SET(0x0B4, fogColor);
        }
    }

    void applyLighting(vec3 &result, const Vertex &vertex) {
      //ubyte4 argb = vertex.color;

      vec3 coord  = vec3(float(vertex.coord.x), float(vertex.coord.y), float(vertex.coord.z));
      vec3 normal = vec3(float(vertex.normal.x), float(vertex.normal.y), float(vertex.normal.z));
      vec3f_normalize(normal.x, normal.y, normal.z);

      for (int i = 0; i < lightsCount; i++) {
	LightSW &light = lights[i];
	vec3 pos;

	float x, y, z;
	pos.x = light.pos.x;
	pos.y = light.pos.y;
	pos.z = light.pos.z;

	mat_trans_single3_nodiv(pos.x, pos.y, pos.z);

	vec3 dir = (pos - coord) * light.radius;

	float att;
	vec3f_dot(dir.x, dir.y, dir.z, dir.x, dir.y, dir.z, att);

	float lum;
	dir /= sqrtf(att);
	vec3f_dot(normal.x, normal.y, normal.z, dir.x, dir.y, dir.z, lum);
	
	result += light.intensity * (max(0.0f, lum) * max(0.0f, 1.0f - att));
      }

    }

	#define vec4f_ftrv(x, y, z, w) {       \
        register float __x __asm__("fr0") = x; \
        register float __y __asm__("fr1") = y; \
        register float __z __asm__("fr2") = z; \
        register float __w __asm__("fr3") = w; \
        __asm__ __volatile__( \
		"ftrv	xmtrx, fv0\n" \
	        "fldi1	fr2\n" \
		"fdiv	fr3, fr2\n" \
		"fmul	fr2, fr0\n" \
		"fmul	fr2, fr1\n" \
		: "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
		: "0" (__x), "1" (__y), "2" (__z), "3" (__w) \
                : ); \
        x = __x; y = __y; z = __z; w = __w; \
        }

   void transform(const Index *indices, const Vertex *vertices, int iStart, int iCount, int vStart) {

	polygon_vertex_t vertex_buffer[4] __attribute__((aligned(32)));

	//Vertex vertex[4];

    const bool colored = vertices[vStart + indices[iStart]].color.w == 142;
    int vIndex = 0;
    bool isTriangle = false;

	pvr_context pc;
	ReCompileHeader(&pc);

    if (colored) {
	    pc_set_textured(&pc, FALSE);
    } else {
	    pc_set_textured(&pc, TRUE);
    }

    primitive_header((void *)&pc, 32);

    uint32_t vcnt = 0;
    for (int i = 0; i < iCount; i++) {
            const Index  index   = indices[iStart + i];
            const Vertex &vertex = vertices[vStart + index];

            vIndex++;

            if (vIndex == 1) {
                isTriangle = vertex.normal.w == 1;
            } else {
                if (vIndex == 4) { // loader splits quads to two triangles with indices 012[02]3, we ignore [02] to make it quad again!
                    vIndex++;
                    i++;
                    continue;
                }
            }

            {
       unsigned int argb, oargb;
       vec4 c;

       c = vec4(vertex.coord.x, vertex.coord.y, vertex.coord.z, 1.0f);
       vec4f_ftrv(c.x, c.y, c.z, c.w);

       vec2 uv = vec2(vertex.texCoord.x, vertex.texCoord.y) * INV_SHORT_HALF;
       float depth = c.w;

       ubyte4 color;
       int C[4];

       if (colored) {
	 color.x = uint32(vertex.color.x * vertex.light.x) >> 8;
	 color.y = uint32(vertex.color.y * vertex.light.y) >> 8;
	 color.z = uint32(vertex.color.z * vertex.light.z) >> 8;
	 color.w = uint32(vertex.color.w * vertex.light.w) >> 8;
       } else {
           color = vertex.light;
       }

       if (PaletteFlag == PaletteWater) {
	 color.x = (uint32(color.x) * 150) >> 8;
	 color.y = (uint32(color.y) * 230) >> 8;
	 color.z = (uint32(color.z) * 230) >> 8;

	 depth -= WATER_FOG_DIST;
       } else {
	 depth -= SW_FOG_START;
       }

       vec3 result = vec3(0.0f);

       if (lightsCount) {
	 xmtrxPush();
	 xmtrxLoad(&tmpMatrix);
	 applyLighting(result, vertex);
	 xmtrxPop();
       } else if(c.z == 1.0f) {
	 result = vec3(1.0f);
	 color = vertex.light;
       }

       result += ambient;

       C[0] = int(result.x * color.x);
       C[1] = int(result.y * color.y);
       C[2] = int(result.z * color.z);
       C[3] = color.w;

       C[0] = C[0] > 255 ? 255 : C[0];
       C[1] = C[1] > 255 ? 255 : C[1];
       C[2] = C[2] > 255 ? 255 : C[2];

       C[0] = C[0] < 0 ? color.x : C[0];
       C[1] = C[1] < 0 ? color.y : C[1];
       C[2] = C[2] < 0 ? color.z : C[2];

       argb = ARGB8888(C[0], C[1], C[2], C[3]);

#ifdef ENABLE_FOG
       if (FogParams > 0.0f && depth > 0.0f && !AlphaBlendEnable) {
	 int alpha = 0;
	 alpha = clamp(int(depth * FogParams * 255), 0, 255);
	 oargb = alpha << 24;
       } else
#endif
	 oargb = 0;

       vertex_buffer[vcnt].x = c.x;
       vertex_buffer[vcnt].y = c.y;
       vertex_buffer[vcnt].z = c.z;
       vertex_buffer[vcnt].u = uv.x;
       vertex_buffer[vcnt].v = uv.y;

       vertex_buffer[vcnt].base.color = argb;
       vertex_buffer[vcnt].offset.color = oargb;

       vcnt++;

            }

	  if (isTriangle  && vIndex == 3) {
	    int face_list[] = {0, 1, 2};
	    primitive_nclip_polygon(vertex_buffer, face_list, 3);
        vcnt = 0;

        vIndex = 0;
	  } else if (vIndex == 6) {
	    int face_list[] = {4, 0, 1, 3, 2};
	    primitive_nclip_polygon_strip(vertex_buffer, face_list, 5);
        vcnt = 0;

        vIndex = 0;
	  }
    }
   }

    void DIP(Mesh *mesh, const MeshRange &range) {

      mat4 m;
      m = mModel.inverseOrtho();
      xmtrxLoadUnaligned((float*)&m);
      xmtrxStore(&tmpMatrix);

      xmtrxLoadUnaligned((float*)&svMatrix);
      xmtrxMultiplyUnaligned((float*)&mViewProj);
      xmtrxMultiplyUnaligned((float*)&mModel);

      transform(mesh->iBuffer, mesh->vBuffer, range.iStart, range.iCount, range.vStart);

    }

    void initPalette(Color24 *palette, uint8 *lightmap) {
        for (uint32_t i = 0; i < 256; i++) {
            const Color24 &p = palette[i];
            if ( i == 0 ) {
              swPaletteColor[i] = 0;
	      continue;
            }

            swPaletteColor[i] = CONV_COLOR(p.r, p.g, p.b);
        }

	for (uint32_t i = 0; i < 256; i++) {
	  pvr_set_pal_entry(i, swPaletteColor[i]);
	}

	for (uint32_t i = 0; i < 256; i++) {
	  pvr_set_pal_entry(i+256, CONV_LUMINANCE(i));
	}
    }

    void setPalette(int flag) {
	PaletteFlag = flag;
    }

    vec4 copyPixel(int x, int y) {
        return vec4(0.0f); // TODO: read from framebuffer
    }
}

#endif
