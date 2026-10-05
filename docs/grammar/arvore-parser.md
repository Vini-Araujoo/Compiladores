# Arvore da gramatica do parser

## 1. Entrada da analise

O Bison declara `%start programa`. Portanto, a derivacao completa comeca em:

```text
programa
```

Na implementacao atual de `parser/parser.y`, a regra `programa` deriva exclusivamente:

```text
programa
└── declaracao_classe
```

> **Nota:** As alternativas `lista_instrucoes_io` e `%empty` que permitiam comandos soltos e entradas vazias foram comentadas no `parser.y`. A regra `lista_instrucoes_io` ainda existe no arquivo, mas e inalcancavel a partir da raiz (gerando aviso de *nonterminal useless in grammar* no Bison). Toda a entrada valida exige a estrutura formal de uma classe Java.

## 2. Arvore principal

```text
programa
└── declaracao_classe
    ├── class ID { corpo_classe }
    ├── final class ID { corpo_classe }
    ├── public class ID { corpo_classe }
    └── public final class ID { corpo_classe }

    corpo_classe
    ├── vazio
    └── corpo_classe membro_classe
        ├── declaracao_variavel
        │   └── modificadores tipo lista_declaracoes ;
        │       ├── modificadores
        │       │   ├── vazio
        │       │   └── modificadores modificador
        │       │       ├── public
        │       │       ├── private
        │       │       ├── protected
        │       │       ├── static
        │       │       └── final
        │       ├── tipo
        │       │   ├── double
        │       │   ├── int
        │       │   ├── String
        │       │   ├── long
        │       │   ├── boolean
        │       │   ├── short
        │       │   ├── char
        │       │   ├── float
        │       │   └── Scanner
        │       └── lista_declaracoes
        │           ├── item_declaracao
        │           │   ├── ID
        │           │   └── ID = expressao
        │           └── lista_declaracoes, item_declaracao
        │
        └── declaracao_metodo
            ├── modificadores tipo ID ( lista_parametros ) bloco
            └── modificadores void ID ( lista_parametros ) bloco
                ├── lista_parametros
                │   ├── vazio
                │   └── parametros_com_virgula
                │       ├── parametro
                │       │   ├── tipo ID
                │       │   └── tipo [ ] ID
                │       └── parametros_com_virgula, parametro
                └── bloco
                    └── { lista_comandos }
                        ├── vazio
                        └── lista_comandos comando
                            ├── declaracao_local
                            │   └── modificador_variavel tipo lista_declaracoes ;
                            │       ├── modificador_variavel: vazio | final
                            │       └── lista_declaracoes
                            ├── atribuicao
                            ├── comando_io
                            ├── condicional
                            ├── repeticao
                            ├── comando_salto
                            └── bloco
```

A recursao de `corpo_classe` e `lista_comandos` representa listas de tamanho zero ou maior. A recursao de `lista_declaracoes`, `parametros_com_virgula` e `lista_argumentos` representa listas separadas por virgula.

## 3. Galho de comandos

Todos os comandos abaixo sao alcancaveis por `lista_comandos`, que por sua vez e alcancavel pelo corpo de um metodo:

```text
comando
├── declaracao_local
├── atribuicao
│   ├── ID = expressao ;
│   ├── acesso_array = expressao ;
│   ├── ++ ID ;
│   ├── -- ID ;
│   ├── ID ++ ;
│   ├── ID -- ;
│   ├── ID += expressao ;
│   └── ID -= expressao ;
├── comando_io
├── condicional
├── repeticao
├── comando_salto
│   ├── break ;
│   └── comando_return
│       ├── return ;
│       └── return expressao ;
└── bloco
```

### 3.1 Entrada e saida

```text
comando_io
├── comando_print
│   ├── System.out.println ( argumento_opcional ) ;
│   │   ├── vazio
│   │   └── expressao
│   └── System.out.print ( expressao ) ;
└── expressao ;
```

### 3.2 Condicionais

```text
condicional
├── comando_if
│   ├── if ( condicao ) comando
│   └── if ( condicao ) comando else comando
└── comando_switch
    └── switch ( expressao ) { casos_switch default_switch }
        ├── casos_switch
        │   ├── vazio
        │   └── casos_switch case_switch
        │       └── case expressao : lista_comandos
        └── default_switch
            ├── vazio
            └── default : lista_comandos
```

`condicao` e um alias sintatico para `expressao`. As diretivas `LOWER_THAN_ELSE` e `TOKEN_ELSE` resolvem a associacao do `else` ao `if` mais proximo.

### 3.3 Repeticoes

```text
repeticao
├── comando_for
│   └── for ( atribuicao_for condicao_for incremento_for ) bloco
│       ├── atribuicao_for
│       │   ├── ;
│       │   ├── ID = expressao ;
│       │   └── declaracao_local
│       ├── condicao_for
│       │   ├── ;
│       │   └── condicao ;
│       └── incremento_for
│           ├── vazio
│           ├── ID = expressao
│           ├── ID ++ | ID --
│           └── ++ ID | -- ID
├── comando_do_while
│   └── do bloco while ( condicao ) ;
└── comando_while
    └── while ( condicao ) bloco
```

