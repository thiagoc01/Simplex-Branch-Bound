# Trabalho Final | Programação Linear 2022/1


##### Repositório com a implementação do Método Simplex de Duas Fases e do método Branch and Bound para Programação Linear Inteira.


&nbsp;

## Como funciona?

O programa possui uma classe Simplex, com um construtor público e uma função pública para início da execução.
Após a entrada dos dados necessários, o programa fornece cada iteração da execução do método Simplex.
Ao final, ele mostrará a matriz de coeficientes das restrições resultante, bem como o vetor de soluções, o vetor de
coeficientes da função objetivo e as variáveis básicas. Se desejado, pode-se tentar arredondar o problema para variáveis inteiras.
Utiliza o método Branch and Bound para tal arredondamento. Se o problema inteiro for de variáveis binárias, basta adicionar restrições
para cada variável do problema da forma canônica, tal que elas sejam menores ou iguais a 1.

## Formato da entrada

- tipo do problema (1 se maximizazção, 0 se minimização)
- número de variáveis de decisão (exceto folga)
- número de coeficientes da função objetivo (variáveis de decisão + folga)
- número de restrições do problema

- Para i de 0 até o número de restrições do problema
    - Para j até o número de coeficientes da função objetivo
        - coeficientes da i-ésima restrição

- Para i de 0 até o número de restrições do problema
    - valor B da i-ésima restrição

- Para i até o número de coeficientes da função objetivo
    - valor C da j-ésima variável do problema

- problema de programação inteira (1 se deseja arredondar as variáveis para variáveis inteiras, 0 caso contrário)

## Como compilar?

##### Baixe o código ou clone o repositório com:

```
$ git clone https://github.com/thiagoc01/trab3-prog-linear.git
```

##### Dentro do diretório, digite:

```
$ make
```

##### Execute o programa com:

```
$ ./simplex-solver
```

**O programa irá solicitar a entrada para funcionamento.**

Você também pode fornecer uma entrada e redirecionar o input via terminal utilizando o '<'.

```
$ ./simplex-solver < entrada
```

Ou utilizando o comando cat.

```
$ cat entrada | ./simplex-solver
```

