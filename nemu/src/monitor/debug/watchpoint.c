#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp(bool *success) {
	WP *ans;
	if(free_ == NULL) {
		printf("Not enough free wp!\n");
		*success = false;
		return NULL;
	}
	ans = free_;
	free_ = free_ -> next;
	ans -> next = head;
	head = ans;
	return ans;
}

void free_wp(int ID) {
	WP *p, *q;
	p = head;
	q = NULL;
	while(p != NULL && p -> NO != ID) {
		q = p;
		p = p -> next;
	}
	if(p == NULL) {
		printf("Error! Not found the wp!\n");
		return ;
	}

	if(head -> NO == ID)
		head = head -> next;
	else
		q -> next = p -> next;

	if(free_ == NULL) {
		free_ = p;
		p -> next = NULL;
	}
	else {
		q = free_;
		while(q -> next != NULL)
			q = q -> next;
		q -> next = p;
		p -> next = NULL;
	}
	printf("Watchpoint NO.%d is deleted\n",ID);
}

bool check_watchpoint() {
	uint32_t Cmp;
	bool judge = 1;
	WP *p;
	for(p = head;p != NULL;p = p-> next) {
		Cmp = expr(p -> str, &judge);
		if(Cmp != p -> value) {
			printf("Stop at watchpoint NO.%d\n",p -> NO);
			p -> value = Cmp;
			return 1;
		}
	}
	return 0;
}

void info_watchpoint() {
	if(head == NULL) {
		printf("There is no watchpoint!\n");
		return ;
	}
	printf("NO.\tVALUE\t\t  EXPR\n");
	WP *p;
	for(p = head;p != NULL;p = p -> next)
		printf("%d\t0x%-16x%s\n",p -> NO,p -> value,p -> str);
}

