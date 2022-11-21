#ifndef H_GAPI_TA
#define H_GAPI_TA

#include "core.h"

#include "private.h"
#include "xmtrx.h"
#include "primitive/primitive.h"

#define PROFILE_MARKER(title)
#define PROFILE_LABEL(id, name, label)
#define PROFILE_TIMING(time)

#define ENABLE_FOG

#define INV_SHORT_HALF      (1.0 / 32767.0)

#define CONV_COLOR(r,g,b) (((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3) | (1 << 15))

namespace GAPI {

    using namespace Core;

    typedef ::Vertex Vertex;

    typedef uint16 ColorSW;
    ColorSW swPaletteColor[256];

    xMatrix ScreenView;
    xMatrix ViewProj;

    enum {
      PaletteColor,
      PaletteWater,
      PaletteGray,
    };
    int PaletteFlag;

    #ifdef ENABLE_FOG
    float FogParams;
    //uint32 fogColor;
    #endif

    vec3 ambient;
    int32 lightsCount;

   struct LightPVR {
     vec3 intensity;
     vec3  pos;
     float att;
   } lights[MAX_LIGHTS];

    pvr_ptr_t pvr_base_mem;

    pvr_ptr_t allocVRAM(unsigned int size) {
      pvr_base_mem = (pvr_ptr_t)psp_valloc( size );
#ifndef NOSERIAL
      printf("[%s] tex = %p size 0x%x\n", __func__, pvr_base_mem, size);
#endif
      return pvr_base_mem;
    }

    void freeVRAM() {
      if (pvr_base_mem != NULL) {
	psp_vfree( (void*)pvr_base_mem );
#ifndef NOSERIAL
      printf("[%s] tex = %p size 0x%x\n", __func__, pvr_base_mem, size);
#endif
      }

      pvr_base_mem = NULL;
    }

    pvr_poly_cxt_t m_PvrContext;

    bool AlphaTestEnable;
    bool AlphaBlendEnable;
    bool DepthTestEnable;
    bool ZWriteEnable;

    int CullMode;
    int AlphaBlendSrc;
    int AlphaBlendDst;

    

    enum {
        FALSE = false,
        TRUE = true
    };

    void ReCompileHeader(pvr_poly_hdr_t *dst) {

      //m_PvrContext.txr.enable = PVR_TEXTURE_DISABLE;

        if(AlphaBlendEnable || AlphaTestEnable) {
	        m_PvrContext.gen.alpha = PVR_ALPHA_ENABLE;
	        m_PvrContext.txr.alpha = PVR_TXRALPHA_ENABLE;
	        m_PvrContext.txr.env = PVR_TXRENV_MODULATEALPHA;

	        if(AlphaBlendEnable) {
		    m_PvrContext.list_type = PVR_LIST_TR_POLY;
	            m_PvrContext.blend.src = AlphaBlendSrc;
	            m_PvrContext.blend.dst = AlphaBlendDst;
	        } else {
	            m_PvrContext.list_type = PVR_LIST_PT_POLY;
	            m_PvrContext.blend.src = PVR_BLEND_SRCALPHA;
	            m_PvrContext.blend.dst = PVR_BLEND_INVSRCALPHA;
	        }

	    } else {
            m_PvrContext.list_type = PVR_LIST_OP_POLY;
            m_PvrContext.gen.alpha = PVR_ALPHA_DISABLE;
            m_PvrContext.blend.src = PVR_BLEND_ONE;
	        m_PvrContext.blend.dst = PVR_BLEND_ZERO;
            m_PvrContext.txr.alpha = PVR_TXRALPHA_DISABLE;
            m_PvrContext.txr.env = PVR_TXRENV_MODULATE;
        }

        if (ZWriteEnable) {
            m_PvrContext.depth.write = PVR_DEPTHWRITE_ENABLE;
        } else {
            m_PvrContext.depth.write = PVR_DEPTHWRITE_DISABLE;
        }

        if (DepthTestEnable) {
	        m_PvrContext.depth.comparison = PVR_DEPTHCMP_GEQUAL;
        } else {
	        m_PvrContext.depth.comparison = PVR_DEPTHCMP_ALWAYS;
        }

        m_PvrContext.gen.culling = CullMode;

        kos_poly_compile(dst, &m_PvrContext);
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
      int bpp, textureFormat;
    } formats[FMT_MAX] = {
      {  8, PVR_TXRFMT_ARGB4444|PVR_TXRFMT_NONTWIDDLED}, // LUMINANCE
      { 32, PVR_TXRFMT_ARGB1555|PVR_TXRFMT_NONTWIDDLED}, // RGBA
      { 16, PVR_TXRFMT_RGB565|PVR_TXRFMT_NONTWIDDLED}, // RGB16
      { 16, PVR_TXRFMT_ARGB1555|PVR_TXRFMT_NONTWIDDLED}, // RGBA16
      { 32, PVR_TXRFMT_ARGB1555|PVR_TXRFMT_NONTWIDDLED}, // RG_FLOAT
      { 32, PVR_TXRFMT_ARGB1555|PVR_TXRFMT_NONTWIDDLED}, // RG_HALF
      { 32, PVR_TXRFMT_ARGB1555|PVR_TXRFMT_NONTWIDDLED}, // DEPTH
      { 32, PVR_TXRFMT_ARGB1555|PVR_TXRFMT_NONTWIDDLED}, // SHADOW
    };

