#include "trap.h"

int A[20],b,c,d;

int main()
{
    A[0] = 0;
	A[1] = 1;
	A[2] = 2;
	A[3] = 3;
	A[4] = 4;

	b = A[3];
	A[5] = b;
	
    c = 11;
    A[11] = c;

    d = A[4];

	nemu_assert(A[0] == 0);
	nemu_assert(A[1] == 1);
	nemu_assert(A[2] == 2);
	nemu_assert(A[3] == 3);
	nemu_assert(A[4] == 4);
	nemu_assert(b == 3);
	nemu_assert(A[5] == 3);
    nemu_assert(A[11] == 11);
    nemu_assert(d == 4);
return 0;
}