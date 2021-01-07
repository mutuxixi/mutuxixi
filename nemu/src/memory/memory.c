#include "common.h"
#include "burst.h"
#include "memory/cache.h"
#include "nemu.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg){
	if (cpu.cr0.protect_enable == 0)
		return addr;
	return cpu.sreg[sreg].base + addr;
}

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int L1_1st_line = read_cache1(addr);
	uint32_t offset = addr & (Cache_L1_Block_Size - 1);
	uint8_t ret[BURST_LEN << 1];
	if(offset + len > Cache_L1_Block_Size){
		int L1_2nd_line = read_cache1(addr + Cache_L1_Block_Size - offset);
		memcpy(ret,cache1[L1_1st_line].data + offset,Cache_L1_Block_Size - offset);
		memcpy(ret + Cache_L1_Block_Size - offset,cache1[L1_2nd_line].data,len - (Cache_L1_Block_Size - offset));
	}
	else{
		memcpy(ret,cache1[L1_1st_line].data + offset,len);
	}

	int wtf = 0;
	uint32_t ans = unalign_rw(ret + wtf, 4) & (~0u >> ((4 - len) << 3));
	return ans;
	//return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	write_cache1(addr, len, data);
	//dram_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}

