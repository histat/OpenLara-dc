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

//#define COLOR_16

#ifdef COLOR_16
    #define COLOR_FMT_555
    #define CONV_COLOR(r,g,b) (((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3) | (1 << 15))
#else 
    #define COLOR_FMT_888
    #define CONV_COLOR24(r,g,b) ((r << 16) | (g << 8) | b)
    #define CONV_COLOR(r,g,b) ((r << 16) | (g << 8) | b | (255 << 24))
#endif

#define SW_MAX_DIST  (20.0f * 1024.0f)
#define SW_FOG_START (12.0f * 1024.0f)

//#define ENABLE_FOG

namespace GAPI {

    using namespace Core;

    typedef ::Vertex Vertex;

    #ifdef COLOR_16
        typedef uint16 ColorSW;
    #else
        typedef uint32 ColorSW;
    #endif
    //typedef uint16 DepthSW;

    ColorSW *swPalette;
    ColorSW swPaletteColor[256];
    ColorSW swPaletteWater[256];
    ColorSW swPaletteGray[256];
    uint8   swGradient[256];
    Tile8   *curTile;
    ColorSW swPaletteLight[256 * 2];

    #ifdef ENABLE_FOG
    float FogParams;
    #endif

    uint8 ambient;
    int32 lightsCount;

    struct LightSW {
        uint32 intensity;
        vec3   pos;
        float  radius;
    } lights[MAX_LIGHTS], lightsRel[MAX_LIGHTS];

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
	        m_PvrContext.depth.comparison = PVR_DEPTHCMP_GEQUAL;
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

    #define ARGB1555(r,g,b,a)	( (((r)>>3)<<10) | (((g)>>3)<<5) |((b)>>3) | (((a)&0x80)<<8) )
    #define ARGB8888(r,g,b,a)	( ((a) << 24) | ((r)<<16) | ((g)<<8) | (b) )
    #define LUMINANCE(a)	((((a)>>4)*0x111)|0xf0e0)

