# Validação semântica de arrays e chamadas de métodos — Issue 10.3

## Objetivo

Definir as verificações semânticas para acesso indexado a arrays e chamadas de métodos declarados na classe. A implementação de apoio está em `src/semantica.h` e `src/semantica.c`.

## Acesso a arrays

A função `validarAcessoArray(nome, indice, linha)` realiza duas verificações:

1. Consulta `buscarSimbolo` e exige que o identificador exista e esteja registrado com `TIPO_ARRAY`.
2. Chama `inferirTipoExpressao(indice)` e exige igualdade estrita com `TIPO_INT`. Não são aceitos `float`, `double`, `String` ou conversões numéricas implícitas.

A função retorna `1` quando válido e `0` quando inválido. Em caso de erro, escreve mensagem em `stderr` com a linha de origem.

## Assinaturas e chamadas

A tabela de assinaturas é mantida separadamente da tabela de variáveis. `registrarAssinaturaMetodo` armazena nome, tipo de retorno e tipos dos parâmetros. `obterAssinaturaMetodo` recupera a assinatura pelo nome.

`validarChamadaMetodo(nome, argumentos, linha)`:

1. Rejeita métodos não registrados.
2. Compara a quantidade de filhos do nó de argumentos com a quantidade de parâmetros.
3. Infere o tipo de cada argumento e compara com o parâmetro correspondente. São aceitas correspondências exatas e conversões numéricas widening de acordo com a hierarquia implementada em `tiposCompativeis`.
4. Retorna `1` em caso de sucesso ou `0` e uma mensagem com linha em caso de erro.

O nó de argumentos esperado é uma lista (`NoAST`) cujos filhos são as expressões na ordem da chamada. Uma chamada sem argumentos pode fornecer `NULL`.

## Integração com a AST e dependências

A inferência é conservadora: reconhece nós literais, identificadores registrados e wrappers `NO_EXPRESSAO` com um filho. Para nós ainda não representados pela AST, retorna `TIPO_VOID` (tipo desconhecido), evitando aceitar silenciosamente uma expressão como índice inteiro ou argumento compatível.

A integração completa no parser depende de duas capacidades que ainda não existem nesta revisão do projeto: (a) a gramática atual descarta a maioria dos nós de expressão e não cria nós de chamada de método; (b) as regras atuais de declaração local não registram variáveis nem reconhecem declaradores `tipo[] nome`. Por isso, os arquivos Java de `tests/semantic/` especificam os cenários de aceitação da issue, mas ainda não podem ser considerados executáveis pela gramática atual. O módulo fornece a API semântica para ser chamado quando as issues de AST/declarações e inferência disponibilizarem esses nós e símbolos.

## Ciclo de vida

Chamar `inicializarTabelaMetodos()` no início da análise de uma classe/unidade e `liberarTabelaMetodos()` ao final. Registrar cada assinatura antes de validar chamadas que a referenciem. A tabela de símbolos deve estar inicializada e conter os identificadores de arrays antes de chamar `validarAcessoArray`.

## Casos de teste previstos

- `valido_indice_inteiro_array.java`: índice `int` aceito.
- `invalido_indice_array_float.java`: índice `float` rejeitado.
- `valido_chamada_metodo_com_tipos_corretos.java`: aridade e tipos compatíveis aceitos.
- `invalido_chamada_metodo_menos_argumentos.java`: chamada com aridade inferior rejeitada.
