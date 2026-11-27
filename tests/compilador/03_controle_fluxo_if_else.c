#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	int x = 15;
	int y = 20;
	if (((x > 10) && (y < 30))) {
		printf("%d\n", 1);
	} else {
		printf("%d\n", 0);
	}
	if (((x == 100) || (!((y > 50))))) {
		printf("%d\n", 2);
	}
	if ((x < 5)) {
		printf("%d\n", 10);
	} else if ((x == 15)) {
		printf("%d\n", 20);
	} else {
		printf("%d\n", 30);
	}
	return 0;
}

