#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	int r = multiplicar(4, 5);
	printf("%d\n", r);
	return 0;
}

int multiplicar(int a, int b) {
	return (a * b);
}

