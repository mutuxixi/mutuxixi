#include "cpu/exec/template-start.h"

#define instr stos

make_helper(concat(stos_n_,SUFFIX))
{
    MEM_W(reg_l(R_EDI), REG(R_EAX));
    if(!cpu.DF)
        reg_l(R_EDI) += DATA_BYTE;
    else
        reg_l(R_EDI) -= DATA_BYTE;
    print_asm("stos%s",str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"