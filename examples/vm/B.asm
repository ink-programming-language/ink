.module "B"
.native_data @buffer_b, @host_buffer

.section .data
.global counter, 8, 8 external
    .byte 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal
.global scratch, 8, 8
    .byte 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.endglobal

.section .text
.function helper external
entry:
    load64 [bp], [@scratch]
    load64 [bp + 16], [@buffer_b + 8]
    ret
.endfunction