    #define ARGB1555(r,g,b,a)	( (((r)>>3)<<10) | (((g)>>3)<<5) |((b)>>3) | (((a)&0x80)<<8) )
    #define ARGB8888(r,g,b,a)	( ((a) << 24) | ((r)<<16) | ((g)<<8) | (b) )
    #define LUMINANCE(a)	((((a)>>4)*0x111)|0xf0e0)


#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )

#define MIN(a, b) ( (a)<(b)? (a):(b) )

        void twiddle(uint8 *out, const uint8 *in, uint32 w, uint32 h, int bpp) {
	  int x,y,min,mask,yout;
        
	  min = MIN(w, h);
	  mask = min - 1;
        
	  switch (bpp) {
	  case 8: {
	    uint8 * pixels;
	    uint16 * vtex;
	    pixels = (uint8 *)in;
	    vtex = (uint16 *)out;
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

    void upload_vram(uint8 *out, const uint8 *in, uint32 w, uint32 h) {
	    uint8 *dst = out + 2048;
	    uint32 *s = (uint32 *)in;
	    uint32 *d = (uint32 *)(void *)				\
	        (0xe0000000 | (((unsigned long)dst) & 0x03ffffc0));

	    volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
	    qacr[0] = qacr[1] = 0xa4;
	
	    int cnt = w * h;
        cnt >>= 6;

    	while (cnt--) {
	        d[0] = *s++;
	        d[1] = *s++;
	        d[2] = *s++;
	        d[3] = *s++;
            __asm__("pref @%0" : : "r" (s+8));
	        d[4] = *s++;
	        d[5] = *s++;
	        d[6] = *s++;
	        d[7] = *s++;
	        __asm__("pref @%0" : : "r" (d));
	        d += 8;
	        d[0] = *s++;
	        d[1] = *s++;
	        d[2] = *s++;
	        d[3] = *s++;
            __asm__("pref @%0" : : "r" (s+8));
	        d[4] = *s++;
	        d[5] = *s++;
	        d[6] = *s++;
	        d[7] = *s++;
	        __asm__("pref @%0" : : "r" (d));
	        d += 8;
	    }
    }

    void tex_memcpy(void *dest, void *src, uint32 cnt) {
	    uint32 *s = (uint32 *)src;
	    uint32 *d = (uint32 *)(void *)				\
	        (0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));

	    volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
	    qacr[0] = qacr[1] = 0xa4;

        cnt >>= 6;

	asm("pref @%0" : : "r" (s));

        while (cnt--) {
	  asm("pref @%0" : : "r" (s+8));
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
	  asm("pref @%0" : : "r" (s+8));
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
    }

    #define COPY8888TO16(n) do {	\
        tmp = ARGB1555(s[0],s[1],s[2],s[3]);    \
        tmp |= ARGB1555(s[4],s[5],s[6],s[7]) << 16;         \
        d[n] = tmp;                             \
        s += 8;                                 \
    } while(0)

    void tex_memcpy_pal(void *dest, void *src, uint32 cnt) {
        unsigned char *s = (unsigned char *)src;
        unsigned int *d = (unsigned int *)(void *)              \
          (0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));

	    volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
	    qacr[0] = qacr[1] = 0xa4;

	    uint32 tmp;
	    cnt >>= 5;
	
        while (cnt--) {
          COPY8888TO16(0);
          COPY8888TO16(1);
          COPY8888TO16(2);
          COPY8888TO16(3);
	        __asm__("pref @%0" : : "r" (s+16*8));
          COPY8888TO16(4);
          COPY8888TO16(5);
          COPY8888TO16(6);
          COPY8888TO16(7);
          __asm__("pref @%0" : : "r" (d));
          d += 8;
          COPY8888TO16(0);
          COPY8888TO16(1);
          COPY8888TO16(2);
          COPY8888TO16(3);
	        __asm__("pref @%0" : : "r" (s+16*8));
          COPY8888TO16(4);
          COPY8888TO16(5);
          COPY8888TO16(6);
          COPY8888TO16(7);
            __asm__("pref @%0" : : "r" (d));
          d += 8;
        }
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

            int size = 0;

	    size = width * height * 2;

            memory = (pvr_ptr_t)psp_valloc( size );
#ifndef NOSERIAL
	    printf("[%s] tex = %p size 0x%x\n", __func__, memory, size);
#endif

            if (memory == NULL) {
	          LOG("Unable to create %dx%dx 0x%x \n", width, height, size);
	    }

            if (data && memory) {
                update(data);
            }
        }

        void deinit() {
            if (memory) {
		psp_vfree( (void*)memory );
#ifndef NOSERIAL
      printf("[%s] tex = %p size 0x%x\n", __func__, pvr_base_mem, size);
#endif
                memory = NULL;
            }
        }


        void generateMipMap() {}

        void update(void *data) {
            ASSERT(data);

            FormatDesc desc = formats[fmt];
            if (desc.bpp == 8) {
              int n = origWidth * origHeight;
	      uint8 *dst = (uint8 *)memory;
	      uint8 *src = (uint8 *)data;
	      while(n--) {
		uint8 c = *src++;
		*dst++ = LUMINANCE(c);
	      }
            } else if (desc.bpp == 16) {
                uint16 *dst = (uint16 *)memory;
                uint16 *src = (uint16 *)data;
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
                    uint16 *dst = (uint16 *)memory;
                    uint32 *src = (uint32 *)data;
                    for (int y = 0; y < origHeight; y++) {
		                int n = origWidth;
            	        uint16 *d = dst;
		                tex_memcpy_pal(dst, src, n);
		                dst += width;
                        src += origWidth;
                    }
	            } else {
		            int n = origWidth * origHeight;
		            uint16 *dst = (uint16 *)memory;
		            uint32 *src = (uint32 *)data;
		            tex_memcpy_pal(dst, src, n);
	            }
	        }
        }

