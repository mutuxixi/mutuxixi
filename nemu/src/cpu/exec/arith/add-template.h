#include "cpu/exec/template-start.h"

#define instr add

static void do_execute()
{
    DATA_TYPE result = op_dest->val + op_src->val;
    OPERAND_W(op_dest, result);
    int len = (DATA_BYTE << 3) - 1;
    int tmp1 = (op_dest->val) >> len;
    int tmp2 = (op_src->val) >> len;

    cpu.CF = result < op_dest->val;
    cpu.ZF = !result;
    cpu.SF = result >> len;
    cpu.OF = (tmp1 == tmp2 && tmp2 != cpu.SF);

    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result&1);

    print_asm_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm);
#endif

make_instr_helper(i2a);
make_instr_helper(i2rm);
make_instr_helper(r2rm);
make_instr_helper(rm2r);

#include "cpu/exec/template-end.h"