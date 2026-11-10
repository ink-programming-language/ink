.module "A"
.declare global @counter
.declare function @helper
.native_data @buffer_a, @host_buffer

.section .data
.global scratch, 8, 8
    .byte 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal

.section .text
.function main external
entry:
    load64 [bp], [@scratch]
    add64mmi [@counter], [@counter], 0x1
    store64i [bp + 8], @helper
    load64 [bp + 16], [@buffer_a + 8]
    jmp done
done:
    ret
.endfunction