        void bind(int sampler) {
            Core::active.textures[sampler] = this;

            if (!this || (opt & OPT_PROXY)) return;
            ASSERT(memory);

            FormatDesc desc = formats[fmt];
	  
	        if (opt & OPT_REPEAT) {
	            m_PvrContext.txr.uv_clamp = PVR_UVCLAMP_NONE;
	        } else {
	            m_PvrContext.txr.uv_clamp = PVR_UVCLAMP_UV;
	        }

	        if (opt & OPT_NEAREST) {
	            m_PvrContext.txr.filter = PVR_FILTER_NEAREST;
	        } else {
	            m_PvrContext.txr.filter = PVR_FILTER_BILINEAR;
	        }

	        m_PvrContext.txr.width = width;
	        m_PvrContext.txr.height = height;
	        m_PvrContext.txr.base = memory;
	        m_PvrContext.txr.format = desc.textureFormat;

		if (width == 640) {
		  m_PvrContext.txr.format |= PVR_TXRFMT_STRIDE;
		}
	}

        void bindTileIndices(void *tile) {
            m_PvrContext.txr.width = width;
	    m_PvrContext.txr.height = height;
	    m_PvrContext.txr.base = (pvr_ptr_t)tile;
            m_PvrContext.txr.format = PVR_TXRFMT_PAL8BPP|PVR_TXRFMT_8BPP_PAL(0);
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


    int cullMode, blendMode;

    void init() {
        LOG("Vendor   : %s\n", "SEGA");
        LOG("Renderer : %s\n", "PowerVR2DC");
        LOG("Version  : %s\n", "0.1");

        support.texMinSize  = 8;

        pvr_base_mem = NULL;

        pvr_poly_cxt_txr(&m_PvrContext, PVR_LIST_OP_POLY, PVR_TXRFMT_ARGB1555, 8, 8, 0, 0);

        m_PvrContext.gen.specular = 1;
	m_PvrContext.gen.fog_type = PVR_FOG_VERTEX;
	#ifdef ENABLE_FOG
        FogParams = 0.0f;
	PVR_SET(0x0B4, PVR_PACK_COLOR(0, 0, 0, 0));
        #endif
    }

    void deinit() {}

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
        return true;
    }

