#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"\\*", '*'},					// multiply
	{"[-^0123456789]", '-'},				// minus
	{"/", '/'},					// divide
	{"\\(", '('},					// left barket
	{"\\)", ')'},					// right barket
        {"-?[0-9]+", '0'},                              // number

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
					assert(0);
				}
				switch(rules[i].token_type) {
					case NOTYPE :
						break;
					case '+':
					{
						tokens[nr_token++].type = rules[i].token_type;
						break;
					}
					case '-':
                                        {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '*':
                                        {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '/':
                                        {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '(':
                                        {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case ')':
                                        {
                                                tokens[nr_token++].type = rules[i].token_type;
                                                break;
                                        }
					case '0':
					{
						tokens[nr_token].type = rules[i].token_type;
						int j;
						for(j = 0;j < 32; ++j)
							tokens[nr_token].str[j] = '0';
						for(j = 0;j < substr_len - 1; ++j)
							tokens[nr_token].str[31 - j] = substr_start[substr_len - 1 - j];
						if(substr_start[0] == '-')
							tokens[nr_token].str[0] = '-';
						else {
							tokens[nr_token].str[0] = '+';
							tokens[nr_token].str[31 - j] = substr_start[0];
							++j;
						}
						for(;j < 31; ++j)
							tokens[nr_token].str[31 - j] = '0';
						++nr_token;
						break;
					}
					case '1':
					{
						assert(0);
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
		/* Single token, it should be a num */
		long long temp = 0;
		int i;
		for(i = 1;i < 32; ++i)
			temp = temp * 10 + (long long)(tokens[p].str[i] - '0');
		if(tokens[p].str[0] == '-')
			temp *= -1;
		return temp;
	}
	else if(check_parentheses(p,q) == true) {
		/* just throw away parentheses */
		return eval(p + 1,q - 1);
	}
	else {
		long long val1,val2;
		int op,i,tmp[32],cnt = 0;
		for(i = p;i <= q; ++i) {
			if(tokens[i].type == '(') {
				int j,judge = 1;
				for(j = i + 1;j <= q; ++j) {
					if(tokens[j].type == '(')
						++judge;
					if(tokens[j].type == ')')
						--judge;
					if(!judge) {
						i = j;
						break;
					}
				}
				continue;
			}
			if(tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == '*' || tokens[i].type == '/')
				tmp[cnt++]=i;
		}
		op = tmp[cnt-1];
		for(i = cnt-2;i >= 0; --i) {
			if(tokens[op].type == '+' || tokens[op].type == '-')
				break;
			if(tokens[tmp[i]].type == '+' || tokens[tmp[i]].type == '-') {
				op = tmp[i];
				break;
			}
		}
		val1 = eval(p, op - 1);
		val2 = eval(op + 1, q);
		switch (tokens[op].type) {
			case '+' : return val1 + val2;
                        case '-' : return val1 - val2;
                        case '*' : return val1 * val2;
                        case '/' : return val1 / val2;
			default : assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	long long ans = eval(0, nr_token - 1);
	return ans;
		
	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

