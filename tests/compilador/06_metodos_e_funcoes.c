#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int somar(int a, int b) {
	return (a + b);
}

int fatorial(int n) {
	if ((n <= 1)) {
		return 1;
	}
	int temp = fatorial((n - 1));
	return (n * temp);
}

int main(void) {
	int s = somar(15, 25);
	int fat = fatorial(5);
	printf("%d\n", s);
	printf("%d\n", fat);
	return 0;
}

