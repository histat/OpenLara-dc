#ifndef H_GAPI_TA
#define H_GAPI_TA

#include "core.h"


#include <ronin/ta.h>
#include <ronin/matrix.h>
#include "private.h"
#include "primitive/primitive.h"

#define PROFILE_MARKER(title)
#define PROFILE_LABEL(id, name, label)
#define PROFILE_TIMING(time)

namespace GAPI {

    using namespace Core;

    struct Vertex {
        short2 texCoord;
        ubyte4 color;
        short4 normal;
        short3 coord;
    };

    vec3 ambient;
    int32 lightsCount;

   struct LightPVR {
     vec3  color;
     vec3  pos;
     float att;
   } lights[MAX_LIGHTS];

    #define ARGB1555(r,g,b,a)	( (((r)>>3)<<10) | (((g)>>3)<<5) |((b)>>3) | (((a)&0x80)<<8) )
    #define ARGB4444(r,g,b,a)	( (((((a)&0xf0)<<8) | ((r)>>4))<<8) | ((g)&0xf0) | ((b)>>4) )
    #define ARGB8888(r,g,b,a)	( ((a) << 24) | ((r)<<16) | ((g)<<8) | (b) )
    #define LUMINANCE(a)	((((a)>>4)*0x111)|0xf0e0)

    #define TA_PAL8BPP_TWID (PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_TWIDDLED)
    #define TA_PAL4BPP_TWID (PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_TWIDDLED)
        
    pvr_ptr_t pvr_base_mem;
    
    pvr_ptr_t allocVRAM(unsigned int size) {
      pvr_base_mem = pvr_mem_malloc(size);
      return pvr_base_mem;
    }

    void freeVRAM() {
      if (pvr_base_mem != NULL)
        pvr_mem_free(pvr_base_mem);

      pvr_base_mem = NULL;
    }

    mat4 m_Matrix[4] __attribute__((aligned(32)));

    pvr_poly_cxt_t m_PvrContext;

    bool AlphaTestEnable;
    bool AlphaBlendEnable;
    bool DepthTestEnable;
    bool ZWriteEnable;

    int CullMode;
    int AlphaBlendSrc;
    int AlphaBlendDst;

    float scaleX;
    float scaleY;

    enum {
        FALSE = false,
        TRUE = true
    };

    void ReCompileHeader(pvr_poly_hdr_t *dst) {

	//m_PvrContext.txr.enable = PVR_TEXTURE_DISABLE;

        if(AlphaBlendEnable || AlphaTestEnable) {
	  m_PvrContext.list_type = PVR_LIST_TR_POLY;
	  m_PvrContext.gen.alpha = PVR_ALPHA_ENABLE;
	  m_PvrContext.txr.alpha = PVR_TXRALPHA_ENABLE;
	  m_PvrContext.txr.env = PVR_TXRENV_MODULATEALPHA;

	  if(AlphaBlendEnable) {
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
	  m_PvrContext.depth.comparison = PVR_DEPTHCMP_GREATER;
        } else {
	  m_PvrContext.depth.comparison = PVR_DEPTHCMP_ALWAYS;
        }

        m_PvrContext.gen.culling = CullMode;

        pvr_poly_compile(dst, &m_PvrContext);
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

    struct Texture {
      int width, height, origWidth, origHeight;
      TexFormat  fmt;
      uint32     opt;
      pvr_ptr_t  memory;
      
#define TWIDTAB(x) ( (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)| \
                     ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9) )
#define TWIDOUT(x, y) ( TWIDTAB((y)) | (TWIDTAB((x)) << 1) )

#define MIN(a, b) ( (a)<(b)? (a):(b) )

      void twiddle(uint8 *out, const uint8 *in, uint32 w, uint32 h, int bpp) {
        int x,y,min,mask,yout;
        
        min = MIN(w, h);
        mask = min - 1;
        
        switch (bpp) {
        case 4: {
          uint8 * pixels;
          uint16 * vtex;
          pixels = (uint8 *)in;
          vtex = (uint16 *)out;

          for (y=0; y<h; y += 2) {
            yout = y;
            for (x=0; x<w; x += 2) {
              vtex[TWIDOUT((x&mask)/2, (yout&mask)/2) + 
                   (x/min + yout/min)*min*min/4] = 
                (pixels[(x+y*w) >>1]&15) | ((pixels[(x+(y+1)*w) >>1]&15)<<4) | 
                ((pixels[(x+y*w) >>1]>>4)<<8) | ((pixels[(x+(y+1)*w) >>1]>>4)<<12);
            }
          }
        }
          break;
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
        }

      }


        Texture(int width, int height, int depth, uint32 opt) : width(width), height(height), origWidth(width), origHeight(height), fmt(FMT_RGBA), opt(opt) {}

        void init(void *data) {
            ASSERT((opt & OPT_PROXY) == 0);

            opt &= ~(OPT_CUBEMAP | OPT_MIPMAPS);

            FormatDesc desc = formats[fmt];

            if (width < 8 || height < 8) {
                LOG("texture too small %dx%d [%d %d]!\n", width, height, fmt, opt);
                width  = 8;
                height = 8;
		//data   = NULL;
            }

            if (width > 1024 || height > 1024) {
                LOG("texture too large %dx%d [%d %d]!\n", width, height, fmt, opt);
                width  = 8;
                height = 8;
		data   = NULL;
            }

	    if (origWidth == 320) {
              width = 320;
            }

            printf("[%s] %dx%d orig %dx%d fmt=%d \n",__func__,width, height, origWidth,origHeight, fmt);

            int size = 0;

	    size = width * height * 2;

	    memory = pvr_mem_malloc(size);
	    if (memory == NULL) {
	      LOG("Unable to create %dx%dx%d \n", width, height, size);
	    }
            if (data) {
              update(data);
            }
        }

        void deinit() {
            if (memory)
              pvr_mem_free(memory);
            memory = NULL;
        }

      void upload_vram(uint8 *out, const uint8 *in, uint32 w, uint32 h) {
	uint8 *dst = out;
	uint32 *s = (uint32 *)in;
	uint32 *d = (uint32 *)(void *)				\
	  (0xe0000000 | (((unsigned long)dst) & 0x03ffffc0));

	volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
	qacr[0] = qacr[1] = 0xa4;
	
	int cnt = w * h / 32;

	while (cnt--) {
	  __asm__("pref @%0" : : "r" (s+8));
	  d[0] = *s++;
	  d[1] = *s++;
	  d[2] = *s++;
	  d[3] = *s++;
	  d[4] = *s++;
	  d[5] = *s++;
	  d[6] = *s++;
	  d[7] = *s++;
	  __asm__("pref @%0" : : "r" (d));
	  d += 8;
	}
      }


#define COPY8888TO16(n) do {	\
        tmp = ARGB1555(s[0],s[1],s[2],s[3]);    \
        tmp |= ARGB1555(s[4],s[5],s[6],s[7]) << 16;         \
        d[n] = tmp;                             \
        s += 8;                                 \
      } while(0)

