# Lusa

> **Uma linguagem de programação direta, rápida e sem enrolação.**

[![Status](https://img.shields.io/badge/status-beta-orange)]()
[![Versão](https://img.shields.io/badge/versão-0.0.1-blue)]()
[![Licença](https://img.shields.io/badge/licença-MIT-green)]()

---

## ⚠️ Aviso importante — beta em desenvolvimento ativo

Lusa ainda está crescendo. Isso significa que **não garantimos compatibilidade entre versões** — sintaxe, comportamento e APIs podem mudar enquanto construímos as bases certas.

Se você é do tipo que gosta de estar no começo das coisas, de opinar antes que tudo esteja engessado, de ver uma linguagem nascer — bem-vindo. Se precisa de estabilidade de produção, aguarde versões futuras e acompanhe o progresso.

---

## O que é Lusa?

Lusa é uma linguagem compilada para **bytecode**, executada sobre uma **máquina virtual baseada em registradores**. Tipagem estática, sem coletor de lixo, e com um compilador que não te faz esperar.

A premissa é que você não deveria ter que escolher entre clareza e velocidade. Lusa compila rápido, executa rápido, e foi pensada para ser escrita rápido — sem cerimônia, sem boilerplate, sem abstrações que escondem o que está acontecendo.

**Sem GC** é uma decisão de designer. Você sabe quando memória é alocada. O comportamento em runtime é o comportamento que você escreveu.

---

## Primeiros passos

### Instalação

```sh
git clone https://github.com/lusalang/lusa.git
cd lusa
cmake -B build
cmake --build build
export PATH="$PATH:$(pwd)/bin"

lusa --version
```

### Hello, World!

Crie um arquivo `ola.lusa`:

```lusa
print_str("Olá, Mundo!");
```

Compile e execute:

```sh
lusa build ola.lusa -o ola.lusa
lusa run ola.lusa
```

---

## A linguagem

### Variáveis

Em Lusa, variáveis são declaradas com `var`, seguidas do nome e do tipo:

```lusa
var x int = 42;
var pi float = 3.14;
var nome string = "Lusa";
var ativo bool = true;
```

Tipos disponíveis: `int`, `float`, `string`, `bool`.

A variável pode ser declarada sem valor inicial — ela começa com o zero do tipo:

```lusa
var contador int;   // começa em 0
var ligado bool;    // começa em false
```

---

### Funções

Funções são declaradas com `fn`. Parâmetros levam o tipo junto ao nome. O retorno usa `return`:

```lusa
fn soma(a int, b int) {
    return a + b;
}

fn saudacao(nome string) {
    print_str("Olá, ");
    print_str(nome);
}
```

Chamando funções:

```lusa
var resultado int = soma(10, 32);
print(resultado);      // 42

saudacao("Mundo");
```

---

### Controle de fluxo

**`if` e `else`:**

```lusa
var x int = 10;

if x == 10 {
    print_str("x é dez");
} else {
    print_str("x é outro valor");
}
```

Encadeamento com `else if`:

```lusa
if x < 0 {
    print_str("negativo");
} else if x == 0 {
    print_str("zero");
} else {
    print_str("positivo");
}
```

**`while`:**

```lusa
var i int = 0;

while i < 5 {
    print(i);
    i = i + 1;
}
```

---

### Arrays

Arrays são alocados com um literal de colchetes e indexados com `[]`:

```lusa
var numeros int = [10, 20, 30, 40, 50];

// leitura
var primeiro int = numeros[0];
print(primeiro);   // 10

// escrita
numeros[2] = 99;
print(numeros[2]); // 99
```

Iterando um array:

```lusa
var dados int = [3, 7, 2, 9, 1];
var i int = 0;

while i < 5 {
    print(dados[i]);
    i = i + 1;
}
```

---

### Saída

Lusa tem duas funções nativas de output:

| Função | Uso |
|---|---|
| `print(valor)` | Imprime um valor numérico ou booleano |
| `print_str(texto)` | Imprime uma string |

```lusa
print(42);
print(3 + 7);
print_str("Lusa é rápida");

var x int = 100;
print(x);
```

---

### Operadores

| Operador | Significado |
|---|---|
| `+` | Adição |
| `-` | Subtração |
| `==` | Igualdade |
| `<` | Menor que |
| `>` | Maior que |

```lusa
var a int = 10;
var b int = 3;

print(a + b);    // 13
print(a - b);    // 7

if a > b {
    print_str("a é maior");
}
```

---

### Comentários

```lusa
// isto é um comentário de linha
```

---

## Exemplos completos

### FizzBuzz

```lusa
fn fizz(n int) {
    var i int = 1;

    while i < n + 1 {
        if i == 3 {
            print_str("Fizz");
        } else if i == 5 {
            print_str("Buzz");
        } else {
            print(i);
        }
        i = i + 1;
    }
}

fizz(15);
```

### Máximo de um array

```lusa
fn maximo(nums int, tamanho int) {
    var max int = nums[0];
    var i int = 1;

    while i < tamanho {
        if nums[i] > max {
            max = nums[i];
        }
        i = i + 1;
    }

    return max;
}

var lista int = [4, 9, 2, 17, 6, 3];
var resultado int = maximo(lista, 6);
print(resultado);   // 17
```

### Fibonacci

```lusa
fn fib(n int) {
    if n < 2 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

print(fib(10));   // 55
```

---

## A toolchain

| Comando | O que faz |
|---|---|
| `lusa build <arquivo>.lusa -o <saida>.lusa` | Compila para bytecode |
| `lusa run <arquivo>.lusc` | Executa o bytecode compilado |
| `lusa check <arquivo>.lusa` | Verifica erros sem compilar |

### Fluxo de trabalho

```sh
# Compile
lusa build programa.lusa -o programa.lusa

# Execute
lusa run programa.lusa
```

O arquivo `.lusa` é portável: você compila uma vez e executa em qualquer máquina com a VM de Lusa instalada. Sem recompilar, sem dependências externas.

---

## Como funciona por dentro

Lusa compila em **uma única passagem** — sem AST intermediária, direto para bytecode. É parte do motivo pelo qual a compilação é tão rápida.

A VM opera com instruções de 32 bits e um conjunto de registradores fixos. Cada instrução carrega opcode + operandos em uma palavra só. Sem GC significa sem pausas em runtime: a memória alocada tem tempo de vida determinístico, e o comportamento do programa é o comportamento que você escreveu.

---

## O que vem por aí

Lusa está longe de pronta — e não temos problema em admitir isso. Aqui está o que está no horizonte:

- **AST própria** — o compilador atual faz passagem única direta. Uma AST vai abrir espaço para otimizações reais, melhores mensagens de erro e análise estática mais profunda.
- **Loop `for` com ranges** — o token já existe no lexer, o loop ainda não foi implementado.
- **Operadores lógicos** — `&&`, `||` e `!` estão na lista.
- **Multiplicação e divisão** — `*` e `/` ainda não existem; vêm em breve.
- **Tipos de retorno explícitos em funções** — para verificação estática mais rigorosa.
- **Mensagens de erro mais úteis** — erros devem ensinar, não apenas apontar o problema.
- **Ergonomia de sintaxe** — algumas construções ainda têm mais atrito do que deveriam; estamos refinando.
- **Biblioteca padrão** — funções de string, matemática e I/O de arquivo.
- **Módulos** — separação de código em múltiplos arquivos.
- **Suporte explícito a múltiplas plataformas** — Windows e ARM na toolchain.

---

## Contribuindo

Lusa é construída abertamente. Se você encontrou um bug, tem uma ideia ou simplesmente quer entender como um compilador funciona na prática — o código está lá, é pequeno e legível.

```sh
git clone https://github.com/lusalang/lusa.git
cd lusa
```

O compilador e a VM são escritos em C. Issues com `good first issue` são um bom ponto de entrada.

---

## Licença

MIT. Código aberto, sem restrições.

---

<p align="center">
  <em>Lusa — compilação rápida, execução previsível, código que vai direto ao ponto.</em>
</p>
