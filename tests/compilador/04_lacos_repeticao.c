#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	int i = 0;
	while ((i < 3)) {
		printf("%d\n", i);
		i++;
	}
	int j = 10;
	do {
		printf("%d\n", j);
		j--;
	} while ((j > 8));
	for (int k = 1; (k <= 3); (k++)) {
		printf("%d\n", k);
	}
	return 0;
}

