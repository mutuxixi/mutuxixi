#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ = 257,
	negative = 258, NEQ = 259,
	h_num = 260, d_num = 261,
	AND = 262, OR = 263,
	INV = 264, DEREF = 265, REG = 266,
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
        {" +",  NOTYPE},                                // spaces
	{"0x[a-z,0-9]+", h_num},			// hexadecimal-number
        {"[0-9]+", d_num},                              // decimal-number
	{"\\$[a-z]+", REG},				// REG
	{"==", EQ},						// equal
	{"!=", NEQ},						// not equal
	{"&&", AND},						// AND
	{"\\|\\|", OR},						// OR
	{"!", INV},						// INV
        {"\\+", '+'},                                   // plus
	{"\\*", '*'},					// multiply
	{"-", '-'},					// minus
	{"/", '/'},					// divide
	{"\\(", '('},					// left barket
	{"\\)", ')'},					// right barket

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				if(substr_len > 32) {
					/* make sure substr_len <= 32 */
					printf("Waring! token's len exceeds 32\n");
					return false;
				}
				switch(rules[i].token_type) {
					case NOTYPE :
						break;
					case '+': {
						tokens[nr_token++].type = rules[i].token_type;
						break;
					}
					case '-': {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '*': {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '/': {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '(': {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case ')': {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
                                        case EQ: {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
                                        case NEQ: {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
                                        case AND: {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
                                        case OR: {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
                                        case INV: {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
                                        case REG: {
                                                tokens[nr_token].type = rules[i].token_type;
						int j;
						if(substr_len == 4) {
							for(j = 0;j < 8; ++j) {
								if(strcmp(regsl[j], substr_start + 1) == 0) {
									tokens[nr_token].str[0] = 1 + '0';
									tokens[nr_token].str[1] = j + '0';
									break;
								}
							}
							if(strcmp("eip", substr_start + 1) == 0) {
								tokens[nr_token].str[0] = 4 + '0';
								break;
							}
						}
						else {
							for(j = 0;j < 8; ++j) {
								if(strcmp(regsw[j], substr_start + 1) == 0) {
									tokens[nr_token].str[0] = 2 + '0';
									tokens[nr_token].str[1] = j + '0';
									break;
								}
								if(strcmp(regsb[j], substr_start + 1) == 0) {
                                                                        tokens[nr_token].str[0] = 3 + '0';
                                                                        tokens[nr_token].str[1] = j + '0';
                                                                        break;
                                                                }
							}
						}
						++nr_token;
						break;
                                        }
                                        case h_num: {
                                                tokens[nr_token].type = rules[i].token_type;
                                                int j;
                                                for(j = 0;j < 32; ++j)
                                                        tokens[nr_token].str[j] = '0';
                                                for(j = 0;j < substr_len - 2; ++j)
                                                        tokens[nr_token].str[31 - j] = substr_start[substr_len - 1 - j];
                                                for(;j < 32; ++j)
                                                        tokens[nr_token].str[31 - j] = '0';
                                                ++nr_token;
                                                break;
                                        }
					case d_num: {
						tokens[nr_token].type = rules[i].token_type;
						int j;
						for(j = 0;j < 32; ++j)
							tokens[nr_token].str[j] = '0';
						for(j = 0;j < substr_len; ++j)
							tokens[nr_token].str[31 - j] = substr_start[substr_len - 1 - j];
						for(;j < 32; ++j)
							tokens[nr_token].str[31 - j] = '0';
						++nr_token;
						break;
					}
					default: panic("please implement me");
				}
				break;
			}
		}
		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	return true; 
}

bool check_parentheses(int p, int q) {
	int i, cnt = 0, judge = 0;
	for(i = p;i <= q; ++i) {
		if(tokens[i].type == '(')
			++cnt;
		if(tokens[i].type == ')')
			--cnt;
		if(cnt == 0)
			++judge;
		if(cnt < 0) {
			printf("Bad expression!\n");
			assert(0);
		}
	}
	if(cnt != 0) {
		printf("Bad expression!\n");
		assert(0);
	}
	if(tokens[p].type == '(' && tokens[q].type == ')' && judge == 1)
		return 1;
	return 0;
}

long long eval(int p,int q) {
	if(p > q) {
		printf("Bad expression!\n");
		assert(0);
	}
	else if(p == q) {
		/* Single token, it should be a number */
		long long temp = 0;
		int i;
		if(tokens[p].type == h_num) {
		/* Hexadecimal-number */
			for(i = 0;i < 32; ++i) {
				if(tokens[p].str[i] <= '9' && tokens[p].str[i] >= '0')
					temp = temp * 16 + (long long)(tokens[p].str[i] - '0');
				else
					temp = temp * 16 + (long long)(tokens[p].str[i] - 'a' + 10);
			}
		}
		else if(tokens[p].type == d_num) {
		/* Decimal-number */
			for(i = 0;i < 32; ++i)
				temp = temp * 10 + (long long)(tokens[p].str[i] - '0');
		}
		else if(tokens[p].type == REG) {
		/* REG */
			int CMP = tokens[p].str[0] - '0', Index = tokens[p].str[1] - '0';
                        printf("CMP: %d Index: %d\n",CMP,Index);
			switch (CMP) {
				case 1 : temp =  reg_l(Index);break;
                                case 2 : temp =  reg_w(Index);break;
                                case 3 : temp =  reg_b(Index);break;
                                case 4 : temp =  cpu.eip;break;
				default : assert(0);
			}
			printf("CMP: %d Index: %d\n",CMP,Index);
		}
		return temp;
	}
	else if(check_parentheses(p,q) == true) {
		/* Just throw away parentheses */
		return eval(p + 1,q - 1);
	}
	else {
		int i,op = -1,tmp[32],cnt = 0;
		for(i = p;i <= q; ++i) {
			if(tokens[i].type == '(') {
				int j,judge = 1;
				for(j = i + 1;j <= q; ++j) {
					if(tokens[j].type == '(')
						++judge;
					if(tokens[j].type == ')')
						--judge;
					if(!judge) {
						i = j + 1;
						break;
					}
				}
			}
			if(tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == '*' || tokens[i].type == '/')
				tmp[cnt++]=i;
			if(tokens[i].type == EQ || tokens[i].type == NEQ || tokens[i].type == AND || tokens[i].type == OR)
				op = i;
		}
		if(op == -1 && cnt) {
			op = tmp[cnt - 1];
			if(tokens[op].type != '+' && tokens[op].type != '-') {
				for(i = cnt-2;i >= 0; --i) {
					if(tokens[tmp[i]].type == '+' || tokens[tmp[i]].type == '-') {
						op = tmp[i];
						break;
					}
				}
			}
		}
		if(op == -1) {
			if(tokens[p].type == negative)
				return -1 * eval(p + 1,q);
			else if(tokens[p].type == INV)
				return !eval(p + 1,q);
			else if(tokens[p].type == DEREF)
				return swaddr_read(eval(p + 1,q),4);
		}
		long long val1, val2;
		val1 = eval(p, op - 1);
		val2 = eval(op + 1, q);
		switch (tokens[op].type) {
			case EQ  : return val1 == val2;
			case NEQ : return val1 != val2;
			case AND : return val1 && val2;
			case OR  : return val1 || val2;
			case '+' : return val1 + val2;
       	                case '-' : return val1 - val2;
       	                case '*' : return val1 * val2;
                        case '/' : return val1 / val2;
			default : assert(0);
		}
	}
}

void Init_minus() {
	int i;
	if(tokens[0].type == '-')
                tokens[0].type = negative;
	for(i = 1;i < nr_token; ++i) {
		if(tokens[i].type == '-') {
			if(tokens[i - 1].type != d_num && tokens[i - 1].type != h_num && tokens[i - 1].type != REG && tokens[i - 1].type != ')')
				tokens[i].type = negative;
		}
	}
}

void Init_multiply() {
        int i;
        if(tokens[0].type == '*')
                tokens[0].type = DEREF;
        for(i = 1;i < nr_token; ++i) {
                if(tokens[i].type == '*') {
                        if(tokens[i - 1].type != d_num && tokens[i - 1].type != h_num && tokens[i - 1].type != REG && tokens[i - 1].type == ')')
                                tokens[i].type = DEREF;
                }
        }
}


uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	Init_minus();
	Init_multiply();
	printf("nr_token: %d\n",nr_token);
	return eval(0, nr_token - 1);

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

