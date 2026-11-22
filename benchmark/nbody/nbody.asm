; Scalar FP64 five-body simulation. Positions/velocities/mass use seven double words per body.
; Initial conditions and algorithm: Computer Language Benchmarks Game, Christoph Bauer.
; https://benchmarksgame-team.pages.debian.net/benchmarksgame/program/nbody-gcc-1.html
; See LICENSE.benchmarksgame for the upstream redistribution notice.
; main resets all bodies before each run; the host supplies nbody_steps and reads the exported results.
.module "nbody"

.section .data
.global @nbody_steps size 8 align 8 external
    .i64 1000
.endglobal
.global @nbody_initial_energy size 8 align 8 external
.endglobal
.global @nbody_final_energy size 8 align 8 external
.endglobal
.global @nbody_bodies size 280 align 8 external
.endglobal
.global @nbody_energy size 8 align 8
.endglobal
.global @loop_i size 8 align 8
.endglobal
.global @loop_j size 8 align 8
.endglobal
.global @iteration size 8 align 8
.endglobal
.global @constants size 24 align 8 readonly
    .f64 0.01, 0.5 ; dt, half
    .i64 0x4043BD3CC9BE45DE ; solar mass
.endglobal
.global @initial_bodies size 280 align 8 readonly
    ; sun: x, y, z, vx, vy, vz, mass
    .zero 48
    .i64 0x4043BD3CC9BE45DE
    ; jupiter: x, y, z, vx, vy, vz, mass
    .i64 0x40135DA0343CD92C, 0xBFF290ABC01FDB7C, 0xBFBA86F96C25EBF0
    .i64 0x3FE367069B93CCBC, 0x40067EF2F57D949B, 0xBF99D2D79A5A0715
    .i64 0x3FA34C95D9AB33D8
    ; saturn: x, y, z, vx, vy, vz, mass
    .i64 0x4020AFCDC332CA67, 0x40107FCB31DE01B0, 0xBFD9D353E1EB467C
    .i64 0xBFF02C21B8879442, 0x3FFD35E9BF1F8F13, 0x3F813C485F1123B4
    .i64 0x3F871D490D07C637
    ; uranus: x, y, z, vx, vy, vz, mass
    .i64 0x4029C9EACEA7D9CF, 0xC02E38E8D626667E, 0xBFCC9557BE257DA0
    .i64 0x3FF1531CA9911BEF, 0x3FEBCC7F3E54BBC5, 0xBF862F6BFAF23E7C
    .i64 0x3F5C3DD29CF41EB3
    ; neptune: x, y, z, vx, vy, vz, mass
    .i64 0x402EC267A905572A, 0xC039EB5833C8A220, 0x3FC6F1F393ABE540
    .i64 0x3FEF54B61659BC4A, 0x3FE307C631C4FBA3, 0xBFA1CB88587665F6
    .i64 0x3F60A8F3531799AC
.endglobal

.section .text
.function @main external
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

.function @offset_momentum
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

.function @advance
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

.function @energy
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
