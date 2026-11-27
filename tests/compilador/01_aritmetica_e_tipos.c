#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
	int a = 10;
	int b = 20;
	int c = (((a + (b * 2)) - (15 / 3)) + (8 % 3));
	double pi = 3.14;
	double raio = 2.0;
	double area = ((pi * raio) * raio);
	bool ativo = true;
	char letra = 'X';
	char *saudacao = "Compiladores 2026";
	printf("%d\n", c);
	printf("%lf\n", area);
	printf("%s\n", (ativo) ? "true" : "false");
	printf("%c\n", letra);
	printf("%s\n", saudacao);
	return 0;
}