    void endFrame() {}

    void resetState() {}

    void bindTarget(Texture *texture, int face) {}

    void discardTarget(bool color, bool depth) {}

    void copyTarget(Texture *dst, int xOffset, int yOffset, int x, int y, int width, int height) {}

    void setVSync(bool enable) {}

    void waitVBlank() {}

    void clear(bool color, bool depth) {}

    void setClearColor(const vec4 &color) {}

    void setViewport(const short4 &v) {
       mat4 m;

       m.viewport((float)v.x, (float)v.y+v.w, (float)v.z, -(float)v.w, 0.0f, 1.0f);
       xmtrxLoadUnaligned((float*)&m);
       xmtrxStoreUnaligned((float*)&ScreenView);

      //xmtrxPrintLn();
    }

    void setScissor(const short4 &s) {
    }

    void setDepthTest(bool enable) {

        DepthTestEnable = enable;
    }

    void setDepthWrite(bool enable) {

        ZWriteEnable = enable;
    }

    void setColorWrite(bool r, bool g, bool b, bool a) {}

    void setAlphaTest(bool enable) {

        AlphaTestEnable = enable;
    }

    void setCullMode(int rsMask) {
        cullMode = rsMask;
        switch (rsMask) {
            case RS_CULL_BACK  : CullMode = PVR_CULLING_CW;  break;
            case RS_CULL_FRONT : CullMode = PVR_CULLING_CCW; break;
            default            : CullMode = PVR_CULLING_NONE;
        }
    }

    void setBlendMode(int rsMask) {

        blendMode = rsMask;
        switch (rsMask) {
            case RS_BLEND_ALPHA: 
                AlphaBlendSrc = PVR_BLEND_SRCALPHA;
                AlphaBlendDst = PVR_BLEND_INVSRCALPHA;
                break;
            case RS_BLEND_ADD:
                AlphaBlendSrc = PVR_BLEND_ONE;
                AlphaBlendDst = PVR_BLEND_ONE;
                break;
            case RS_BLEND_MULT:
                AlphaBlendSrc = PVR_BLEND_DESTCOLOR;
                AlphaBlendDst = PVR_BLEND_ZERO;
                break;
            case RS_BLEND_PREMULT :
                AlphaBlendSrc = PVR_BLEND_ONE;
                AlphaBlendDst = PVR_BLEND_INVSRCALPHA;
                break;
            default:
                AlphaBlendEnable = FALSE;
                return;
        }
        AlphaBlendEnable = TRUE;
    }

    void setViewProj(const mat4 &mView, const mat4 &mProj) {

      xmtrxLoadUnaligned((float*)&mProj);
      xmtrxMultiplyUnaligned((float*)&mView);
      xmtrxStoreUnaligned((float*)&ViewProj);
    }

    void updateLights(vec4 *lightPos, vec4 *lightColor, int count) {

      ambient = vec3(Core::active.material.y);
      //ambient = Core::active.material.xyz();

      lightsCount = 0;
      for (int i = 0; i < count; i++) {
	if (lightColor[i].w >= 1.0f) {
	  continue;
	}
	LightPVR &light = lights[lightsCount++];
	light.intensity = lightColor[i].xyz();
	light.pos   = lightPos[i].xyz();
	light.att   = lightColor[i].w * lightColor[i].w;
      }
    }

