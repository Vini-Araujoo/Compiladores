#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int contador;
char *prefixo;
void incrementar() {
	contador = (contador + 1);
}

int main(void) {
	contador = 10;
	prefixo = "Resultado";
	incrementar();
	incrementar();
	printf("%s\n", prefixo);
	printf("%d\n", contador);
	return 0;
}

