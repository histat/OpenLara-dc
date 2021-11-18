    AREA |C$$code|, CODE, READONLY
|x$codeseg|

    IMPORT cameraViewOffset
    IMPORT gVertices
    EXPORT unpackRoom_asm

unpackRoom_asm

vertices RN r0
vCount   RN r1
vx0      RN r1
vy0      RN r2
vz0      RN r3
vx1      RN r4
vy1      RN r5
vz1      RN r6
vx2      RN vx0
vy2      RN vy0
vz2      RN vz0
vx3      RN vx1
vy3      RN vy1
vz3      RN vz1
n0       RN vz1
n1       RN r7
maskH    RN r8
maskV    RN r9
res      RN r12
last     RN lr

        stmfd sp!, {r4-r9, lr}
        ldr res, =gVertices
        add last, vertices, vCount, lsl #1 ; last = vertices + vCount * 2
        mov maskH, #0x7C00
        mov maskV, #0x3F00

loop    ldmia vertices!, {n0, n1} ; load four encoded vertices
        cmp vertices, last

        ; n0 = z1:5, y1:6, x1:5, z0:5, y0:6, x0:5
        ; n0 = z3:5, y3:6, x3:5, z2:5, y2:6, x2:5

    ; 1st vertex
        and vx0, maskH, n0, lsl #10     ; decode x0
        and vy0, maskV, n0, lsl #3      ; decode y0
        and vz0, maskH, n0, lsr #1      ; decode z0

    ; 2nd vertex
        and vx1, maskH, n0, lsr #6      ; decode x0
        and vy1, maskV, n0, lsr #13     ; decode y0
        and vz1, maskH, n0, lsr #17     ; decode z0

    ; store
        stmia res!, {vx0, vy0, vz0, vx1, vy1, vz1}

    ; 3rd vertex
        and vx2, maskH, n1, lsl #10     ; decode x0
        and vy2, maskV, n1, lsl #3      ; decode y0
        and vz2, maskH, n1, lsr #1      ; decode z0

    ; 4th vertex
        and vx3, maskH, n1, lsr #6      ; decode x0
        and vy3, maskV, n1, lsr #13     ; decode y0
        and vz3, maskH, n1, lsr #17     ; decode z0

    ; store
        stmia res!, {vx2, vy2, vz2, vx3, vy3, vz3}

        blt loop

        ldmfd sp!, {r4-r9, pc}
    END
