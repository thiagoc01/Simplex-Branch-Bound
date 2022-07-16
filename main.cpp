#include <vector>
#include <iostream>

/**
 * @file main.cpp
 * @brief Arquivo que interage com o usuário e inicializa um problema de programação linear.
 * 
 */

#include "simplex/simplex_inteiro.hpp"
#include "simplex/simplex.hpp"
#include "util/preparador.hpp"

/**
 * @brief Inicializa o problema conforme as entradas do usuário
 * 
 * @param a A matriz de coeficientes do problema
 * @param b O vetor de soluções do problema
 * @param c O vetor de coeficientes da função objetivo
 * @param tamanhoLinhaA O número de restrições no problema
 * @param tamanhoColunaA O número de coeficientes no problema
 * @param numVars O número de variáveis na forma canônica
 * @param tipoProblema Indica se o problema é de maximização ou minimização
 */

void inicializaProblemaOriginal(std::vector<std::vector<double>> a, std::vector<double> b, std::vector<double> c, int tamanhoLinhaA, int tamanhoColunaA, int numVars, bool tipoProblema)
{
    /*
    * Ao resolvermos o Simplex, iremos perder o problema original. Caso o usuário deseje arredondar para variáveis inteiras,
    * é necessário uma cópia do problema original para começarmos o método Branch and Bound.
    */

    std::vector<std::vector<double>> aOriginal;
    std::vector<double> bOriginal;
    std::vector<double> cOriginal;
    
    bool eProblemaInteiro = false; // Supõe, inicialmente, que o usuário não deseja arredondar as variáveis.

    PreparacaoSimplex preparador = retornaPreparacaoSimplex(a, tamanhoLinhaA, tamanhoColunaA, numVars); // Contém a preparação para o método de duas fases, se necessário.

    /* Copia o problema original para as variáveis referência */

    aOriginal = realizaCopiaProfunda(a);
    bOriginal = realizaCopiaProfunda(b);
    cOriginal = realizaCopiaProfunda(c);

    adicionaVariaveisArtificiais(a, c, tamanhoColunaA, tamanhoLinhaA, preparador); // Já que copiamos o problema original, pode-se adicionar as variáveis artificiais, se necessário.

    Simplex simplex(a, b, c, tipoProblema, preparador.eDuasFases, preparador.numVarArtificiais, numVars); // Cria a instância do Simplex do PPL original.
    simplex.aplicaSimplex(preparador.ondeAdicionar); // Resolve o problema

    std::cout << std::endl << "Deseja que as variáveis sejam inteiras? Digite 1 para sim, 0 para não.\n"; 
    std::cin >> eProblemaInteiro;

    if (eProblemaInteiro)
        iniciaProblemaInteiro(simplex, aOriginal, bOriginal, cOriginal, numVars); // Começa a resolução do problema inteiro.
}

/**
 * @brief Realiza a interação com o usuário e a recepção dos dados do problema original.
 * 
 */
void recebeEntradaUsuario()
{
    int tamanhoColunaA; 
    int tamanhoLinhaA;
    int numVars = 0;
    bool tipoProblema;    

    std::cout << "Se o problema for de maximização, digite 1. Caso contrário, digite 0." << std::endl;
    std::cin >> tipoProblema;

    std::cout << "Digite o número de variáveis de decisão no problema (as da forma canônica): " << std::endl;
    std::cin >> numVars;

    std::cout << "Digite o número de coeficientes na função objetivo:" << std::endl;
    std::cin >> tamanhoColunaA;

    std::cout << "Digite o número de restrições do problema:" << std::endl;
    std::cin >> tamanhoLinhaA;

    std::vector <std::vector<double>> a(tamanhoLinhaA, std::vector<double>(tamanhoColunaA, 0));
    std::vector<double> b(tamanhoLinhaA, 0);
    std::vector<double> c(tamanhoColunaA, 0);

    for (int i = 0 ; i < tamanhoLinhaA ; i++)
    {    
        std::cout << "Digite os coeficientes da restrição " + std::to_string(i + 1) << std::endl;

        for (int j = 0 ; j < tamanhoColunaA ; j++)
            std::cin >> a[i][j];
    }    

    std::cout << "Digite os valores do vetor B:\n";
    for (int i = 0 ; i < tamanhoLinhaA ; i++)
        std::cin >> b[i];

    std::cout << "Digite os coeficientes da função objetivo:\n";
    for (int i = 0 ; i < tamanhoColunaA ; i++)
        std::cin >> c[i];    

    inicializaProblemaOriginal(a, b, c, tamanhoLinhaA, tamanhoColunaA, numVars, tipoProblema);
}

int main()
{
    recebeEntradaUsuario(); 
    
    return 0;
}