#ifndef PREPARADOR_H
#define PREPARADOR_H

/**
 * @file preparador.hpp
 * @brief Declaração de utilitários para a criação da primeira fase de um problema de programação linear. Além de declarar uma função para realizar deep copy.
 * 
 */

#include <vector>

/**
 * @brief Contém as variáveis utilizadas para a estrutura da primeira fase de um PPL.
 * 
 */

typedef struct
{
    std::vector<int> ondeAdicionar; // Vetor que contém os índices das linhas onde devem ser adicionadas variáveis artificiais.
    int numVarArtificiais = 0; // Número de váriaveis artificiais no problema.
    bool eDuasFases = false; // Indica se possui duas fases ou não. Se não possuir, essa estrutura não é utilizada.
} PreparacaoSimplex;

/**
 * @brief Realiza a identificação de linhas com desigualdades de maior ou igual ou de igualdades. Com isso, verifica se é necessário aplicar o método de duas fases.
 * 
 * @param a A matriz de coeficientes do problema de programação de linear
 * @param tamanhoLinhaA // Número de restrições do problema
 * @param tamanhoColunaA // Número de coeficientes do problema na forma padrão
 * @param numVars // Número de variáveis na forma canônica
 * @return PreparacaoSimplex A estrutura contendo a sinalização da necessidade de duas fases, o número de variáveis artificiais e o vetor com índices de onde colocá-las
 */

PreparacaoSimplex retornaPreparacaoSimplex(std::vector< std::vector<double>> a, int tamanhoLinhaA, int tamanhoColunaA, int numVars);

/**
 * @brief Adiciona as variáveis artificiais no problema. As linhas de desigualdades maior ou igual que e as igualdades recebem 1 na coluna. Caso contrário, recebem 0.
 * 
 * @param a A matriz de coeficientes do problema de programação linear
 * @param c O vetor de coeficientes da função objetivo
 * @param tamanhoColunaA Número de coeficientes do problema na forma padrão
 * @param tamanhoLinhaA Número de restrições do problema
 * @param preparador A estrutura contendo o vetor de índices das linhas que possuem desigualdades maior ou igual que ou igualdades.
 */

void adicionaVariaveisArtificiais(std::vector<std::vector<double>> &a, std::vector<double> &c, int &tamanhoColunaA, int tamanhoLinhaA, PreparacaoSimplex preparador);

/**
 * @brief Função de cópia profunda de um vetor de qualquer tipo.
 * 
 * @tparam T O tipo do vetor
 * @param v O vetor original
 * @return std::vector<T> O vetor novo que possui uma cópia do vetor v
 */

template <typename T>
std::vector<T> realizaCopiaProfunda(const std::vector<T> v)
{
    std::vector<T> ret;

    for (long long unsigned int i = 0 ; i < v.size() ; i++)
       ret.push_back(v[i]);

    return ret;    
}

#endif
