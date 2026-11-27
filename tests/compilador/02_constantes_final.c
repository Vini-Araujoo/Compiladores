#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	const int MAX = 100;
	const double TAXA = 0.05;
	int valor = 50;
	int total = (valor + MAX);
	printf("%d\n", total);
	printf("%lf\n", TAXA);
	return 0;
}

