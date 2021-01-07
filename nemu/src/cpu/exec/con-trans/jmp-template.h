#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute()
{
    DATA_TYPE_S displacement = op_src->val;
    if(op_src->type == OP_TYPE_IMM)
        cpu.eip += displacement;
    else
        cpu.eip = displacement - concat(decode_rm_,SUFFIX)(cpu.eip + 1) - 1;
    print_asm_template1();
}

make_instr_helper(i);
make_instr_helper(rm);

#if DATA_BYTE == 4
extern SREG_INFO *sreg_info;
SREG_INFO tmp;
make_helper(ljmp){
    sreg_info = &tmp;

    uint32_t op1 = instr_fetch(cpu.eip + 1,4);
    uint16_t op2 = instr_fetch(cpu.eip + 1 + 4,2);
    cpu.eip = op1;
    cpu.cs.selector = op2;

    uint16_t idx = cpu.cs.selector >> 3;
	lnaddr_t chart_addr = cpu.gdtr.base + (idx << 3);
	sreg_info->part1 = lnaddr_read(chart_addr, 4);
	sreg_info->part2 = lnaddr_read(chart_addr + 4, 4);

	cpu.cs.base = (uint32_t)sreg_info->base1 + (sreg_info->base2 << 16) + (sreg_info->base3 << 24);
	cpu.cs.limit = (uint32_t)sreg_info->limit1 + (sreg_info->limit2 << 16) + (0xfff << 24);
	if (sreg_info->g == 1) {
		cpu.cs.limit <<= 12;
	}
    print_asm("ljmp 0x%x 0x%x",op2,op1);
    return 0;
}
#endif

#include "cpu/exec/template-end.h"