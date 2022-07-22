#ifndef SIMPLEX_INTEIRO_H
#define SIMPLEX_INTEIRO_H

/**
 * @file simplex_inteiro.hpp
 * @brief Arquivo contendo a classe SimplexInteiro para resolução de problemas de programação linear inteiros e contendo funções para a resolução do mesmo.
 * 
 */

#include "simplex.hpp"

#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>

/**
 * @brief Estrutura de dados que contém a matriz A e os vetores B e C originais do problema, ou seja, antes da resolução.
 * 
 * Para cada ramificação de um problema de programação linear inteira, devemos adicionar uma restrição ao problema do nó pai.
 * Como, ao resolvermos o problema do nó pai, perdemos as instâncias originais, devemos tê-las copiadas antes de aplicar a resolução.
 */
typedef struct 
{
    std::vector<std::vector<double>> A;
    std::vector<double> B;
    std::vector<double> C;
} ElementosOriginais;

/**
 * @brief Classe que herda da classe Simplex, com modificações para a resolução do problema de programação linear inteiro.
 * 
 * Essa classe diferencia-se na necessidade de acessarmos os membros da mesma.
 * Logo, há a existência de métodos "get".
 * Precisamos, também, do mapeamento da matriz A e dos vetores B e C originais.
 * Para sabermos a quantidade de problemas existentes, há uma variável existente em todos os objetos dessa classe, e ele é incrementável a cada criação.
 * Os métodos da classe Simplex que realizam impressões na tela são modificados para imprimirem apenas os resultados finais, para evitar poluição na tela,
 * devido à grande quantidade de ramificações.
 */
class SimplexInteiro : public Simplex
{
    private:
        /* Membros do problema do nó pai */      
        std::vector<std::vector<double>> aOriginal;
        std::vector<double> bOriginal;
        std::vector<double> cOriginal;

        int divisoesProblema[2]; // Guarda a informação de quais nós são filhos desse problema se ele ramificar

        static int numTotalProblemas; // Total de problemas ramificados
        int idProblema; // Identificador deste problema
        short int tipoPoda; // 0 = não ramificou, 1 = inviabilidade/ausência ou ilimitação da solução, 2 = solução inteira incumbente, 3 = solução inteira menor que a incumbente

        /* Sobrescrição das funções para não impressão de status em cada iteração, evitando poluição da tela */
        void imprimeInformacao(std::string informacao) override;

        void imprimeInformacao(double informacao) override;

        void printMatrizes() override;      
              

    public:
        /**
         * @brief Cria uma instância de SimplexInteiro. Utilizada ao analisarmos o problema de programação linear fracionário, pois essa classe contém métodos "get".
         * 
         * @param s Objeto da classe Simplex que contém o problema fracionário original do usuário resolvido
         * @param aOriginal Matriz A antes da resolução do problema
         * @param bOriginal Vetor B antes da resolução do problema
         * @param cOriginal vetor C antes da resolução do problema
         */

        SimplexInteiro(Simplex s, std::vector<std::vector<double>> aOriginal, std::vector<double> bOriginal, std::vector<double> cOriginal);

        /**
         * @brief Cria uma instância de SimplexInteiro. Utilizada para a criação de nós filhos, representando as ramificações.
         * 
         * @param coeficientes Matriz de coeficientes do problema do nó pai com acréscimo de uma restrição do Branch and Bound
         * @param b Vetor de soluções do problema do nó pai com acréscimo do arredondamento inferior da variável fracionária que causou a ramificação.
         * @param c Vetor de coeficientes da função objetivo do problema do nó pai com acréscimo da variável de folga da nova restrição.
         * @param tipoProblema Tipo do problema original
         * @param eDuasFases Indica se esse novo problema realizará duas fases ou não
         * @param numVarArtificiais O número de variáveis artificiais no novo problema
         * @param numVars O número de variáveis na forma canônica
         * @param e A estrutura de dados contendo os elementos do problema do nó pai mais as adições a matriz coeficientes e aos vetores B e C
         */
        SimplexInteiro(std::vector <std::vector<double>> coeficientes, std::vector<double> b, std::vector<double> c,
                        bool tipoProblema, bool eDuasFases, int numVarArtificiais, int numVars, ElementosOriginais e);

        /**
         * @brief Retorna uma cópia da matriz A desse problema antes da resolução
         * 
         * @return std::vector<std::vector<double>> A cópia da matriz A
         */

        std::vector<std::vector<double>> getMatrizAOriginal();

        /**
         * @brief Retorna uma cópia do vetor B desse problema antes da resolução
         * 
         * @return std::vector<double> A cópia do vetor B
         */

        std::vector<double> getVetorBOriginal();

        /**
         * @brief Retorna uma cópia do vetor C desse problema antes da resolução
         * 
         * @return std::vector<double> A cópia do vetor C
         */

        std::vector<double> getVetorCOriginal();

