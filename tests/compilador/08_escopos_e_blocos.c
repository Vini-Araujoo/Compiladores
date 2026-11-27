#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	int x = 10;
	printf("%d\n", x);
	{
		int y = 20;
		printf("%d\n", y);
	}
	printf("%d\n", x);
	return 0;
}

