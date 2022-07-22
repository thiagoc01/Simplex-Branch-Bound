#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <vector>

/**
 * @brief Implementa o método Simplex e o método de duas fases.
 * 
 */

class Simplex
{
    protected:
        int linhas, colunas; // Linhas = número de restrições ; Colunas = número de variáveis
        int numVarArtificiais; // Número de variáveis artificiais no problema
        int numVars; // Número de variáveis na forma canônica
        std::vector <std::vector<double> > A; // Matriz dos coeficientes das restrições
        std::vector<double> B; // Vetor de soluções das restrições
        std::vector<double> C; // Vetor de coeficientes da função objetivo.
        std::vector< std::pair<int, double> > base; // Vetor de pares para mapearmos as bases e os B_i's respectivos
        std::vector <double> C_artificial; // Vetor de coeficientes da função objetivo artificial da primeira fase
        double solucaoOtima; // Solução ótima do problema
        double solucaoOtimaPrimeiraFase; // Solução ótima da primeira fase
        bool eIlimitado; // Caso que o problema é ilimitado
        bool eMaximizacao; // Utilizada para verificar se o problema é de maximização ou não.
        bool semSolucao; // Caso que o problema não possui solução.
        bool eDuasFases; // Verificador se estamos na primeira fase ou na segunda
        bool tipoProblema; // Indica se o problema é de maximização (true) ou minimização (false)

        /**
         * @brief Realiza o cálculo de uma iteração da segunda fase do Simplex.
         * 
         * @param iteracao O número da iteração
         * 
         * @return true - Se a iteração for final por algum motivo do problema.
         * @return false - Se ainda há possibilidade de maximizar/minimizar.
         */

        virtual bool calculaIteracaoSimplex(int iteracao);

        /**
         * @brief Verifica se todos os coeficientes são positivos ou nulos.
         * 
         * @return true - Se não há valores negativos na função objetivo.
         * @return false - Se ainda há valor negativo na função objetivo.
         */
        bool verificarSolucaoOtima();

        /**
         * @brief Aplica o pivoteamento em cada linha que não seja a pivô.
         * 
         * @param linhaPivo O índice da linha do número pivô
         * @param colunaNumPivo O índice da coluna do número pivô
         */
        void realizaPivoteamento(int linhaPivo, int colunaNumPivo);

        /**
         * @brief Imprime a matriz A e os vetores B e C.
         * 
         */
        virtual void printMatrizes();

        /**
         * @brief Imprime a matriz A e os vetores B e C da última iteração.
         * 
         */
        virtual void printMatrizesFinais();

        /**
         * @brief Procura pelo coeficiente mais negativo da função objetivo
         * 
         * @return int - O índice da coluna que contém o número mais negativo.
         */
        int achaColunaPivo();

        /**
         * @brief Procura pela linha i tal que B_i / A[i][colunaNumPivo] é o menor dos valores.
         * 
         * @param colunaNumPivo O índice da coluna da variável a entrar na base. 
         * @return int O índice da linha do número pivô.
         */
        int achaLinhaPivo(int colunaNumPivo);

        /**
         * @brief Prepara o PPL artificial para a primeira fase do método de duas fases
         * 
         * @param ondeAdicionar Vetor que contém as linhas que têm variáveis artificiais
         * @return true - Se o PPL original tem solução
         * @return false - Caso o PPL original não tenha solução
         */

        virtual bool iniciaPrimeiraFase(std::vector<int> ondeAdicionar);

        /**
         * @brief Realiza o controle da primeira fase, a análise do resultado e a remoção das variáveis artificiais
         * 
         * @return true - Se o PPL original tem solução
         * @return false - Caso o PPL original não tenha solução
         */

        virtual bool realizaPrimeiraFase();

        /**
         * @brief Realiza a impressão dos resultados na última iteração
         * 
         */

        virtual void realizaImpressaoFinal();

        /**
         * @brief Imprime a string passada por argumento na tela
         * 
         * @param informacao A informação desejada para impressão
         */

        virtual void imprimeInformacao(std::string informacao);

        /**
         * @brief Imprime a string passada por argumento na tela
         * 
         * @param informacao A informação desejada para impressão
         */

        virtual void imprimeInformacao(double informacao);

    public:
        /**
         * @brief Construtor da classe
         * 
         * @param coeficientes Coeficientes da matriz A
         * @param b Vetor de soluções de cada restrição
         * @param c Vetor de coeficientes da função objetivo
         * @param tipoProblema true se é de maximização, false se é de minimização.
         * @param eDuasFases true se o problema tem duas fases, false caso contrário.
         */
        Simplex (std::vector <std::vector<double>> coeficientes, std::vector<double> b, std::vector<double> c, bool tipoProblema, bool eDuasFases, int numVarArtificiais, int numVars);        

        /**
         * @brief Função que inicia o método Simplex.
         * 
         */
        virtual void aplicaSimplex(std::vector<int> ondeAdicionar); 
};

#endif