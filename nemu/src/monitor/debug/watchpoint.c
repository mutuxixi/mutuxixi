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

WP* new_wp() {
	WP *ans;
	if(free_ == NULL) {
		printf("Not enoough free wp!\n");
		assert(0);
	}
	ans = free_;
	free_ = free_ -> next;
	ans -> next = head;
	head = ans;
	return ans;
}

void free_wp(WP* wp) {
	WP *tmp, *last;
	tmp = head;
	last = NULL;
	while(tmp != NULL && tmp -> NO != wp -> NO) {
		last = tmp;
		tmp = tmp -> next;
	}
	if(tmp == NULL) {
		printf("Error! Not found the wp!\n");
		assert(0);
	}

	if(head == wp)
		head = head -> next;
	else
		last -> next = tmp -> next;

	if(free_ == NULL) {
		free_ = wp;
		wp -> next = NULL;
	}
	else {
		tmp = free_;
		while(tmp -> next != NULL)
			tmp = tmp -> next;
		tmp -> next = wp;
		wp -> next = NULL;
	}
}

