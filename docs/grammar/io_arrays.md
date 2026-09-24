# Entrada, saída e arrays

As regras da Issue 5 são integradas à gramática de comandos e expressões.

## Regras BNF

```bnf
comando_io ::= comando_print
             | leitura_scanner ";"
             | chamada_metodo ";"

comando_print ::= "System" "." "out" "." "println" "(" argumento_opcional ")" ";"
                | "System" "." "out" "." "print" "(" expressao ")" ";"

argumento_opcional ::= vazio | expressao
leitura_scanner ::= ID "." metodo_scanner "(" ")"
metodo_scanner ::= "next" | "nextLine" | "nextInt" | "nextDouble"
                 | "nextFloat" | "nextLong" | "nextShort"
                 | "nextByte" | "nextBoolean"

instanciacao ::= "new" "Scanner" "(" "System" "." "in" ")"
               | "new" tipo_base "[" expressao "]"
acesso_array ::= ID "[" expressao "]"

chamada_metodo ::= ID "(" argumentos_opcionais ")"
                 | ID "." ID "(" argumentos_opcionais ")"
```

`acesso_array` é uma expressão, portanto pode aparecer como valor e como alvo
de atribuição. A instanciação é aceita como valor de uma atribuição, por
exemplo `leitor = new Scanner(System.in);`.

Chamadas qualificadas usam exatamente um `TOKEN_PONTO` seguido de um `ID`.
Os nomes reservados de `Scanner` e de seus métodos continuam sendo tokens
próprios; assim, um identificador comum não é confundido com uma chamada
especial e o ponto não é tratado como parte do identificador.
