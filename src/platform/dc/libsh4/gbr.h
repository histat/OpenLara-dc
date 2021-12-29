#ifndef GBR_H
#define GBR_H

#define GBR_WORD_BASE	(64)
#define GBR_LONG_BASE	(128)
#define GBR_SIZE	(512)


#define GBR_BYTE_OFS(idx)	(idx)
#define GBR_WORD_OFS(idx)	(GBR_WORD_BASE + (idx) * 2)
#define GBR_LONG_OFS(idx)	(GBR_LONG_BASE + (idx) * 4)

#define GBR_ERRNO_PTR                   GBR_LONG_OFS(0)
#define GBR_DBGCON_PTR                  GBR_LONG_OFS(1)
#define GBR_MATRIX_STACK                GBR_LONG_OFS(2)
#define GBR_MATRIX_STACK_MIN            GBR_LONG_OFS(3)
#define GBR_MATRIX_STACK_MAX            GBR_LONG_OFS(4)
#define GBR_POLYGON_COUNT               GBR_LONG_OFS(5)
#define GBR_VERTEX_COUNT                GBR_LONG_OFS(6)
#define GBR_CONTEXT_COUNT               GBR_LONG_OFS(7)
#define GBR_OPAQUE_LIST_PTR             GBR_LONG_OFS(8)
#define GBR_OPAQUE_MOD_LIST_PTR         GBR_LONG_OFS(9)
#define GBR_BLEND_LIST_PTR              GBR_LONG_OFS(10)
#define GBR_BLEND_MOD_LIST_PTR          GBR_LONG_OFS(11)
#define GBR_PUNCH_LIST_PTR              GBR_LONG_OFS(12)
#define GBR_TACHYON_WRITE_DST           GBR_LONG_OFS(13)

#endif
