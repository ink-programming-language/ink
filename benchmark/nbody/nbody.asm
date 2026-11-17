; Scalar FP64 five-body simulation. Positions/velocities/mass use seven double words per body.
; Initial conditions and algorithm: Computer Language Benchmarks Game, Christoph Bauer.
; https://benchmarksgame-team.pages.debian.net/benchmarksgame/program/nbody-gcc-1.html
; See LICENSE.benchmarksgame for the upstream redistribution notice.
; main resets all bodies before each run; the host supplies nbody_steps and reads the exported results.
.module "nbody"

.section .data
.global nbody_steps, 8, 8 external
    .byte 0xE8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal
.global nbody_initial_energy, 8, 8 external
.endglobal
.global nbody_final_energy, 8, 8 external
.endglobal
.global nbody_bodies, 280, 8 external
.endglobal
.global nbody_energy, 8, 8
.endglobal
.global loop_i, 8, 8
.endglobal
.global loop_j, 8, 8
.endglobal
.global iteration, 8, 8
.endglobal
.global constants, 24, 8 readonly
    .byte 0x7B, 0x14, 0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F ; dt
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F ; half
    .byte 0xDE, 0x45, 0xBE, 0xC9, 0x3C, 0xBD, 0x43, 0x40 ; solar mass
.endglobal
.global initial_bodies, 280, 8 readonly
    ; sun: x, y, z, vx, vy, vz, mass
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    .byte 0xDE, 0x45, 0xBE, 0xC9, 0x3C, 0xBD, 0x43, 0x40
    ; jupiter: x, y, z, vx, vy, vz, mass
    .byte 0x2C, 0xD9, 0x3C, 0x34, 0xA0, 0x5D, 0x13, 0x40
    .byte 0x7C, 0xDB, 0x1F, 0xC0, 0xAB, 0x90, 0xF2, 0xBF
    .byte 0xF0, 0xEB, 0x25, 0x6C, 0xF9, 0x86, 0xBA, 0xBF
    .byte 0xBC, 0xCC, 0x93, 0x9B, 0x06, 0x67, 0xE3, 0x3F
    .byte 0x9B, 0x94, 0x7D, 0xF5, 0xF2, 0x7E, 0x06, 0x40
    .byte 0x15, 0x07, 0x5A, 0x9A, 0xD7, 0xD2, 0x99, 0xBF
    .byte 0xD8, 0x33, 0xAB, 0xD9, 0x95, 0x4C, 0xA3, 0x3F
    ; saturn: x, y, z, vx, vy, vz, mass
    .byte 0x67, 0xCA, 0x32, 0xC3, 0xCD, 0xAF, 0x20, 0x40
    .byte 0xB0, 0x01, 0xDE, 0x31, 0xCB, 0x7F, 0x10, 0x40
    .byte 0x7C, 0x46, 0xEB, 0xE1, 0x53, 0xD3, 0xD9, 0xBF
    .byte 0x42, 0x94, 0x87, 0xB8, 0x21, 0x2C, 0xF0, 0xBF
    .byte 0x13, 0x8F, 0x1F, 0xBF, 0xE9, 0x35, 0xFD, 0x3F
    .byte 0xB4, 0x23, 0x11, 0x5F, 0x48, 0x3C, 0x81, 0x3F
    .byte 0x37, 0xC6, 0x07, 0x0D, 0x49, 0x1D, 0x87, 0x3F
    ; uranus: x, y, z, vx, vy, vz, mass
    .byte 0xCF, 0xD9, 0xA7, 0xCE, 0xEA, 0xC9, 0x29, 0x40
    .byte 0x7E, 0x66, 0x26, 0xD6, 0xE8, 0x38, 0x2E, 0xC0
    .byte 0xA0, 0x7D, 0x25, 0xBE, 0x57, 0x95, 0xCC, 0xBF
    .byte 0xEF, 0x1B, 0x91, 0xA9, 0x1C, 0x53, 0xF1, 0x3F
    .byte 0xC5, 0xBB, 0x54, 0x3E, 0x7F, 0xCC, 0xEB, 0x3F
    .byte 0x7C, 0x3E, 0xF2, 0xFA, 0x6B, 0x2F, 0x86, 0xBF
    .byte 0xB3, 0x1E, 0xF4, 0x9C, 0xD2, 0x3D, 0x5C, 0x3F
    ; neptune: x, y, z, vx, vy, vz, mass
    .byte 0x2A, 0x57, 0x05, 0xA9, 0x67, 0xC2, 0x2E, 0x40
    .byte 0x20, 0xA2, 0xC8, 0x33, 0x58, 0xEB, 0x39, 0xC0
    .byte 0x40, 0xE5, 0xAB, 0x93, 0xF3, 0xF1, 0xC6, 0x3F
    .byte 0x4A, 0xBC, 0x59, 0x16, 0xB6, 0x54, 0xEF, 0x3F
    .byte 0xA3, 0xFB, 0xC4, 0x31, 0xC6, 0x07, 0xE3, 0x3F
    .byte 0xF6, 0x65, 0x76, 0x58, 0x88, 0xCB, 0xA1, 0xBF
    .byte 0xAC, 0x99, 0x17, 0x53, 0xF3, 0xA8, 0x60, 0x3F
