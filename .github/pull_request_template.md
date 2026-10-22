## Descrição da Alteração
Implementação da estrutura mínima da AST para declaração de classes Java,
preservando o nome da classe no nó raiz `NO_CLASSE` e agrupando atributos e
métodos como filhos diretos. Os modificadores `public` e `final` continuam
sendo reconhecidos pela gramática, mas não são armazenados na AST nesta etapa.

- **Issue relacionada:** Issue 6.2 (`ISSUE-6.2-AST-CLASSES.md`)
- **Responsável:** Kelyton de Lucas
- **Branch de origem:** `feat/issue-6.2-ast-classes` -> **Branch de destino:** `main`

---

## O que foi implementado?
- [x] Regras sintáticas no Bison (`parser/parser.y`) correspondentes ao escopo da issue.
- [x] Garantia de ausência de conflitos Shift/Reduce ou Reduce/Reduce no Bison.
- [x] Remoção ou adequação dos stubs locais para integração.

---

## Critérios de Aceite (Definition of Done)
- [x] **Documentação:** Arquivo em `docs/ast/estrutura_ast.md` atualizado com a representação da classe e as decisões de projeto.
- [x] **Testes:**
  - [x] Casos positivos em `tests/ast/` validados com código 0.
  - [x] Caso negativo em `tests/ast/` validado com rejeição e erro sintático.
  - [x] Comando `make test-ast` executado com 100% de aprovação.

---

## Evidências da Execução dos Testes
O comando previsto no template (`make test ISSUE=issue_X`) não é aplicável a
esta issue, pois os casos foram implementados na suíte da AST. Foi executado:

```text
$ make test-ast
...
Resumo: 21 passou, 0 falhou.
Todos os testes unitários da AST passaram com sucesso!
```
