# Validação de saltos e retornos

Esta etapa valida o contexto de `break` e `continue` e a forma/tipo de `return` dentro dos métodos.

## `break` e `continue`

`break` é aceito quando a travessia da AST está dentro de um laço (`for`, `while` ou `do-while`) ou de um `switch`. O analisador mantém a profundidade dessas estruturas ao entrar e sair dos nós.

`continue` é aceito somente dentro de um laço. Um `switch` sem laço externo não autoriza `continue`; um `switch` dentro de um laço continua permitindo que `continue` avance para o próximo ciclo do laço.

## `return`

Durante a análise do corpo de cada método, o analisador mantém o nó do método atual e consulta seu `tipo_dado`:

- método `void`: permite `return;` e rejeita `return expressão;`;
- método não-`void`: exige `return expressão;`;
- quando há expressão, seu tipo deve ser compatível com o tipo de retorno declarado.

Os erros são emitidos com a linha do comando `return` e incrementam o total de erros semânticos.

## Testes

Os casos são executados por `make test-semantic` e ficam em `tests/semantic/`:

- positivos: `valido_break_em_for.java`, `valido_break_em_switch.java`, `valido_continue_em_laco.java`, `valido_return_em_metodo_int.java` e `valido_return_widening.java`;
- negativos: `invalido_break_fora_de_laco.java`, `invalido_continue_fora_de_laco.java`, `invalido_continue_em_switch.java`, `invalido_return_com_valor_em_void.java`, `invalido_return_vazio_em_nao_void.java` e `invalido_return_tipo_incompativel.java`.
