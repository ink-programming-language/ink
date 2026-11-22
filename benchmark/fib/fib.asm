; Recursive Fibonacci: fib(n) = n when n < 2, otherwise fib(n - 1) + fib(n - 2).
; A host can change fib_input before execute() and read fib_result afterwards.
.module "fib"

.section .data
.global @fib_input size 8 align 8 external
    .i64 30
.endglobal
.global @fib_result size 8 align 8 external
.endglobal

.section .text
.function @main external
entry:
    ; The call area contains an eight-byte return slot followed by n.
    add64rri sp, sp, 16
    load64 [sp - 8], [@fib_input]
    call @fib
    load64 [@fib_result], [sp - 16]
    add64rri sp, sp, -16
    ret
.endfunction

.function @fib
    .stackslot size 8 align 8 ; fib(n - 1): offset 16
    .frame base 16
entry:
    jltu64mib [bp - 8], 2, base

    ; Reserve the local result and a reusable sixteen-byte recursive call area.
    add64rri sp, sp, 24
    add64mmi [bp + 32], [bp - 8], -1
    call @fib
    load64 [bp + 16], [bp + 24]

    add64mmi [bp + 32], [bp - 8], -2
    call @fib

    add64mmm [bp - 16], [bp + 16], [bp + 24]
    ret
base:
    load64 [bp - 16], [bp - 8]
    ret
.endfunction