## 4. Arvore de expressoes

`expressao` e usada em inicializacoes, atribuicoes, condicoes, argumentos, indices de arrays, retornos e criacao de arrays.

```text
expressao
├── expressao || expressao
├── expressao && expressao
├── expressao == expressao
├── expressao != expressao
├── expressao < expressao
├── expressao > expressao
├── expressao <= expressao
├── expressao >= expressao
├── expressao + expressao
├── expressao - expressao
├── expressao * expressao
├── expressao / expressao
├── ! expressao
├── - expressao              [prec UMINUS]
├── ( expressao )
├── ID
├── NUMERO
├── true
├── false
├── acesso_array
├── leitura_scanner
├── chamada_metodo
└── instanciacao
```

A precedencia declarada no arquivo, da menor para a maior, e: `OU`, `E`, igualdade (`==`, `!=`), relacionais (`<`, `>`, `<=`, `>=`), soma/subtracao (`+`, `-`), multiplicacao/divisao (`*`, `/`) e operadores unarios (`!`, `-` unario). Os operadores binarios sao associativos a esquerda.

## 5. Galhos de expressao especializados

```text
acesso_array
└── ID [ expressao ]

leitura_scanner
└── ID . metodo_scanner ( )
    ├── next
    ├── nextLine
    ├── nextInt
    ├── nextDouble
    ├── nextFloat
    ├── nextLong
    ├── nextShort
    ├── nextByte
    └── nextBoolean

chamada_metodo
├── ID ( argumentos_opcionais )
└── ID . ID ( argumentos_opcionais )
    ├── argumentos_opcionais
    │   ├── vazio
    │   └── lista_argumentos
    └── lista_argumentos
        ├── expressao
        └── lista_argumentos, expressao

instanciacao
├── new Scanner ( System.in )
└── new tipo_base [ expressao ]
    └── tipo_base
        ├── int
        ├── double
        └── String
```

## 6. Alcance real a partir de `programa`

O unico caminho sintatico valido para alcancar instrucoes e comandos e atraves da hierarquia de classes e metodos:

```text
programa
└── declaracao_classe
    └── corpo_classe
        └── membro_classe
            └── declaracao_metodo
                └── bloco
                    └── lista_comandos
                        └── comando
                            ├── declaracao_local
                            ├── atribuicao
                            ├── comando_io
                            ├── condicional
                            ├── repeticao
                            ├── comando_salto
                            └── bloco
```

Com a remocao (comentarios) de `lista_instrucoes_io` e `%empty` da regra `programa`, nao existe mais entrada sem classe. Todos os testes de todas as issues foram reestruturados para estarem devidamente encapsulados dentro de classes e metodos validos (ex.: `class Nome { void metodo() { ... } }`), garantindo que 100% dos testes sejam aceitos atraves dessa unica raiz formal.

## 7. Pontos de atencao encontrados

- **Raiz estrita em `declaracao_classe`:** `programa` nao aceita mais entradas vazias (`%empty`) nem comandos soltos fora de classes (`lista_instrucoes_io`). Ambas as alternativas estao comentadas em `parser.y`.
- **Regra orfa (`lista_instrucoes_io`):** A regra `lista_instrucoes_io` permanece escrita no arquivo `parser.y`, mas como foi comentada da regra `programa`, tornou-se codigo inativo (*dead code*), gerando avisos de *nonterminal useless in grammar* no Bison.
- **Tipo `Scanner` adicionado:** `tipo` inclui `TOKEN_SCANNER` (`Scanner`), permitindo declaracoes como `Scanner sc;` tanto em atributos quanto em variaveis locais e parametros.
- **Token `byte` ausente em `tipo`:** Embora `%token TOKEN_BYTE` esteja declarado no cabecalho do `parser.y`, ele nao foi incluido nas producoes da regra `tipo` (que contem apenas `double`, `int`, `String`, `long`, `boolean`, `short`, `char`, `float` e `Scanner`).
- **Uma unica classe por arquivo:** `declaracao_classe` aceita apenas uma classe por programa. O teste `invalido_duas_classes.java` confirma que multiplas classes no mesmo arquivo sao consideradas sintaticamente invalidas.
- **Modificadores diferenciados:** `declaracao_variavel` (campos da classe) suporta `modificadores` completos (`public`, `private`, `protected`, `static`, `final`), enquanto `declaracao_local` (dentro de metodos) suporta apenas `modificador_variavel` (`vazio` ou `final`).
- **`final` em classes:** `TOKEN_FINAL` aparece tanto na lista geral de `modificadores` quanto nas formas especificas da regra `declaracao_classe` (`final class ...` e `public final class ...`).
- **Natureza puramente sintatica:** O parser valida apenas a conformidade gramatical (Bison/Flex). Nao ha analise semantica (checagem de tipos compativeis, escopo/declaracao previa de variaveis, presenca de `break` exclusivamente dentro de loops/switches, ou tipo de retorno em metodos).
