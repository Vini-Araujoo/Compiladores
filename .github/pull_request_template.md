## Descrição da Alteração
Descreva de forma concisa o que foi implementado nesta branch.

- **Issue relacionada:** Issue X (`issues_X.md`)
- **Responsável:** 
- **Branch de origem:** `feat/issue-X-...` -> **Branch de destino:** `main`

---

## O que foi implementado?
- [ ] Regras sintáticas no Bison (`parser/parser.y`) correspondentes ao escopo da issue.
- [ ] Garantia de ausência de conflitos Shift/Reduce ou Reduce/Reduce no Bison.
- [ ] Remoção ou adequação dos stubs locais para integração.

---

## Critérios de Aceite (Definition of Done)
- [ ] **Documentação:** Arquivo em `docs/grammar/...` preenchido com a especificação formal BNF e decisões de projeto.
- [ ] **Testes:**
  - [ ] Casos de teste positivos da pasta `tests/parser/issue_X/` validados com código 0.
  - [ ] Casos de teste negativos da pasta `tests/parser/issue_X/` validados com rejeição e erro sintático.
  - [ ] Comando `make test ISSUE=issue_X` executado com 100% de aprovação.

---

## Evidências da Execução dos Testes
Cole abaixo a saída do comando `make test ISSUE=issue_X`:

```text
(cole aqui a saída do terminal)
```
