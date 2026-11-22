; Windows x64 VM example: native adapters write "hello, world\r\n" to stdout.
.module "hello_world_windows"
.declare function @GetStdHandle
.declare function @WriteFile

.section .data
.global @message size 14 readonly
    .bytes "hello, world\0D\0A"
.endglobal

.section .text
.function @main external
entry:
    call @hello
    ret
.endfunction

.function @hello
    .stackslot size 4 align 4 ; written: offset 16
    .frame base 16
entry:
    ; Reserve written and four padding bytes to keep native calls eight-byte aligned.
    add64rri sp, sp, 8
    store32i [bp + 16], 0

    ; GetStdHandle(u32) -> HANDLE: an eight-byte return slot, then one argument slot.
    add64rri sp, sp, 16
    store32i [sp - 8], 0xFFFFFFF5
    call @GetStdHandle

    ; WriteFile has an eight-byte return slot followed by five eight-byte argument slots.
    add64rri sp, sp, 48
    load64 [sp - 40], [sp - 64]
    store64i [sp - 32], @message
    store32i [sp - 24], 14
    add64mrr [sp - 16], bp + 16, zero
    store64i [sp - 8], 0
    call @WriteFile

    add64rri sp, sp, -72
    ret
.endfunction
