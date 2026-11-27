# Compilador Java para C (Transpilador Fonte-para-Fonte)

Este projeto implementa um tradutor de código-fonte (*source-to-source compiler* / transpilador) que recebe um programa escrito em um subconjunto da linguagem Java e gera código equivalente em **C procedural**. Desenvolvido para a disciplina de Compiladores 1 da Universidade de Brasília (UnB / FGA).

O compilador executa todo o pipeline clássico de compilação:
1. **Análise Léxica (Flex):** Reconhece tokens, palavras-chave, identificadores e literais.
2. **Análise Sintática (Bison):** Reconhece a gramática e constrói a Árvore Sintática Abstrata (AST).
3. **Análise Semântica:** Gerencia a Tabela de Símbolos com escopos aninhados, valida declarações, constantes (`final`) e infere tipos estáticos.
4. **Geração de Código C:** Percorre a AST e emite código C formatado com indentação hierárquica e algoritmo de duas passagens para variáveis globais.

---

## Como Rodar (Java $\rightarrow$ C)

### 1. Compilar o Projeto
Para construir o binário do compilador utilizando o `Makefile`:

```bash
make
# ou: make compilador
```

Para limpar os arquivos intermediários e executáveis da pasta `build/`:
```bash
make clean
```

---

### 2. Traduzir um Arquivo Java para C
O executável `./build/compilador` recebe o arquivo `.java` como entrada e emite o código C gerado diretamente na saída padrão (`stdout`). Utilize o operador de redirecionamento `>` para salvar o arquivo `.c`:

```bash
./build/compilador programa.java > programa.c
```

*(Opcional: o arquivo C gerado pode ser inspecionado diretamente ou compilado com qualquer compilador C padrão, como `gcc -Wall -Wextra -o main programa.c && ./main`).*

---

### 3. Como Executar os Testes

* **Testes de Tradução do Compilador (Java $\rightarrow$ C):**
  ```bash
  make test-compilador
  ```
* **Todas as Suítes de Teste do Projeto (290 testes):**
  ```bash
  make test-all
  ```
* **Testes por Etapa Isolada:**
  ```bash
  make test           # 64 testes do Parser (Bison/Flex)
  make test-ast       # 39 testes unitários da AST
  make test-tabela    # 4 testes da Tabela de Símbolos
  make test-semantic  # 39 testes de Validação Semântica
  make test-codegen   # 129 testes de Geração de Código C
  ```

---

## Organização do Projeto

```text
Compiladores/
├── lexer/               # Regras do analisador léxico escritas em Flex (lexer.l)
├── parser/              # Gramática sintática LALR(1) escrita em Bison (parser.y)
├── src/                 # Código-fonte em C dos módulos do compilador
│   ├── ast/             # Definição, nós, impressão e liberação da AST
│   ├── semantic/        # Tabela de símbolos, escopos, matriz de tipos e inferência
│   ├── codegen/         # Geração de código C (infraestrutura, decls, exprs, fluxo, I/O)
│   ├── main.c           # Ponto de entrada (CLI driver) e orquestrador da compilação
│   └── tipos.h          # Enumerações globais de nós da AST e tipos de dados
├── tests/               # Bateria de testes automatizados
│   ├── compilador/      # Cenários de teste de tradução Java para C (.java e .c gerados)
│   ├── parser/          # Casos de teste léxicos e sintáticos (Issues 1 a 5)
│   ├── ast/             # Testes unitários de construção e visualização da AST
│   ├── semantic/        # Testes de validação semântica e escopo
│   └── codegen/         # Testes modulares de emissão de código C
├── docs/                # Documentação técnica detalhada do projeto
└── Makefile             # Automação de compilação e suítes de testes
```

---

## Links para Documentação Extra

Para consultar detalhes aprofundados sobre a arquitetura e especificações do projeto:

* **[Guia e Relatório de Testes do Compilador](docs/testes_compilador.md):** Tabela detalhada de todos os cenários de teste, saídas geradas e comandos de execução.
* **[Limitações e Fora de Escopo](docs/limitacoes_e_escopo.md):** Construções da linguagem Java padrão não suportadas pela gramática ou pela tradução procedural.
* **[Erros Conhecidos e Soluções](docs/erros-conhecidos.md):** Registro de bugs identificados durante o desenvolvimento e resoluções aplicadas.
* **[Especificação da Gramática BNF](docs/grammar/):** Regras de produção do Parser Bison separadas por domínio (classes, declarações, expressões, fluxo, I/O).
* **[Estrutura da Árvore Sintática (AST)](docs/ast/estrutura_ast.md):** Mapeamento de nós da AST e decisões de projeto.
* **[Tabela de Símbolos e Escopos](docs/semantic/tabela_simbolos.md):** Arquitetura da pilha de escopos dinâmicos e resolução de variáveis.
* **[Tradução e Geração de Código C](docs/codegen/):** Especificação da emissão de tipos, declarações, expressões e I/O.
