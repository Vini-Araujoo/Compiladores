#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	int opcao = 2;
	switch (opcao) {
		case 1:
			printf("%d\n", 100);
			break;
		case 2:
			printf("%d\n", 200);
			break;
		case 3:
			printf("%d\n", 300);
			break;
		default:
			printf("%d\n", 999);
			break;
				}
	return 0;
}