.endglobal

.section .text
.function main external
entry:
    add64rri r0, zero, @nbody_bodies
    add64rri r1, zero, @initial_bodies
    store64i [@loop_i], 0
reset_bodies:
    load64 [r0], [r1]
    add64rri r0, r0, 8
    add64rri r1, r1, 8
    add64mmi [@loop_i], [@loop_i], 1
    jltu64mib [@loop_i], 35, reset_bodies
    call @offset_momentum
    call @energy
    load64 [@nbody_initial_energy], [@nbody_energy]
    store64i [@iteration], 0
    jltu64mmb [@iteration], [@nbody_steps], step
    jmp finished
step:
    call @advance
    add64mmi [@iteration], [@iteration], 1
    jltu64mmb [@iteration], [@nbody_steps], step
finished:
    call @energy
    load64 [@nbody_final_energy], [@nbody_energy]
    ret
.endfunction

.function offset_momentum
entry:
    add64rri r0, zero, 0
    add64rri r1, zero, 0
    add64rri r2, zero, 0
    add64rri r8, zero, @nbody_bodies
    store64i [@loop_i], 0
body:
    add64rrm r3, zero, [r8 + 48]
    add64rrm r4, zero, [r8 + 24]
    fmul64rrr r4, r4, r3
    fadd64rrr r0, r0, r4
    add64rrm r4, zero, [r8 + 32]
    fmul64rrr r4, r4, r3
    fadd64rrr r1, r1, r4
    add64rrm r4, zero, [r8 + 40]
    fmul64rrr r4, r4, r3
    fadd64rrr r2, r2, r4
    add64rri r8, r8, 56
    add64mmi [@loop_i], [@loop_i], 1
    jltu64mib [@loop_i], 5, body
    add64rrm r3, zero, [@constants + 16]
    fsub64rrr r0, zero, r0
    fdiv64rrr r0, r0, r3
    add64mrr [@nbody_bodies + 24], r0, zero
    fsub64rrr r1, zero, r1
    fdiv64rrr r1, r1, r3
    add64mrr [@nbody_bodies + 32], r1, zero
    fsub64rrr r2, zero, r2
    fdiv64rrr r2, r2, r3
    add64mrr [@nbody_bodies + 40], r2, zero
    ret
.endfunction

.function advance
entry:
    add64rri r8, zero, @nbody_bodies
    store64i [@loop_i], 0
outer:
    add64rri r9, r8, 56
    add64mmi [@loop_j], [@loop_i], 1
