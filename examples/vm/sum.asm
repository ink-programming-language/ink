; MachineModule text example: compute and check 20 + 22 in local stack slots.
.module "sum_example"

.section .data
.global @message size 3 readonly
    .bytes "Hi\00"
.endglobal

.global @scratch size 16 align 8
    .zero 4
.endglobal

.section .text
.function @sum
    .stackslot size 4 align 4 ; input: offset 16
    .stackslot size 4 align 4 ; result: offset 20
    .frame base 16
entry:
    ; Assume the call frame saved BP and PC, with SP initially equal to BP + 16.
    add64rri sp, sp, 8
    store32i [bp + 16], 20
    add32mmi [bp + 20], [bp + 16], 22 ; result = 42
    jne32mib [bp + 20], 42, failure
    jmp done
failure:
    store32i [bp + 20], 0
done:
    ret
.endfunction
