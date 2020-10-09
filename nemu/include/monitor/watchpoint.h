#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	/* TODO: Add more members if necessary */
	char *str;
	uint32_t value;
} WP;

WP *new_wp(bool *success);
void free_wp(int ID);
bool check_watchpoint();
void info_watchpoint();

#endif
