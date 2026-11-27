#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	char *texto = "abcdef";
	int deslocamento = 2;
	printf("%s\n", (texto + deslocamento));
	return 0;
}

