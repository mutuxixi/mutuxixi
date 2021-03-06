#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_,SUFFIX))
{
    cpu.eip = MEM_R(REG(R_ESP), R_SS) - 1;
    REG(R_ESP) += DATA_BYTE;
    print_asm("ret");
    return 1;
}

make_helper(concat(ret_i_,SUFFIX))
{
    uint32_t val = instr_fetch(eip + 1, 2);
    cpu.eip = MEM_R(REG(R_ESP), R_SS) - 1;
    if(DATA_BYTE == 2)  cpu.eip &= 0xffff;
    REG(R_ESP) += DATA_BYTE + val;
    print_asm("ret 0x%x",val);
    return 1;
}

#include "cpu/exec/template-end.h"