      void tex_memcpy_pal(void *dest, void *src, uint32 cnt)
      {
        unsigned char *s = (unsigned char *)src;
        unsigned int *d = (unsigned int *)(void *)              \
          (0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));

	volatile unsigned int *qacr = (volatile unsigned int *)0xff000038;
	qacr[0] = qacr[1] = 0xa4;

	uint32 tmp;
	cnt /= 32;
	
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


        void generateMipMap() {}

        void update(void *data) {
            FormatDesc desc = formats[fmt];

	    //*((volatile unsigned int *)(void *)0xa05f8040) = 0xFF0000;

	    if(memory == NULL) {
	      printf("ERR:texture update data\n");
	      return;
	    }

            if (desc.bpp == 8) {
	      int n = origWidth * origHeight;
	      uint16 *dst = (uint16 *)memory;
	      uint8 *src = (uint8 *)data;
	      while(n--) {
		uint8 c = *src++;
		*dst++ = LUMINANCE(c);
	      }
            } else if (desc.bpp == 16 && fmt == 2) {
	      int n = origWidth * origHeight;
	      uint16 *dst = (uint16 *)memory;
	      uint16 *src = (uint16 *)data;
	      memcpy(dst, src, n);
            } else if (desc.bpp == 16 && fmt == 3) {
	      int n = origWidth * origHeight;
	      uint16 *dst = (uint16 *)memory;
	      uint16 *src = (uint16 *)data;
	      memcpy(dst, src, n);
            }  else if (desc.bpp == 32) {

	      if (width != origWidth /*|| height != origHeight*/) {
                uint16 *dst = (uint16 *)memory;
                uint8 *src = (uint8 *)data;
                for (int y = 0; y < origHeight; y++) {
		  int n = origWidth;
		  uint16 *d = dst;
		  tex_memcpy_pal(dst, src, n);
		  dst += width;
                }
	      } else {
		int n = origWidth * origHeight;
		uint16 *dst = (uint16 *)memory;
		uint8 *src = (uint8 *)data;
		tex_memcpy_pal(dst, src, n);
	      }
	    }
	    //*((volatile unsigned int *)(void *)0xa05f8040) = 0x000000;
        }