        /**
         * @brief Retorna uma cópia do vetor de bases desse problema antes da resolução
         * 
         * @return std::vector<std::pair<int, double>> A cópia do vetor de bases
         */

        std::vector<std::pair<int, double>> getBase();

        /**
         * @brief Retorna a solução ótima desse problema
         * 
         * @return double A solução ótima do problema
         */

        double getSolucaoOtima();

        /**
         * @brief Retorna o indicador de existência de solução do problema
         * 
         * @return true Se existe solução
         * @return false Caso contrário
         */
        bool getSemSolucao();

        /**
         * @brief Retorna o indicador de ilimitação da solução do problema
         * 
         * @return true Se o problema é ilimitado
         * @return false Caso contrário
         */

        bool getEIlimitado();

        /**
         * @brief Retorna se o problema é de maximização ou minimização
         * 
         * @return true Se o problema é de maximização
         * @return false Se o problema é de minimização
         */

        bool getTipoProblema();

        /**
         * @brief Retorna o identificador desse problema ou a quantidade de problemas existentes até esse problema
         * 
         * @param deTodos true Se é desejado a quantidade de problemas existentes, false se deseja o identificador desse problema
         * @return int O identificador desse problema ou a quantidade de problemas existentes
         */
        int getNumeroProblema(bool deTodos);

        /**
         * @brief Retorna os IDs das ramificações do problema
         * 
         * @return int* o array de 2 posições contendo o nó da esquerda e da direita respectivamente
         */

        int* getDivisoesProblema();

        /**
         * @brief Retorna o tipo de poda efetuada no nó
         * 
         * @return int O tipo de poda efetuada
         */

        short int getTipoPoda();

        /**
         * @brief Configura o tipo de poda do problema
         * 
         * @param tipo O tipo de poda do problema
         */

        void setTipoPoda(int tipo);

        /**
         * @brief Configura quais nós são filhos desse nó
         * 
         * @param divisoes Os IDs dos ramos da esquerda e da direita
         */

        void setDivisoesProblema(int divisoes[2]);

        /**
         * @brief Configura o ID desse problema
         * 
         * @param id O ID do problema
         */

        void setNumeroProblema(int id);

        /**
         * @brief Aumenta a quantidade de problemas criados em 2
         * 
         */

        void aumentaQuantidadeProblemas();

        /**
         * @brief Imprime as matrizes finais chamando a função da classe Simplex
         * 
         */

        void printMatrizesFinais() override;

        /**
         * @brief Realiza a impressão dos resultados finais chamando a função da classe Simplex
         * 
         */

        void realizaImpressaoFinal() override;

        /**
         * @brief Aplica o Simplex sem realizar as impressões de informação na tela
         * 
         * @param ondeAdicionar 
         */

        void aplicaSimplex(std::vector<int> ondeAdicionar) override;
              
};

/**
 * @brief Retorna a função ceil ou floor, dependendo do sinal da solução ótima. Se o número é positivo, utiliza-se floor. Caso contrário, ceil.
 * @param solucaoOtima A solução do problema atual a ser testada
 * @return double(*)(double valor) O endereço da função ceil ou floor
 */

double (*retornaFuncaoComparacao(double solucaoOtima))(double valor);

/**
 * @brief Testa se o número é inteiro com uma tolerância de 7 casas decimais.
 * 
 * @param num O número a ser testado
 * @return true Se o número tem uma parte decimal maior que 10^7.
 * @return false Se o número é fracionário
 */

bool eInteiro(double num);

/**
 * @brief Controla a fila de problemas a serem analisados, implementado com uma estratégia de busca em largura.
 * 
 * @param solucaoOtimaGlobal A solução ótima incumbente
 * @param solucaoGlobal As coordenadas da solução incumbente
 */

void controlaProblemasInteiros(double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal);

/**
 * @brief Retorna a posição da primeira coordenada não inteira encontrada.
 * 
 * @param solucao As coordenadas da solução encontrada no problema
 * @return int O índice da coordenada fracionária no vetor ou -1 se a solução é inteira.
 */

int retornaPosicaoNaoInteiro(std::vector<double> solucao);

/**
 * @brief Verifica se irá encerrar a sub-árvore seguindo os critérios do Branch and Bound para Programação Linear Inteira
 * 
 * @param problema O problema do nó
 * @param solucaoOtimaGlobal A solução incumbente
 * @param solucaoGlobal Coordenadas da solução incumbente
 * @param solucao Coordenadas da solução atual
 * @param solucaoOtimaTeste Solução ótima desse problema
 * @param posicaoFracionario Posição da primeira coordenada que não é inteira
 * @return true Se o nó foi podado
 * @return false Caso o nó não tenha sido podado. Ele será ramificado
 */

bool deveRealizarPoda(SimplexInteiro problema, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal, std::vector<double> solucao,
                                        double solucaoOtimaTeste, int posicaoFracionario);

