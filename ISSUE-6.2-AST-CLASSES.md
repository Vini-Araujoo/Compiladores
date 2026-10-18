- **Responsável:** Kelyton de Lucas
- **Branch:** `feat/issue-6.2-ast-classes`
- **Módulo:** Estrutura do Programa & Classes
- **Status:** Planejada

---

## 1. Descrição da Alteração

Conectar a declaração sintática de uma classe (`declaracao_classe`) à raiz da
AST, preservando o nome da classe e agrupando seus atributos e métodos como
filhos diretos do nó `NO_CLASSE`.

O projeto aceita uma classe por programa. Portanto, o próprio `NO_CLASSE` será
a raiz da AST, sem a necessidade de criar um nó adicional `NO_PROGRAMA`.

O nome da classe será armazenado em `NO_CLASSE.valor.valor_str`. Os tokens
puramente sintáticos, como `class`, chaves e ponto e vírgula, não serão
representados como nós da AST.

---

## 2. O que deve ser implementado

- Associar a regra `declaracao_classe` ao nó `NO_CLASSE` produzido pelo parser.
- Preservar o identificador da classe em `NO_CLASSE.valor.valor_str`.
- Manter cada atributo ou método declarado em `corpo_classe` como filho direto
  de `NO_CLASSE`.
- Manter `corpo_classe` como acumulador dos membros da classe.
- Manter a variável global `NoAST *arvore` apontando para a raiz produzida pelo
  Bison.
- Liberar corretamente a string do nome da classe durante a destruição da AST.
- Aceitar as formas `class`, `final class`, `public class` e
  `public final class`.

Os modificadores `public` e `final` serão reconhecidos pela gramática, mas não
serão armazenados em nós separados nesta etapa, pois não são necessários para a
tradução atual para C.

---

## 3. Critérios de Aceite (Definition of Done)

- [ ] Todo programa válido possui um `NO_CLASSE` como raiz da AST.
- [ ] O nome da classe é preservado em `NO_CLASSE.valor.valor_str`.
- [ ] Atributos e métodos aparecem como filhos diretos de `NO_CLASSE`.
- [ ] As formas simples e com modificadores são aceitas sem novos conflitos
      Shift/Reduce ou Reduce/Reduce.
- [ ] O nome da classe é liberado corretamente por `liberarAST`.
- [ ] A documentação em `docs/ast/estrutura_ast.md` descreve a representação
      adotada.
- [ ] Os testes positivos e negativos são executados pelo Makefile ou script
      existente.

---

## 4. Testes

### Testes positivos

- `tests/ast/valido_classe_simples.java`: classe vazia com corpo mínimo.
- `tests/ast/valido_classe_com_modificadores.java`: classe
  `public final class Nome`.

Os testes devem confirmar a aceitação sintática e a impressão de
`NO_CLASSE (Nome)`.

### Teste negativo

- `tests/ast/invalido_classe_sem_nome.java`: classe sem identificador, rejeitada
  com erro sintático informando a linha.

Os casos devem ser executados por:

```bash
make test-ast
```

---

## 5. Documentação Necessária

- Atualizar `docs/ast/estrutura_ast.md` com:
  - `NO_CLASSE` como raiz para programas com uma classe;
  - nome da classe em `valor.valor_str`;
  - atributos e métodos como filhos diretos;
  - comportamento dos modificadores;
  - testes implementados e comando de execução.

---

## 6. Interdependências

- O nó `NO_CLASSE` agrupa membros produzidos pelas regras de atributos e
  métodos.
- As regras de membros podem continuar evoluindo independentemente, desde que
  retornem um `NoAST *` compatível com `membro_classe`.
- Não é necessário criar uma `struct Modificadores` nesta etapa.
- Não é necessário criar um stub separado para uma lista de membros: o
  `corpo_classe` existente já funciona como acumulador.
