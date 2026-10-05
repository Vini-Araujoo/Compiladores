# Testes da Issue 3 — Controle de Fluxo

Os arquivos `valido_*.java` devem ser aceitos pelo parser (código de saída
`0`). Os arquivos `invalido_*.java` devem falhar com erro sintático (código de
saída diferente de `0`).

Os testes usam apenas fragmentos de programa, sem declaração de classe ou
método, pois a Issue 3 cobre comandos de controle de fluxo. Regras semânticas,
como impedir `return` fora de um método ou `break` fora de um laço/switch, não
são verificadas por esta suíte.

Para executar um arquivo manualmente:

```bash
make parser
./build/parser < tests/parser/issue_3/valido_switch.java
echo $?
```
