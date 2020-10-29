#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX) )
{
    int len = concat(decode_i_, SUFFIX) (eip+1);
    reg_l (R_ESP) -= DATA_BYTE;
    printf("len: %d cpu.eip + len: 0x%x\n",len,cpu.eip+len);
    MEM_W (reg_l (R_ESP) , cpu.eip + len);
    DATA_TYPE_S displacement = op_src -> val;
    print_asm("call 0x%x", cpu.eip + 1 + len + displacement);
    cpu.eip += displacement;
    return len + 1;
}

#include "cpu/exec/template-end.h"