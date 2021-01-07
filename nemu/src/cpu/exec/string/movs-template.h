#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_n_,SUFFIX))
{
    MEM_W(reg_l(R_EDI), swaddr_read(reg_l(R_ESI), DATA_BYTE, R_DS), R_ES);
    if(!cpu.DF)
    {
        reg_l(R_EDI) += DATA_BYTE;
        reg_l(R_ESI) += DATA_BYTE;
    }
    else
    {
        reg_l(R_EDI) -= DATA_BYTE;
        reg_l(R_ESI) -= DATA_BYTE;
    }
    print_asm("movs");
    return 1;
}

#include "cpu/exec/template-end.h"