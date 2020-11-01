#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_n_,SUFFIX))
{
    swaddr_t Cmp1 = REG(R_EAX);
    swaddr_t Cmp2 = MEM_R(reg_l(R_EDI));
    DATA_TYPE result = Cmp1 - Cmp2;
    if(!cpu.DF)
        reg_l(R_EDI) += DATA_BYTE;
    else
        reg_l(R_EDI) -= DATA_BYTE;
    int len = (DATA_BYTE << 3) - 1;
    int tmp1 = Cmp1 >> len;
    int tmp2 = Cmp2 >> len;

    cpu.CF = Cmp1 < Cmp2;
    cpu.ZF = !result;
    cpu.SF = result >> len;
    cpu.OF = (tmp1 != tmp2 && tmp2 == cpu.SF);

    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result&1);

    print_asm("scas%s",str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"