#include <asm.h>

int main(int argc, char ** argv) {
	if (argc < 3)
		return 1;
	int a;
	int b;
	sscanf(argv[1], "%d", &b);
	sscanf(argv[2], "%d", &a);
	int x = lin_con(a, -b);
	printf("index = %d, index increment = %d, rounds = %d\n", b, a, x);
	return 0;
}