pair:
    add64rrm r0, zero, [r8]
    add64rrm r3, zero, [r9]
    fsub64rrr r0, r0, r3
    add64rrm r1, zero, [r8 + 8]
    add64rrm r3, zero, [r9 + 8]
    fsub64rrr r1, r1, r3
    add64rrm r2, zero, [r8 + 16]
    add64rrm r3, zero, [r9 + 16]
    fsub64rrr r2, r2, r3
    fmul64rrr r3, r0, r0
    fmul64rrr r4, r1, r1
    fadd64rrr r3, r3, r4
    fmul64rrr r4, r2, r2
    fadd64rrr r3, r3, r4
    fsqrt64rr r3, r3
    fmul64rrr r4, r3, r3
    fmul64rrr r3, r4, r3
    add64rrm r4, zero, [@constants]
    fdiv64rrr r3, r4, r3
    add64rrm r4, zero, [r9 + 48]
    fmul64rrr r5, r0, r4
    fmul64rrr r5, r5, r3
    add64rrm r6, zero, [r8 + 24]
    fsub64rrr r6, r6, r5
    add64mrr [r8 + 24], r6, zero
    add64rrm r4, zero, [r8 + 48]
    fmul64rrr r5, r0, r4
    fmul64rrr r5, r5, r3
    add64rrm r6, zero, [r9 + 24]
    fadd64rrr r6, r6, r5
    add64mrr [r9 + 24], r6, zero
    add64rrm r4, zero, [r9 + 48]
    fmul64rrr r5, r1, r4
    fmul64rrr r5, r5, r3
    add64rrm r6, zero, [r8 + 32]
    fsub64rrr r6, r6, r5
    add64mrr [r8 + 32], r6, zero
    add64rrm r4, zero, [r8 + 48]
    fmul64rrr r5, r1, r4
    fmul64rrr r5, r5, r3
    add64rrm r6, zero, [r9 + 32]
    fadd64rrr r6, r6, r5
    add64mrr [r9 + 32], r6, zero
    add64rrm r4, zero, [r9 + 48]
    fmul64rrr r5, r2, r4
    fmul64rrr r5, r5, r3
    add64rrm r6, zero, [r8 + 40]
    fsub64rrr r6, r6, r5
    add64mrr [r8 + 40], r6, zero
    add64rrm r4, zero, [r8 + 48]
    fmul64rrr r5, r2, r4
    fmul64rrr r5, r5, r3
    add64rrm r6, zero, [r9 + 40]
    fadd64rrr r6, r6, r5
    add64mrr [r9 + 40], r6, zero
    add64rri r9, r9, 56
    add64mmi [@loop_j], [@loop_j], 1
    jltu64mib [@loop_j], 5, pair
    add64rri r8, r8, 56
    add64mmi [@loop_i], [@loop_i], 1
    jltu64mib [@loop_i], 4, outer
    add64rri r8, zero, @nbody_bodies
    add64rrm r7, zero, [@constants]
    store64i [@loop_i], 0
position:
    add64rrm r0, zero, [r8]
    add64rrm r1, zero, [r8 + 24]
    fmul64rrr r1, r7, r1
    fadd64rrr r0, r0, r1
    add64mrr [r8], r0, zero
    add64rrm r0, zero, [r8 + 8]
    add64rrm r1, zero, [r8 + 32]
    fmul64rrr r1, r7, r1
    fadd64rrr r0, r0, r1
    add64mrr [r8 + 8], r0, zero
    add64rrm r0, zero, [r8 + 16]
    add64rrm r1, zero, [r8 + 40]
    fmul64rrr r1, r7, r1
    fadd64rrr r0, r0, r1
    add64mrr [r8 + 16], r0, zero
    add64rri r8, r8, 56
    add64mmi [@loop_i], [@loop_i], 1
    jltu64mib [@loop_i], 5, position
    ret
.endfunction

.function energy
entry:
    add64rri r7, zero, 0
    add64rri r8, zero, @nbody_bodies
    store64i [@loop_i], 0
body:
    add64rrm r0, zero, [r8 + 24]
    fmul64rrr r0, r0, r0
    add64rrm r1, zero, [r8 + 32]
    fmul64rrr r1, r1, r1
    fadd64rrr r0, r0, r1
    add64rrm r1, zero, [r8 + 40]
    fmul64rrr r1, r1, r1
    fadd64rrr r0, r0, r1
    add64rrm r1, zero, [@constants + 8]
    add64rrm r2, zero, [r8 + 48]
    fmul64rrr r1, r1, r2
    fmul64rrr r0, r1, r0
    fadd64rrr r7, r7, r0
    je64mib [@loop_i], 4, next_body
    add64rri r9, r8, 56
    add64mmi [@loop_j], [@loop_i], 1
pair:
    add64rrm r0, zero, [r8]
    add64rrm r3, zero, [r9]
    fsub64rrr r0, r0, r3
    add64rrm r1, zero, [r8 + 8]
    add64rrm r3, zero, [r9 + 8]
    fsub64rrr r1, r1, r3
    add64rrm r2, zero, [r8 + 16]
    add64rrm r3, zero, [r9 + 16]
    fsub64rrr r2, r2, r3
    fmul64rrr r3, r0, r0
    fmul64rrr r4, r1, r1
    fadd64rrr r3, r3, r4
    fmul64rrr r4, r2, r2
    fadd64rrr r3, r3, r4
    fsqrt64rr r3, r3
    add64rrm r0, zero, [r8 + 48]
    add64rrm r1, zero, [r9 + 48]
    fmul64rrr r0, r0, r1
    fdiv64rrr r0, r0, r3
    fsub64rrr r7, r7, r0
    add64rri r9, r9, 56
    add64mmi [@loop_j], [@loop_j], 1
    jltu64mib [@loop_j], 5, pair
next_body:
    add64rri r8, r8, 56
    add64mmi [@loop_i], [@loop_i], 1
    jltu64mib [@loop_i], 5, body
    add64mrr [@nbody_energy], r7, zero
    ret
.endfunction