    void upload_vram(uint8 *out, const uint8 *in, uint32 w, uint32 h) {
	    uint8 *dst = out + 2048;
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

    void tex_memcpy_pal(void *dest, void *src, uint32 cnt) {
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


    struct Texture {
        pvr_ptr_t      memory;
        int        width, height, origWidth, origHeight;
        TexFormat  fmt;
        uint32     opt;

        Texture(int width, int height, int depth, uint32 opt) : memory(0), width(width), height(height), origWidth(width), origHeight(height), fmt(FMT_RGBA), opt(opt) {}

        void init(void *data) {
            ASSERT((opt & OPT_PROXY) == 0);

            opt &= ~(OPT_CUBEMAP | OPT_MIPMAPS);

            if (origWidth == 640) {
                width = 640;
            }

            LOG("%dx%d orig %dx%d fmt=%d \n",width, height, origWidth,origHeight, fmt);

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
            if (memory) {
                pvr_mem_free(memory);
                memory = NULL;
            }
        }

        void generateMipMap() {}

        void update(void *data) {
            ASSERT(data);

            FormatDesc desc = formats[fmt];
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
        }

        void bind(int sampler) {
            Core::active.textures[sampler] = this;

            if (!this || (opt & OPT_PROXY)) return;
            ASSERT(memory);

            curTile = NULL;

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

        typedef struct {
    	    uint16 color[256 * 4];
            Tile8 tile;
	    } vqTex;

        void bindTileIndices(vqTex *tile) {
            curTile = &tile->tile;

            //ColorSW *pal = swPalette;
            ColorSW *pal = swPaletteColor;
            uint16 *color = (uint16 *)tile;

	        for (int i = 0; i < 256; i++) {
                #ifdef COLOR_16
                uint16 rgb16 = pal[i];
                #else
                uint32 c = pal[i];
                uint16 rgb16 = ((c>>16)&0x8000) | ((c>>9)&0x7c00) | ((c>>6)&0x3e0) | ((c>>3) & 0x1f);
                #endif
                color[i*4 + 0] = rgb16;
                color[i*4 + 1] = rgb16;
                color[i*4 + 2] = rgb16;
                color[i*4 + 3] = rgb16;
            }

            m_PvrContext.txr.width = width * 4;
	        m_PvrContext.txr.height = height;
	        m_PvrContext.txr.base = (pvr_ptr_t)tile;
            m_PvrContext.txr.format = PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_NONTWIDDLED | PVR_TXRFMT_VQ_ENABLE;

            if (opt & OPT_NEAREST) {
	            m_PvrContext.txr.filter = PVR_FILTER_NEAREST;
	        } else {
	            m_PvrContext.txr.filter = PVR_FILTER_BILINEAR;
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
                memcpy(vBuffer, vertices, vCount * sizeof(vertices[0]));
            }
        }

        void bind(const MeshRange &range) const {}

        void initNextRange(MeshRange &range, int &aIndex) const {
            range.aIndex = -1;
        }
    };


    int cullMode, blendMode;

    ColorSW *swColor;
    //DepthSW *swDepth;
    short4  swClipRect;

    struct VertexSW {
        float x, y, z, w;
        int32 u, v, l;
    };

    void init() {
        LOG("Vendor   : %s\n", "SEGA");
        LOG("Renderer : %s\n", "PowerVR2DC");
        LOG("Version  : %s\n", "0.1");
        //swDepth = NULL;

        support.texMinSize  = 8;

        pvr_base_mem = NULL;

        pvr_poly_cxt_txr(&m_PvrContext, PVR_LIST_OP_POLY, PVR_TXRFMT_ARGB1555, 8, 8, 0, 0);

        m_PvrContext.gen.specular = 1;
        m_PvrContext.gen.fog_type = PVR_FOG_VERTEX;
        #ifdef ENABLE_FOG
        FogParams = 0.0f;
        #endif
    }

    void deinit() {
        //delete[] swDepth;
    }

    void resize() {
        //delete[] swDepth;
        //swDepth = new DepthSW[Core::width * Core::height];
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
        return true;
    }

    void endFrame() {}

    void resetState() {}

    void bindTarget(Texture *texture, int face) {}

    void discardTarget(bool color, bool depth) {}

    void copyTarget(Texture *dst, int xOffset, int yOffset, int x, int y, int width, int height) {}

    void setVSync(bool enable) {}

    void waitVBlank() {}

    void clear(bool color, bool depth) {
        if (color) {
            //memset(swColor, 0x00, Core::width * Core::height * sizeof(ColorSW));
        }

        if (depth) {
            //memset(swDepth, 0xFF, Core::width * Core::height * sizeof(DepthSW));
        }
    }

    void setClearColor(const vec4 &color) {}

    void setViewport(const short4 &v) {
        mat4 Matrix;

        Matrix.viewport((float)v.x, (float)v.y+v.w, (float)v.z, -(float)v.w, 0.0f, 1.0f);
        //Matrix.viewport(0.0f, (float)Core::height, (float)Core::width, -(float)Core::height, 0.0f, 1.0f);
   	    load_matrix(&Matrix.m);
	    save_matrix(&m_Matrix[0].m);
    }

    void setScissor(const short4 &s) {
        swClipRect.x = s.x;
        swClipRect.y = Core::active.viewport.w - (s.y + s.w);
        swClipRect.z = s.x + s.z;
        swClipRect.w = Core::active.viewport.w - s.y;
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

        load_matrix(&mProj.m);
        apply_matrix(&mView.m);
        save_matrix(&m_Matrix[1].m);
    }

    void updateLights(vec4 *lightPos, vec4 *lightColor, int count) {
        ambient = clamp(int32(active.material.y * 255), 0, 255);

        lightsCount = 0;
        for (int i = 0; i < count; i++) {
            if (lightColor[i].w >= 1.0f) {
                continue;
            }
            LightSW &light = lights[lightsCount++];
            vec4 &c = lightColor[i];
            light.intensity = uint32(((c.x + c.y + c.z) / 3.0f) * 255.0f);
            light.pos    = lightPos[i].xyz();
            light.radius = lightColor[i].w;
        }
    }

    void setFog(const vec4 &params) {
        #ifdef ENABLE_FOG
        FogParams = params.w;

        if (params.w > 0.0f) {
            uint32 fogColor = 0x00000000
                | (uint32(clamp(params.z * 255.0f, 0.0f, 255.0f)) << 0)
                | (uint32(clamp(params.y * 255.0f, 0.0f, 255.0f)) << 8)
                | (uint32(clamp(params.x * 255.0f, 0.0f, 255.0f)) << 16);
            PVR_SET(0x0B4, fogColor);

        } else {
            //PVR_FSET(0x0B4, PVR_PACK_COLOR(0, 0.5, 0.9, 0.9));
            PVR_SET(0x0B4, 0);
        }
        #endif
    }

    void applyLighting(VertexSW &result, const Vertex &vertex, float depth) {
        vec3 coord  = vec3(float(vertex.coord.x), float(vertex.coord.y), float(vertex.coord.z));
        vec3 normal = vec3(float(vertex.normal.x), float(vertex.normal.y), float(vertex.normal.z)).normal();
        float lighting = 0.0f;
        for (int i = 0; i < lightsCount; i++) {
            LightSW &light = lightsRel[i];
            vec3 dir = (light.pos - coord) * light.radius;
            float att = dir.length2();
            float lum = normal.dot(dir / sqrtf(att));
            lighting += (max(0.0f, lum) * max(0.0f, 1.0f - att)) * light.intensity;
        }

        lighting += result.l;

        #ifndef ENABLE_FOG
        depth -= SW_FOG_START;
        if (depth > 0.0f) {
            lighting *= clamp(1.0f - depth / (SW_MAX_DIST - SW_FOG_START), 0.0f, 1.0f);
        }
        #endif

        result.l = (255 - min(255, int32(lighting)));
    }

    bool transform(const Index *indices, const Vertex *vertices, int iStart, int iCount, int vStart) {

        #if 0
        mat4 swMatrix;
        swMatrix.viewport(0.0f, (float)Core::height, (float)Core::width, -(float)Core::height, 0.0f, 1.0f);
        swMatrix = swMatrix * mViewProj * mModel;

        load_matrix(&swMatrix.m);
        #else
        load_matrix(&m_Matrix[0].m);
        apply_matrix(&m_Matrix[1].m);
        apply_matrix(&mModel.m);

        #define vec4f_ftrv(x, y, z, w) { \
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
        #endif

        const bool colored = vertices[vStart + indices[iStart]].color.w == 142;

        if (colored) {
            curTile = (Tile8*)swGradient;
        }

        if (colored) {
            m_PvrContext.txr.enable = PVR_TEXTURE_DISABLE;
        } else {
            m_PvrContext.txr.enable = PVR_TEXTURE_ENABLE;
        }

        pvr_poly_hdr_t hdr;
        ReCompileHeader(&hdr);
        primitive_header((void *)&hdr, 32);

        int vIndex = 0;
        bool isTriangle = false;
        int vcount = 0;
        polygon_vertex_t vertex_buffer[4] __attribute__((aligned(32)));

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

            vec4 c;
            #if 0
            c = swMatrix * vec4(vertex.coord.x, vertex.coord.y, vertex.coord.z, 1.0f);
            #else
            //c.x = vertex.coord.x;
            //c.y = vertex.coord.y;
            //c.w = vertex.coord.z;
            c = vec4(vertex.coord.x, vertex.coord.y, vertex.coord.z, 1.0f);
            vec4f_ftrv(c.x, c.y, c.z, c.w);
            #endif

            #if 0
            if (/*c.w < 0.0f ||*/ c.w > SW_MAX_DIST) { // skip primitive
                if (isTriangle) {
                    i += 3 - vIndex;
                } else {
                    i += 6 - vIndex;
                }
                vIndex = 0;

                vcount = 0;
                continue;
            }
            #endif

            VertexSW result;

            #if 0
            mat_trans_single3_nomod(vertex.coord.x, vertex.coord.y, vertex.coord.z, result.x, result.y, result.z);
            #else
            /*
            c.z = 1 / c.w

            result.x = c.x * c.z;
            result.y = c.y * c.z;
            result.z = c.z * c.z;
            */

            result.x = c.x;
            result.y = c.y;
            result.z = c.z;
            
            #endif

            if (colored) {
                result.u = vertex.color.x;
                result.v = 0;
            } else {
                result.u = vertex.texCoord.x;
                result.v = vertex.texCoord.y;
            }

            result.l = ((vertex.light.x * ambient) >> 8);

            applyLighting(result, vertex, c.w);

            uint32 u = uint32(result.u) & 0xffff;
            uint32 v = uint32(result.v) & 0xffff;

            uint8 tIndex;

            if(curTile) {
                tIndex = curTile->index[(v << 8) + u];
            } else {
                tIndex = 0;
            }

            uint32 argb, oargb;
            packed_color_t tmp;

            int32 k = result.l;
            float lv = float(255 - k); // lighting value in untextured colour
            oargb = 0;

            #ifdef ENABLE_FOG
            if (FogParams > 0.0f) {
                float depth = c.w;

                uint8 fog_a;
                float alpha = clamp(depth * FogParams, 0.0f, 1.0f);
                fog_a = alpha * 255;
                oargb = fog_a << 24;
            }
            #endif

            if (colored) {
                if (tIndex != 0) {
                    #ifdef COLOR_16
                    ColorSW c = swPalette[tIndex];
                    argb = ( ((c&0x8000)?(0xff<<24):0) | ((c&0x7c00)<<9) | ((c&0x3e0)<<6) | ((c& 0x1f)<<3) );
                    #else
                    tmp.color = swPalette[tIndex];
                    #endif
                    //swDepth[x] = z;

                    float R,G,B;
                    R = float(tmp.argb[2]) * 1 / 255.0f;
                    G = float(tmp.argb[1]) * 1 / 255.0f;
                    B = float(tmp.argb[0]) * 1 / 255.0f;
                    #if 0
                    tmp.argb[2] = R * lv * 255.0f;
                    tmp.argb[1] = G * lv * 255.0f;
                    tmp.argb[0] = B * lv * 255.0f;
                    #else
                    tmp.argb[2] = R * lv;
                    tmp.argb[1] = G * lv;
                    tmp.argb[0] = B * lv;
                    #endif

                    argb = tmp.color;
                    
                } else {
                    argb = ARGB8888(vertex.light.x,vertex.light.y,vertex.light.z,vertex.light.w);
                }    
            } else {
                if (tIndex != 0) {
                    if (swPalette == swPaletteWater) {
                        argb = swPaletteLight[k + 256];
                    } else {
                        argb = swPaletteLight[k];
                    }
                    #ifdef COLOR_16
                    argb = ( ((c&0x8000)?(0xff<<24):0) | ((c&0x7c00)<<9) | ((c&0x3e0)<<6) | ((c& 0x1f)<<3) );
                    #endif
                } else {
                    argb = ARGB8888(vertex.light.x,vertex.light.y,vertex.light.z,vertex.light.w);
                }

                if(!curTile) {
                    u = uint32(result.u) * 1 / 32767.0f * 255;
                    v = uint32(result.v) * 1 / 32767.0f * 255;
                }

            }


            vertex_buffer[vcount].x = result.x;
            vertex_buffer[vcount].y = result.y;
            vertex_buffer[vcount].z = result.z;
            vertex_buffer[vcount].u = (u / 255.0f);
            vertex_buffer[vcount].v = (v / 255.0f);
            vertex_buffer[vcount].base.color = argb;
            vertex_buffer[vcount].offset.color = oargb;

            vcount++;

            if (isTriangle && vIndex == 3) {
                int face_list[] = {0, 1, 2};
    	        primitive_nclip_polygon((pvr_vertex_t*)vertex_buffer, face_list, sizeof(face_list)/sizeof(int));
                vcount = 0;

                vIndex = 0;
            } else if (vIndex == 6) {
                int face_list[] = {4, 1, 2, 0, 3};
                primitive_nclip_polygon_strip((pvr_vertex_t*)vertex_buffer, face_list, sizeof(face_list)/sizeof(int));
                vcount = 0;

                vIndex = 0;
            }
        }

        return colored;
    }

    void transformLights() {
        memcpy(lightsRel, lights, sizeof(LightSW) * lightsCount);

        mat4 mModelInv = mModel.inverseOrtho();
        for (int i = 0; i < lightsCount; i++) {
            lightsRel[i].pos = mModelInv * lights[i].pos;
        }
    }

    void DIP(Mesh *mesh, const MeshRange &range) {
        if (curTile == NULL) {
            //uint16 *tex = (uint16*)Core::active.textures[0]->memory; // TODO
            //return;
        }

        transformLights();

        Tile8 *oldTile = curTile;

        bool colored = transform(mesh->iBuffer, mesh->vBuffer, range.iStart, range.iCount, range.vStart);

        curTile = oldTile;
    }

    void initPalette(Color24 *palette, uint8 *lightmap) {
        for (uint32 i = 0; i < 256; i++) {
            const Color24 &p = palette[i];
            swPaletteColor[i] = CONV_COLOR(p.r, p.g, p.b);
            swPaletteWater[i] = CONV_COLOR((uint32(p.r) * 150) >> 8, (uint32(p.g) * 230) >> 8, (uint32(p.b) * 230) >> 8);
            swPaletteGray[i]  = CONV_COLOR((i * 57) >> 8, (i * 29) >> 8, (i * 112) >> 8);
            swGradient[i]     = i;

            if ( i == 0 ) {
              swPaletteColor[i] = 0;
              swPaletteWater[i] = 0;
              swPaletteGray[i]  = 0;
              //swGradient[i]     = 0;
            }
        }
        swPalette  = swPaletteColor;

        for (uint32 i = 0; i < 256; i++) {
            uint8 L = clamp(int(255 - i), 0, 255);

            swPaletteLight[i] = CONV_COLOR(L, L, L);
            swPaletteLight[i + 256] = CONV_COLOR((uint32(L) * 150) >> 8, (uint32(L) * 230) >> 8, (uint32(L) * 230) >> 8);
        }
    }

    void setPalette(ColorSW *palette) {
        swPalette = palette;
    }

    void setShading(bool enabled) {
    }

    vec4 copyPixel(int x, int y) {
        return vec4(0.0f); // TODO: read from framebuffer
    }
}

#endif
