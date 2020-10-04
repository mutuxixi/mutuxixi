#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	uint32_t num = 0;
	if(arg == NULL)
	{
		/* if N isn't give,  make one step */
		cpu_exec(1);
	}
	else
	{
		sscanf(arg,"%u",&num);
		cpu_exec(num);
	}
	return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL, " ");
	char op;
	if(arg == NULL)
		printf("Error! You need to input like this: info r/w\n");
	else
	{
		sscanf(arg,"%c",&op);
		if(op == 'r')
		{
			printf("eip\t0x%08x\t%d\n",cpu.eip,cpu.eip);
			int i;
			for(i = 0;i < 8; ++i)
				printf("%s\t0x%08x\t%d\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
			puts("");
			for(i = 0;i < 8; ++i)
				printf("%s\t0x%-8x\t%d\n",regsw[i],cpu.gpr[i]._16,cpu.gpr[i]._16);
			puts("");
			for(i = 0;i < 4; ++i)
				printf("%s\t0x%-8x\t%d\n",regsb[i],cpu.gpr[i]._8[0],cpu.gpr[i]._8[0]);
			puts("");
			for(i = 4;i < 8; ++i)
				printf("%s\t0x%-8x\t%d\n",regsb[i],cpu.gpr[i-4]._8[1],cpu.gpr[i-4]._8[1]);
			puts("");
			
		}
	}
	return 0;
}

static int cmd_x(char *args) {
	char *arg1 = strtok(NULL, " "),*arg2 = strtok(NULL, " ");
	uint32_t num,pos;
	if(arg1 == NULL || arg2 == NULL)
		printf("Error! You need to input like this: x 10 0x100000\n");
	else
	{
		sscanf(arg1,"%u",&num);
		sscanf(arg2,"%x",&pos);
		int i;
		for(i = 0;i < num; ++i)
		{
			printf("0x%08x:\t%08x\n",pos,swaddr_read(pos,4));
			pos += 4;
		}
	}
	return 0;
}

static int cmd_p(char *args) {
	if(args == NULL) {
		printf("Error! You need to input like this: p 1+2\n");
		return 0;
	}
	bool judge = 1;
	uint32_t ANS = expr(args,&judge);
	if(judge)
		printf("%u\n",ANS);
	return 0;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Make one step", cmd_si },
	{ "info", "Get information from reg", cmd_info },
	{ "x", "Scanf memory", cmd_x},
	{ "p", "Calculation", cmd_p},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