    void setFog(const vec4 &params) {
        #ifdef ENABLE_FOG
        FogParams = params.w;

	uint32 fogColor = 0x00000000;
        if (params.w > 0.0f) {
	        fogColor = 0x00000000
                | (uint32(clamp(params.x * 255.0f, 0.0f, 255.0f)) << 0)
                | (uint32(clamp(params.y * 255.0f, 0.0f, 255.0f)) << 8)
                | (uint32(clamp(params.z * 255.0f, 0.0f, 255.0f)) << 16);
		//PVR_SET(0x0B4, fogColor);
        }
        #endif
    }

    uint32 applyLighting(const Vertex &vertex, ubyte4 argb) {
      //ubyte4 argb = vertex.color;
      vec3 result = vec3(0.0f);
      uint32 R, G, B, A;

      vec3 coord  = vec3(float(vertex.coord.x), float(vertex.coord.y), float(vertex.coord.z));
      vec3 normal = vec3(float(vertex.normal.x), float(vertex.normal.y), float(vertex.normal.z));
      vec3f_normalize(normal.x, normal.y, normal.z);

      for (int i = 0; i < lightsCount; i++) {
	LightPVR &light = lights[i];
	vec3 color = light.intensity;

	vec3 L = light.pos - coord;
	vec3f_normalize(L.x, L.y, L.z);
	
	float D;
	vec3 dis = light.pos - coord;
	vec3f_length(dis.x, dis.y, dis.z, D);

	// I = 1 / kc + kl + kq
	// kc = 1.0
	// kl = 0.0
	// kq = quadratic * d^2
	//float att = 1.0f / (1.0f + light.att * SQRT(D));
	float att = (1.0f + light.att * SQRT(D));
	att = INVERT(att);

	float LdotN;
	vec3f_dot(normal.x, normal.y, normal.z, L.x, L.y, L.z, LdotN);

	float strength = max(0.0f, LdotN);
	vec3 amb = ambient;

	vec3 diffuse = color * strength;
	diffuse *= att;
	amb *= att;
	result += (amb + diffuse);
      }

      R = clamp(int(result.x * argb.x), 0, 255);
      G = clamp(int(result.y * argb.y), 0, 255);
      B = clamp(int(result.z * argb.z), 0, 255);
      A = argb.w;

      return ARGB8888( R, G, B, A);
    }

    void transform(const Index *indices, const Vertex *vertices, int iStart, int iCount, int vStart) {

	#define vec4f_ftrv(x, y, z, w) {       \
        register float __x __asm__("fr0") = x; \
        register float __y __asm__("fr1") = y; \
        register float __z __asm__("fr2") = z; \
        register float __w __asm__("fr3") = w; \
        __asm__ __volatile__( \
                            "ftrv  xmtrx, fv0\n" \
                            "fldi1 fr2\n" \
                            "fdiv    fr3, fr2\n" \
                            "fmul    fr2, fr0\n" \
                            "fmul    fr2, fr1\n" \
                            : "=f" (__x), "=f" (__y), "=f" (__z), "=f" (__w) \
                            : "0" (__x), "1" (__y), "2" (__z), "3" (__w) \
                            : ); \
        x = __x; y = __y; z = __z; w = __w; \
        }

	polygon_vertex_t vertex_buffer[6] __attribute__((aligned(32)));

	pvr_context dst;

	pvr_poly_hdr_t hdr;
	ReCompileHeader(&hdr);

	pc_copy((const pvr_context *)&hdr, &dst);

        //int vIndex = 0;
        //bool isTriangle = false;
        //int vcount = 0;
	//Index index[4];
	Vertex vertex[6];
	int face_list[7];