/**
 * @brief Realiza a poda da sub-árvore conforme a definição do método Branch and Bound ou cria novos problemas se há a possibilidade de encontrar a solução.
 * 
 * @param problema O problema do nó atual a ser analisado
 * @param solucaoOtimaGlobal A solução incumbente atual
 * @param solucaoGlobal As coordenadas da solução incumbente atual
 */

void verificaSolucaoInteira(SimplexInteiro problema, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal);

/**
 * @brief Retorna um dos problemas da ramificação a ser criada
 * 
 * @param A A matriz de coeficientes do nó pai
 * @param B O vetor B do nó pai
 * @param C O vetor C do nó pai
 * @param solucao As coordenadas da solução atual para a captura da variável fracionada, que será utilizada para a criação da nova restrição
 * @param posicaoNaoInteiro A coordenada que tem um valor fracionário
 * @param tipoProblema Tipo do problema fornecido pelo usuário
 * @param eMenor true se é uma restrição menor ou igual que, false se é maior ou igual que
 * @param ondeAdicionar Vetor com os índices das linhas contendo variáveis artificiais
 * @return SimplexInteiro O problema novo com a restrição adicionada
 */

SimplexInteiro retornaProblema(std::vector<std::vector<double>> A, std::vector<double> B, std::vector<double> C,
                                std::vector<double> solucao, int posicaoNaoInteiro, bool tipoProblema, bool eMenor, std::vector<int> &ondeAdicionar);

/**
 * @brief Cria os dois problemas da ramificação e adiciona na fila para análise posterior na busca em largura
 * 
 * @param A A matriz de coeficientes do nó pai
 * @param B O vetor B do nó pai
 * @param C O vetor C do nó pai
 * @param posicaoNaoInteiro A coordenada que tem um valor fracionário
 * @param tipoProblema Tipo do problema fornecido pelo usuário
 * @param solucao As coordenadas da solução atual para a captura da variável fracionada, que será utilizada para a criação da nova restrição
 * @param solucaoOtimaGlobal A solução incumbente atual
 * @param solucaoGlobal As coordenadas da solução incumbente atual
 */

void criaNovosProblemas(std::vector<std::vector<double>> A, std::vector<double> B, std::vector<double> C,
                        int posicaoNaoInteiro, bool tipoProblema, std::vector<double> solucao, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal, int divisoes[2]);

/**
 * @brief Realiza a verificação inicial para viabilidade do problema inteiro e o controle dos resultados finais
 * 
 * @param simplex Problema original do usuário já resolvido
 * @param aOriginal Cópia da matriz A do problema fornecido pelo usuário
 * @param bOriginal Cópia do vetor B do problema fornecido pelo usuário
 * @param cOriginal Cópia do vetor C do problema fornecido pelo usuário
 * @param numVars Número de variáveis na forma canônica
 */

void iniciaProblemaInteiro(Simplex simplex, std::vector<std::vector<double>> aOriginal, std::vector<double> bOriginal, std::vector<double> cOriginal, int numVars);

/**
 * @brief Inicializa os ponteiros que serão usados para comparar a solução incumbente com a solução do problema fornecido no instante
 * 
 * No problema de maximização, alteramos a solução incumbente se, e somente se, ela for menor que a solução nova encontrada.
 * Logo, precisamos da operação solucaoIncumbente < solucaoNova.
 * 
 * Já no de minimização, alteramos a solução incumbente se, e somente se, ela for maior que a solução nova encontrada.
 * Logo, precisamos da operação solucaoIncumbente > solucaoNova.
 * 
 * Para não criarmos funções diferentes ou utilização de desvios condicionais excessivos, basta utilizar um ponteiro de função
 * que é alterado conforme o tipo do problema fornecido pelo usuário. Assim, o código será genérico e utilizado via alteração do ponteiro.
 * 
 * @param simplexInteiro A instância do problema original adaptado para a classe SimplexInteiro
 * @param solucaoOtimaGlobal A variável para guardar a solução incumbente.
 */

void inicializaPonteirosComparacao(SimplexInteiro simplexInteiro, double &solucaoOtimaGlobal);

/**
 * @brief Verifica se a solução do problema original do usuário já é inteira.
 * 
 * @param base As variáveis básicas do problema original após a resolução
 * @param solucao Vetor para inicialização da solução do problema original
 * @param numVars Número de variáveis na forma canônica
 * @return int -1 se a solução é inteira ou a coordenada da primeira coordenada fracionária encontrada
 */

int testaSolucaoOriginal(std::vector<std::pair<int, double>> base, std::vector<double> &solucao, int numVars);

/**
 * @brief Imprime os resultados do problema inteiro
 * 
 * @param solucaoOtimaGlobal Solução ótima encontrada
 * @param solucaoGlobal Coordenadas inteiras da solução ótima
 * @param numVars Número de variáveis na forma canônica
 */

void imprimeSolucaoInteiraFinal(double solucaoOtimaGlobal, std::vector<double> solucaoGlobal, int numVars);

#endif