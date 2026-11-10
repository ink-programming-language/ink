.module "A_B"
.native_data @buffer_a, @host_buffer

.section .data
.global .__ink_link_local_0, 8, 8
    .byte 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal
.global counter, 8, 8 external
    .byte 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal
.global .__ink_link_local_1, 8, 8
    .byte 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal

.section .text
.function main external
entry:
    load64 [bp], [@.__ink_link_local_0]
    add64mmi [@counter], [@counter], 0x1
    store64i [bp + 8], @helper
    load64 [bp + 16], [@buffer_a + 8]
    jmp done
done:
    ret
.endfunction
.function helper external
entry:
    load64 [bp], [@.__ink_link_local_1]
    load64 [bp + 16], [@buffer_a + 8]
    ret
.endfunction