        for (int i = 0; i < iCount;) {
	  
	  Index index[6];
	  int vcount = 0;

	  index[0]   = indices[iStart + i + 0];
	  index[1]   = indices[iStart + i + 1];
	  index[2]   = indices[iStart + i + 2];
	  vertex[0] = vertices[vStart + index[0]];
	  vertex[1] = vertices[vStart + index[1]];
	  vertex[2] = vertices[vStart + index[2]];

	  bool colored = false;

	  bool isTriangle = vertex[0].normal.w == 1;

	  if (isTriangle) {
	    face_list[0] = 3;
	    face_list[1] = 0;
	    face_list[2] = 1;
	    face_list[3] = 2;
	    i += 3;

	    vcount = 3;
	    colored = vertex[1].texCoord.y == 0;
	  } else {

	    index[3]   = indices[iStart + i + 5];
	    vertex[3] = vertices[vStart + index[3]];

	    i += 6;
	    vcount = 4;
	    
	    face_list[0] = 4;
	    face_list[1] = 0;
	    face_list[2] = 1;
	    face_list[3] = 3;
	    face_list[4] = 2;

	    colored = vertex[3].texCoord.y == 0;
	  }

	  if(colored) {
	    pc_set_textured(&dst, 0);
	  } else {
	    pc_set_textured(&dst, 1);
	  }


	  primitive_header((void *)&dst, 32);

	  unsigned int argb, oargb;
	  packed_color_t tmp;
	  
	  for (int i=0; i<vcount; i++) {

	    vec4 c;
	    #if 0
	    mat_trans_single3_nomod(vertex[i].coord.x, vertex[i].coord.y, vertex[i].coord.z, c.x, c.y, c.z);
	    #else
	    c = vec4(vertex[i].coord.x, vertex[i].coord.y, vertex[i].coord.z, 1.0f);
	    vec4f_ftrv(c.x, c.y, c.z, c.w);
	    #endif

	    //xmtrxPush();
	    //xmtrxIdentity();
	    //xmtrxScale(INV_SHORT_HALF, INV_SHORT_HALF, 1.0);
	    vec2 uv = vec2(vertex[i].texCoord.x * INV_SHORT_HALF, vertex[i].texCoord.y * INV_SHORT_HALF);
	    //vec2 uv = vec2(vertex[i].texCoord.x, vertex[i].texCoord.y);
	    //mat_trans_2d_single(uv.x, uv.y);

	    //xmtrxPop();

	    ubyte4 color;
	    
	    if(colored) {
	      color = vertex[i].color;
	    } else {
	      color = vertex[i].light;
	    }

	    if (PaletteFlag == PaletteWater) {
	      color.x = (uint32(color.x) * 150) >> 8;
	      color.y = (uint32(color.y) * 230) >> 8;
	      color.z = (uint32(color.z) * 230) >> 8;
	    }

	    if (lightsCount) {
	      argb = applyLighting(vertex[i], color);
	    } else {
	      if(c.z == 1.0f) {
		color = vertex[i].light;
	      }
	      argb = ARGB8888(color.x, color.y, color.z, color.w);
	    }

#ifdef ENABLE_FOG
            if (FogParams > 0.0f) {
	      float depth = c.w;
	      
	      float alpha = clamp(depth * FogParams, 0.0f, 1.0f);
	      oargb = int(alpha * 255) << 24;
            } else
#endif
	    oargb = 0;

	    vertex_buffer[i].x = c.x;
	    vertex_buffer[i].y = c.y;
	    vertex_buffer[i].z = c.z;
	    vertex_buffer[i].u = uv.x;
	    vertex_buffer[i].v = uv.y;
	    
	    vertex_buffer[i].base.color = argb;
	    vertex_buffer[i].offset.color = oargb;

	  }
	  vcount++; // skip primitive size
	  primitive_nclip_polygon_strip(vertex_buffer, face_list, vcount);
        }
    }

    void DIP(Mesh *mesh, const MeshRange &range) {

      xmtrxLoadUnaligned((float*)&ScreenView);
      xmtrxMultiplyUnaligned((float*)&ViewProj);
      xmtrxMultiplyUnaligned((float*)&mModel);
      
      transform(mesh->iBuffer, mesh->vBuffer, range.iStart, range.iCount, range.vStart);
    }

    void initPalette(Color24 *palette, uint8 *lightmap) {
        for (uint32 i = 0; i < 256; i++) {
            const Color24 &p = palette[i];
            if ( i == 0 ) {
              swPaletteColor[i] = 0;
	      continue;
            }

            swPaletteColor[i] = CONV_COLOR(p.r, p.g, p.b);
        }

	for (uint32 i = 0; i < 256; i++) {
	  pvr_set_pal_entry(i, swPaletteColor[i]);
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
