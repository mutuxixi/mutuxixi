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

hwaddr_t page_translate(lnaddr_t addr){
	if(cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1){
		uint32_t dir = addr >> 22;
		uint32_t page = (addr >> 12) & 0x3ff;
		uint32_t offset = addr & 0xfff;

		//read TLB
		int i = read_TLB(addr);
		if (i != -1) return (tlb[i].page << 12) + offset;

		// get dir position
		uint32_t dir_start = cpu.cr3.page_directory_base;
		uint32_t dir_pos = (dir_start << 12) + (dir << 2);
		PAGE_INFO dir_content;
		dir_content.val = hwaddr_read(dir_pos,4);
		Assert(dir_content.p == 1,"Dir Cannot Be Used!");

		// get page position
		uint32_t page_start = dir_content.addr;
		uint32_t page_pos = (page_start << 12) + (page << 2);
		PAGE_INFO page_content;
		page_content.val = hwaddr_read(page_pos,4);
		Assert(page_content.p == 1,"Page Cannot Be Used!");

		// get hwaddr
		uint32_t addr_start = page_content.addr;
		hwaddr_t hwaddr = (addr_start << 12) + offset;
		write_TLB(addr,hwaddr);
		return hwaddr;
	}
	else return addr;
}

hwaddr_t cmd_page_translate(lnaddr_t addr){
	if(cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1){
		uint32_t dir = addr >> 22;
		uint32_t page = (addr >> 12) & 0x3ff;
		uint32_t offset = addr & 0xfff;

		// get dir position
		uint32_t dir_start = cpu.cr3.page_directory_base;
		uint32_t dir_pos = (dir_start << 12) + (dir << 2);
		PAGE_INFO dir_content;
		dir_content.val = hwaddr_read(dir_pos,4);
		if(dir_content.p == 0){
			printf("Dir Cannot Be Used!\n");
			return 0;
		}

		// get page position
		uint32_t page_start = dir_content.addr;
		uint32_t page_pos = (page_start << 12) + (page << 2);
		PAGE_INFO page_content;
		page_content.val = hwaddr_read(page_pos,4);
		if(page_content.p == 0){
			printf("Page Cannot Be Used!\n");
			return 0;
		}

		// get hwaddr
		uint32_t addr_start = page_content.addr;
		hwaddr_t hwaddr = (addr_start << 12) + offset;
		return hwaddr;
	}else return addr;
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
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	uint32_t now_offset = addr & 0xfff;
	if(now_offset + len -1 > 0xfff){
		// Assert(0,"Cross the page boundary!");
		size_t l = 0xfff - now_offset + 1;
		uint32_t addr_r = lnaddr_read(addr,l);
		uint32_t addr_l = lnaddr_read(addr + l,len - l);
		uint32_t val = (addr_l << (l << 3)) | addr_r;
		return val;
	}
	else{
		hwaddr_t hwaddr = page_translate(addr);
		return hwaddr_read(hwaddr,len);
	}
	// return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	uint32_t now_offset = addr & 0xfff;
	if(now_offset + len - 1 > 0xfff){
		// Assert(0,"Cross the page boundary!");
		size_t l = 0xfff - now_offset + 1;
		lnaddr_write(addr,l,data & ((1 << (l << 3)) - 1));
		lnaddr_write(addr + l,len - l,data >> (l << 3));
	}
	else{
		hwaddr_t hwaddr = page_translate(addr);
		hwaddr_write(hwaddr, len, data);
	}
	// hwaddr_write(addr, len, data);
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
