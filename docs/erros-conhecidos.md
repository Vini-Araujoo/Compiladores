# Erros Encontrados e Resoluções

Este documento registra os erros, bugs e inconsistências identificados durante as etapas de análise léxica, sintática, semântica e geração de código (codegen), bem como as soluções aplicadas.

---

## 1. Testes da Issue 3 após a integração
Depois da integração com a Issue 4, os testes válidos da Issue 3 que usavam comandos diretamente na raiz do arquivo deixaram de passar. 
* **Causa:** A gramática passou a exigir obrigatoriamente que comandos vivam dentro de métodos contidos em classes (`declaracao_classe` -> `declaracao_metodo` -> `bloco`).
* **Resolução:** Os testes foram adaptados para a estrutura canônica da linguagem Java (envolvidos em classe e método `main`).

---

## 2. Inferência de Tipos de Literais (`String`, `char`, `float`)
Durante a compilação de atribuições simples como `nome = "texto";`, o analisador semântico emitia:
> *Erro semantico: tipos incompativeis na atribuicao para 'nome'. Esperado 'String', recebido 'desconhecido'.*

* **Causa:** Na função `inferirTipoExpressao` (`src/semantic/inferencia.c`), o `switch (expr->tipo_no)` tratava apenas `NO_LITERAL_INT`, `NO_LITERAL_DOUBLE` e `NO_LITERAL_BOOL`. Literais de texto (`NO_LITERAL_STRING`), caracteres (`NO_LITERAL_CHAR`) e ponto flutuante simples (`NO_LITERAL_FLOAT`) caíam no `default:`, sendo inferidos como `TIPO_INDEFINIDO` ("desconhecido").
* **Resolução:** Adicionados os casos explícitos para `NO_LITERAL_STRING` (`TIPO_STRING`), `NO_LITERAL_CHAR` (`TIPO_CHAR`) e `NO_LITERAL_FLOAT` (`TIPO_FLOAT`) tanto no cálculo do tipo quanto na anotação do nó da AST.

---

## 3. Descarte de Nós de Chamada de Método no Parser (`parser.y`)
Ao analisar chamadas de função/método como `somar(a, b);` ou `int x = somar(2, 3);`, o nome da função e seus argumentos eram descartados da árvore sintática (AST).
* **Causa:** A regra `chamada_metodo` em `parser/parser.y` continha um stub temporário remanescente da Issue 9.1 (`{ free($1); $$ = cria_no_expressao_generica(NULL, NULL); }`), desalocando o identificador e gerando um nó genérico vazio sem filhos.
* **Resolução:** Substituição do stub pela construção canônica do nó `NO_CHAMADA_METODO`, armazenando o identificador em `valor.valor_str` e conectando cada argumento da lista como filho da árvore sintática (`adicionarFilho`), além de atualizar `imprimirAST` e `liberarAST` em `src/ast/ast.c`.

---

## 4. Declaração e Instanciação de `Scanner`
Historicamente, declarações com instanciação de `Scanner` como:
```java
Scanner sc = new Scanner(System.in);
```
produziam erro de sintaxe.
* **Causa:** A gramática inicial não integrava a instanciação de objetos especiais de entrada como expressão válida de inicializador de variáveis.
* **Resolução:** Criação dos nós dedicados `NO_INSTANCIACAO_SCANNER` e `NO_LEITURA_SCANNER` no parser, tratados no semântico como tipo `TIPO_SCANNER` e gerados em C como chamadas de `scanf` associadas pelo módulo `codegen_io.c`.

---

## 5. Incompatibilidade de Linkagem em Testes Unitários Isolados
Ao rodar os testes unitários de declarações (`teste_decl_unitario.c`) e blocos (`teste_atrib_bloco_unitario.c`), ocorriam erros de símbolos indefinidos (`undefined reference to emitirPrint / emitirPrintln / emitirIf`).
* **Causa:** O orquestrador central `codegen_main.c` chama funções de módulos que não estavam sendo compilados juntos nos alvos de teste isolados dos outros integrantes.
* **Resolução:** Inclusão de stubs fracos (`__attribute__((weak))`) em `codegen_main.c` e `codegen_fluxo.c`, permitindo que testes unitários continuem linkando de forma isolada, enquanto no compilador completo as implementações reais fortes assumem o controle.

---

## 6. Concatenação de String com Primitivos via `+` (Aritmética de Ponteiros em C)
Ao executar um código Java com concatenação entre `String` e número inteiro, como:
```java
String nome = "asdasd";
int num = 2;
System.out.println(nome + num);
```
O programa executável em C imprimiu inesperadamente:
```text
dasd
```
* **Causa:** Em Java, o operador `+` envolvendo ao menos uma `String` realiza sobrecarga para **concatenação textual** (esperava-se `"asdasd2"`). No entanto, o gerador de expressões aritméticas traduz diretamente `+` para o operador `+` de C (`(nome + num)`). Em C, somar um ponteiro (`char *nome`) com um inteiro (`int num`) é interpretado como **aritmética de ponteiros** (deslocamento de 2 bytes a partir do início da string), fazendo com que o ponteiro pule os dois primeiros caracteres (`'a'`, `'s'`) e aponte para `"dasd"`.
* **Resolução recomendada:** Para suportar concatenação textual no C gerado, seria necessário alocar um buffer temporário e utilizar funções de formatação como `snprintf` ou `strcat`, ou manter documentado que concatenação heterogênea via operador binário `+` puro é uma limitação da tradução procedural direta.
