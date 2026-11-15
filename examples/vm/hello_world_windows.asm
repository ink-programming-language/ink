; Windows x64 VM example: native adapters write "hello, world\r\n" to stdout.
.module "hello_world_windows"
.declare function @GetStdHandle
.declare function @WriteFile

.section .data
.global message, 14, 1 readonly
    .byte 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x0D, 0x0A
.endglobal
.global written, 4, 4
    .byte 0x00, 0x00, 0x00, 0x00
.endglobal

.section .text
.function main external
entry:
    call @hello
    ret
.endfunction

.function hello
entry:
    ; GetStdHandle(u32) -> HANDLE: an eight-byte return slot, then one argument slot.
    add64rri sp, sp, 16
    store32i [sp - 8], 0xFFFFFFF5
    callNative @GetStdHandle

    ; WriteFile has an eight-byte return slot followed by five eight-byte argument slots.
    add64rri sp, sp, 48
    load64 [sp - 40], [sp - 64]
    store64i [sp - 32], @message
    store32i [sp - 24], 14
    store64i [sp - 16], @written
    store64i [sp - 8], 0
    callNative @WriteFile

    add64rri sp, sp, -64
    ret
.endfunction