        void bind(int sampler) {
	  if (opt & OPT_PROXY) return;

	  if (sampler != sDiffuse) {
	    return;
	  }
	  
	  FormatDesc desc = formats[fmt];
	  
	  if (opt & OPT_REPEAT){
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
	  
	  if (width == 320) {
	    m_PvrContext.txr.format |= PVR_TXRFMT_STRIDE;
	  }
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
                ::Vertex *src = vertices;
                Vertex   *dst = vBuffer;

                for (int i = 0; i < vCount; i++) {
                    dst->texCoord = short2(src->texCoord.x, src->texCoord.y);
                    dst->color    = ubyte4(src->light.z, src->light.y, src->light.x, src->light.w); //color;
                    dst->normal   = src->normal;
                    dst->coord    = src->coord;

                    dst++;
                    src++;
                }
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
        LOG("Renderer : %s\n", "PowerVR2");
        LOG("Version  : %s\n", "0.5");

	/*.
        support.maxAniso       = 0;
        support.maxVectors     = 0;
        support.shaderBinary   = false;
        support.VAO            = false;
        support.depthTexture   = false;
        support.shadowSampler  = false;
        support.discardFrame   = false;
        support.texNPOT        = false;
        support.texRG          = false;
        support.texBorder      = false;
        support.colorFloat     = false;
        support.colorHalf      = false;
        support.texFloatLinear = false;
        support.texFloat       = false;
        support.texHalfLinear  = false;
        support.texHalf        = false;
	*/

	support.texMinSize  = 8;

        pvr_base_mem = NULL;

	scaleX = 1.0f;
	scaleY = 1.0f;

        pvr_poly_cxt_txr(&m_PvrContext, PVR_LIST_OP_POLY, PVR_TXRFMT_ARGB1555, 8, 8, 0, 0);

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

    void setClearColor(const vec4 &color) {
	pvr_set_bg_color(color.x, color.y, color.z);
    }

    void setViewport(const short4 &v) {
        float w = v.z * 0.5f;
	float h = v.w * 0.5f;
        float near = 0.0f;
        float far = 1.0f;

	//mat4 matrix;

	clear_matrix();
	save_matrix(&m_Matrix[0].m);
	//matrix.identity();

	m_Matrix[0].e00 = w;
        m_Matrix[0].e11 = -h;
        m_Matrix[0].e22 = (far - near); //(far - near) * 0.5f;
        m_Matrix[0].e23 = near; //(far + near) * 0.5f;
        m_Matrix[0].e03 = v.x + w;
        m_Matrix[0].e13 = v.y + h;

	//load_matrix(&matrix.m);
	//save_matrix(&m_Matrix[0].m);
    }

    void setScissor(const short4 &s) {}

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
      /*
      load_matrix(&mProj.m);
      save_matrix(&m_Matrix[1].m);

      load_matrix(&mView.m);
      save_matrix(&m_Matrix[2].m);
      */

      load_matrix(&mProj.m);
      apply_matrix(&mView.m);
      save_matrix(&m_Matrix[1].m);
    }

    void updateLights(vec4 *lightPos, vec4 *lightColor, int count) {

      ambient = vec3(Core::active.material.y);

      lightsCount = 0;
      for (int i = 0; i < count; i++) {
	if (lightColor[i].w >= 1.0f) {
	  continue;
	}
	LightPVR &light = lights[lightsCount++];
	light.color = lightColor[i].xyz();
	light.pos   = lightPos[i].xyz();
	light.att   = lightColor[i].w * lightColor[i].w;
      }
    }

    uint32 apply_lighting(const Vertex &vertex)
    {
      const uint8 *argb = &vertex.color.x;
      vec3 result = vec3(0.0f);

      //load_matrix(&mModel.m);
      load_matrix(&m_Matrix[3].m);

      vec3 coord  = vec3(vertex.coord.x);
      //vec3 normal = vec3(vertex.normal.x).normal();
      vec3 normal = vec3(vertex.normal.x);
      vec3f_normalize(normal.x, normal.y, normal.z);

      for (int i = 0; i < lightsCount; i++) {
	LightPVR &light = lights[i];
	//vec3 invL = (mModelInv * vec4(lightV, 1.0f)).xyz();
	//vec3f_normalize(invL.x, invL.y, invL.z);

	vec3 vpos;
	mat_trans_normal3_nomod(coord.x, coord.y, coord.z, vpos.x, vpos.y, vpos.z);
	//vec3 vpos = (mModel * vec4(coord, 1.0)).xyz();
	//vec3 vpos = (mModelInv * vec4(coord, 1.0f)).xyz();
	vec3 lightV = light.pos - vpos;
	vec3f_normalize(lightV.x, lightV.y, lightV.z);

	float diff;
	vec3f_dot(normal.x, normal.y, normal.z, lightV.x, lightV.y, lightV.z, diff);
	float d;
	vec3f_length(lightV.x, lightV.y, lightV.z, d);
	float att = 1.0f / (1.0f + light.att * d * d);
	vec3 diffuse = light.color * max(0.0f, diff);
	result += ambient * att + diffuse * att;
      }

      load_matrix(&m_Matrix[2].m);

      uint8 R, G, B, A;
      R = clamp(int(result.x * argb[0]), 0, 255);
      G = clamp(int(result.y * argb[1]), 0, 255);
      B = clamp(int(result.z * argb[2]), 0, 255);
      A = argb[3];
      /* RGB might be wrong */
      R = (R==0)? 100: R;
      G = (G==0)? 100: G;
      B = (B==0)? 100: B;

      return A << 24 | R << 16 | G << 8 | B;
    }

    void DrawMesh(const Index *indices, const Vertex *vertices, int iCount) {

        int vIndex = 0;
        bool isTriangle = false;

        int vcount = 0;
        polygon_vertex_t vertex_buffer[4] __attribute__((aligned(32)));

	pvr_poly_hdr_t hdr;

        ReCompileHeader(&hdr);

	primitive_header((void *)&hdr, 32);

	//*((volatile unsigned int *)(void *)0xa05f8040) = 0x000000;
	//vertex_buffer[0].flags = vertex_buffer[1].flags = vertex_buffer[2].flags = vertex_buffer[3].flags = PVR_CMD_VERTEX;
	vertex_buffer[0].offset.color = vertex_buffer[1].offset.color = vertex_buffer[2].offset.color = vertex_buffer[3].offset.color = 0;

        for (int i = 0; i < iCount; i++) {
            const Index  index   = indices[i];
            const Vertex &vertex = vertices[index];

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

            const int16 *in = &vertex.coord.x;
            float *out = &vertex_buffer[vcount].x;

            mat_trans_single3_nomod(in[0], in[1], in[2], out[0], out[1], out[2]);
            //printf("[%d] %f %f %f %s\n",vIndex,out[0],out[1],out[2],isTriangle?"Tris":"Quad");

            vertex_buffer[vcount].u = vertex.texCoord.x * 1.0f / 32767.0f;
            vertex_buffer[vcount].v = vertex.texCoord.y * 1.0f / 32767.0f;

            if (lightsCount) {
	      vertex_buffer[vcount].base.color = apply_lighting(vertex);
            } else {
	      vertex_buffer[vcount].base.color = vertex.color.value;
            }

            vcount++;

            if (isTriangle && vIndex == 3) {
	        /*
		  0 ---- 1
                  |     /
		  |    /
		  |   /
		  |  /
		  | /
		  2
	        */
                int face_list[] = {0, 1, 2};
                //*((volatile unsigned int *)(void *)0xa05f8040) = 0xff0000;
		primitive_nclip_polygon((pvr_vertex_t*)vertex_buffer, face_list, sizeof(face_list)/sizeof(int));

                vcount = 0;
                vIndex = 0;
            } else if (vIndex == 6) {
	        /*
		  0 ---- 1
		  |     /|
		  |    / |
		  |   /  |
		  |  /   |
		  | /    |
		  2 -----3
		*/
                int face_list[] = {4, 0, 1, 3, 2};
                //*((volatile unsigned int *)(void *)0xa05f8040) = 0x00ff00;
                primitive_nclip_polygon_strip((pvr_vertex_t*)vertex_buffer, face_list, sizeof(face_list)/sizeof(int));

                vcount = 0;
                vIndex = 0;
            }
        }
    }

    void DIP(Mesh *mesh, const MeshRange &range) {
        mat4 m = mModel;
	/*
        float scale[4][4] = {0.0f};

        scale[0][0] = 2.0f;
        scale[1][1] = 1.0f;
        scale[2][2] = 1.0f;
        scale[3][3] = 1.0f;
      */

      /*
        load_matrix(&m_Matrix[0].m);
        apply_matrix(&m_Matrix[1].m);
        apply_matrix(&mModel.m);
      */

	//load_matrix(&mModel.m);
	load_matrix(&m.m);
	save_matrix(&m_Matrix[3].m);

        load_matrix(&m_Matrix[0].m);
        apply_matrix(&m_Matrix[1].m);
        //apply_matrix(&m.m);
	apply_matrix(&m_Matrix[3].m);

	save_matrix(&m_Matrix[2].m);

	DrawMesh(mesh->iBuffer + range.iStart, mesh->vBuffer + range.vStart, range.iCount);

    }

    vec4 copyPixel(int x, int y) {
        return vec4(0.0f); // TODO: read from framebuffer
    }
}

#endif
