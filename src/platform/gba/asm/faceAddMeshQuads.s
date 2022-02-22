#include "common_asm.inc"

polys       .req r0
count       .req r1
vp          .req r2
vg0         .req r3
vg1         .req r4
vg2         .req r5
vg3         .req r6
flags       .req r7
vp0         .req r8
vp1         .req r9
vp2         .req r10
vp3         .req r11
ot          .req r12
face        .req lr

vx0         .req vg0
vy0         .req vg1
vx1         .req vg2
vy1         .req vg3
vx2         .req vg2
vy2         .req vg2

vz0         .req vg0
vz1         .req vg1
vz2         .req vg2
vz3         .req vg3
depth       .req vg0

tmp         .req flags
vertices    .req vg2
next        .req vp0

SP_SIZE = 4

.global faceAddMeshQuads_asm
faceAddMeshQuads_asm:
    stmfd sp!, {r4-r11, lr}

    ldr vp, =gVerticesBase
    ldr vp, [vp]

    ldr vertices, =gVertices
    lsr vertices, #3
    stmfd sp!, {vertices}

    ldr face, =gFacesBase
    ldr face, [face]

    ldr ot, =gOT

    add polys, #2   // skip flags

.loop:
    ldrb vp0, [polys], #1
    ldrb vp1, [polys], #1
    ldrb vp2, [polys], #1
    ldrb vp3, [polys], #3   // + flags

    add vp0, vp, vp0, lsl #3
    add vp1, vp, vp1, lsl #3
    add vp2, vp, vp2, lsl #3
    add vp3, vp, vp3, lsl #3

    CCW .skip

    // fetch clip flags
    ldrb vg0, [vp0, #VERTEX_CLIP]
    ldrb vg1, [vp1, #VERTEX_CLIP]
    ldrb vg2, [vp2, #VERTEX_CLIP]
    ldrb vg3, [vp3, #VERTEX_CLIP]

    // check clipping
    and tmp, vg0, vg1
    and tmp, vg2
    ands tmp, vg3
    bne .skip

    // mark if should be clipped by viewport
    orr tmp, vg0, vg1
    orr tmp, vg2
    orr tmp, vg3
    tst tmp, #(CLIP_MASK_VP >> 8)
    ldrh flags, [polys, #-8]
    orrne flags, #FACE_CLIPPED

    // vz0 = AVG_Z4 (depth)
    ldrh vz0, [vp0, #VERTEX_Z]
    ldrh vz1, [vp1, #VERTEX_Z]
    ldrh vz2, [vp2, #VERTEX_Z]
    ldrh vz3, [vp3, #VERTEX_Z]
    add depth, vz0, vz1
    add depth, vz2
    add depth, vz3
    lsr depth, #(2 + OT_SHIFT)

    // faceAdd
    ldr vertices, [sp]
    rsb vp0, vertices, vp0, lsr #3
    rsb vp1, vertices, vp1, lsr #3
    rsb vp2, vertices, vp2, lsr #3
    rsb vp3, vertices, vp3, lsr #3

    orr vp1, vp0, vp1, lsl #16
    orr vp3, vp2, vp3, lsl #16

    ldr next, [ot, depth, lsl #2]
    str face, [ot, depth, lsl #2]
    stmia face!, {flags, next, vp1, vp3}
.skip:
    subs count, #1
    bne .loop

    ldr tmp, =gFacesBase
    str face, [tmp]

    add sp, #SP_SIZE
    ldmfd sp!, {r4-r11, pc}
