#include "cpu/exec/template-start.h"

#define instr shl

static void do_execute () {
	DATA_TYPE src = op_src->val;
	DATA_TYPE dest = op_dest->val;

	uint8_t count = src & 0x1f;
	dest <<= count;
	OPERAND_W(op_dest, dest);

	/* TODO: Update EFLAGS. */
	//panic("please implement me");

	DATA_TYPE result = dest;
    int len = (DATA_BYTE << 3) - 1;

    cpu.CF = 0;
	cpu.OF = 0;
    cpu.ZF = !result;
    cpu.SF = result >> len;

    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.PF = !(result&1);

	print_asm_template2();
}

make_instr_helper(rm_1)
make_instr_helper(rm_cl)
make_instr_helper(rm_imm)

#include "cpu/exec/template-end.h"
