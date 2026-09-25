# Erros Encontrados

## 1. Testes da Issue 3 após a integração

Depois da integração com a Issue 4, os testes válidos da Issue 3 que usam
comandos diretamente no arquivo não passam. Essa dificuldade ocorreu porque
os testes das duas issues utilizam estruturas diferentes.

```java
if (contador) {
    contador = 1;
}
```

## 2. Variável não declarada

No arquivo abaixo, a atribuição para `n` é aceita mesmo sem uma declaração
anterior para essa variável:

```java
n = 0;
```

Esse comportamento foi identificado durante o teste do arquivo `Main.java`.

## 3. Declaração de `Scanner`

As duas formas abaixo produzem `Erro sintatico` no parser atual:

```java
Scanner n = new Scanner();
```

```java
Scanner n = new Scanner(System.in);
```

As duas formas foram rejeitadas durante os testes. A dificuldade encontrada
foi utilizar `Scanner` em uma declaração com instanciação, tanto sem entrada
quanto com `System.in`.
