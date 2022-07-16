#include <iostream>
#include <cmath>

/**
 * @file simplex_inteiro.cpp
 * @brief Implementa um problema de programação linear inteira geral utilizando busca em largura sem heurística.
 * 
 * Partimos do nó raiz se esse não possui uma solução inteira com tolerância de 7 casas decimais.
 * Para cada nó, dividimos o problema em dois, com uma restrição x_i < piso(k) e x_i > piso(k) + 1,
 * onde x_i é a primeira coordenada fracionária encontrada e k é seu valor fracionário.
 * A resolução dos problemas é feita na criação e análise de resultados é feita realizando busca em largura.
 * Resolvemos um nível da árvore do Branch and Bound antes de prosseguirmos para o próximo.
 * Isso é implementado utilizando o conceito padrão de busca em largura, que é através de uma fila.
 */

#include "simplex_inteiro.hpp"
#include "../util/preparador.hpp"
#include "simplex.hpp"

int SimplexInteiro::numTotalProblemas = 1; // Inicialização do membro static da classe SimplexInteiro
static int numVariaveisCanonica; // Número de variáveis na forma canônica, visível somente para esse arquivo
static std::queue<std::pair<SimplexInteiro, std::vector<int>>> fila; // Fila de problemas ramificados para serem analisados

/* Ponteiros de funções para a comparação entre a solução incumbente e a solução encontrada no nó */

static bool (*comparaSolucoesInclusive)(double a, double b); // Caso para a poda por solução inteira e alteração dessa
static bool (*comparaSolucoesExclusive)(double a, double b); // Caso para a poda por solução menor que a atual


SimplexInteiro::SimplexInteiro(Simplex s, std::vector<std::vector<double>> aOriginal, std::vector<double> bOriginal, std::vector<double> cOriginal) : Simplex(s)
{
    /* Inicialização dos elementos que guardarão o problema original e inicialização de novos elementos, já que eles foram copiados do objeto s. */

   this->aOriginal = realizaCopiaProfunda(aOriginal);
   this->bOriginal = realizaCopiaProfunda(bOriginal);
   this->cOriginal = realizaCopiaProfunda(cOriginal);

   this->A = realizaCopiaProfunda(aOriginal);
   this->B = realizaCopiaProfunda(bOriginal);
   this->C = realizaCopiaProfunda(cOriginal);

   this->idProblema = this->numTotalProblemas; // Identificador deste problema
}

SimplexInteiro::SimplexInteiro(std::vector <std::vector<double>> coeficientes, std::vector<double> b, std::vector<double> c, bool tipoProblema, bool eDuasFases, int numVarArtificiais, int numVars, ElementosOriginais e)
    : Simplex(coeficientes, b, c, tipoProblema, eDuasFases, numVarArtificiais, numVars)
{
    /* Copia o problema original */
    this->aOriginal = e.A;
    this->bOriginal = e.B;
    this->cOriginal = e.C;

    this->idProblema = this->numTotalProblemas; // Identificador deste problema
    this->numTotalProblemas++; // Incrementa o número de problemas executados
}

std::vector<std::vector<double>> SimplexInteiro::getMatrizAOriginal()
{
    return realizaCopiaProfunda(aOriginal);
}

std::vector<double> SimplexInteiro::getVetorBOriginal()
{
    return realizaCopiaProfunda(bOriginal);
}

std::vector<double> SimplexInteiro::getVetorCOriginal()
{
    return realizaCopiaProfunda(cOriginal);
}

std::vector<std::pair<int, double>> SimplexInteiro::getBase()
{
    return realizaCopiaProfunda(base);
}

double SimplexInteiro::getSolucaoOtima()
{
    return solucaoOtima;
}

bool SimplexInteiro::getSemSolucao()
{
    return semSolucao;
}

bool SimplexInteiro::getEIlimitado()
{
    return eIlimitado;
}

bool SimplexInteiro::getTipoProblema()
{
    return tipoProblema;
}

int SimplexInteiro::getNumeroProblema(bool deTodos)
{
    if (deTodos) // Deseja-se o número de problemas criados até a chamada da função
        return numTotalProblemas;

    return idProblema;
}

bool SimplexInteiro::calculaIteracaoSimplex(int iteracao)
{
    if (verificarSolucaoOtima())
        return true;

    int colunaNumPivo = achaColunaPivo();

    int linhaPivo = achaLinhaPivo(colunaNumPivo);

    if (eIlimitado)
        return true;

    realizaPivoteamento(linhaPivo, colunaNumPivo);    

    if (semSolucao)
        return true;

    return false;
}

void SimplexInteiro::aplicaSimplex(std::vector<int> ondeAdicionar)
{
    int iteracao = 1;

    if (eDuasFases)
    {
        bool temSegundaFase = iniciaPrimeiraFase(ondeAdicionar);

        if (!temSegundaFase)
            return;
    }

    bool fim = false;

    std::cout << std::endl;

    while (!fim)
    {
        bool resultado = calculaIteracaoSimplex(iteracao);

        iteracao++;           

        if (resultado)
            fim = true;
    }

    if (!semSolucao && !eIlimitado)
    {
        std::cout << "Matriz de coeficientes e vetores B e C finais: " << std::endl;
        std::cout << "====================================================" << std::endl;
        printMatrizes();

        std::cout << std::endl;

        std::cout << "Variáveis básicas na última iteração: " << std::endl;
        std::cout << "====================================================" << std::endl;

        auto it = base.begin();

        while (it != base.end())
        {
            std::cout << "x" << it->first + 1 << " " << it->second << " " << std::endl;
            it++;
        }

        std::cout << std::endl;

        if (!eMaximizacao && solucaoOtima != 0)
            solucaoOtima *= -1; // A implementação é baseada em maximização. Para obter a solução de uma minimização, basta multiplicar por -1.

        std::cout << "Solução ótima: " << solucaoOtima << std::endl;
        std::cout << "====================================================" << std::endl;
    }    
}

bool SimplexInteiro::iniciaPrimeiraFase(std::vector<int> ondeAdicionar)
{
    C_artificial.resize(colunas - numVarArtificiais, 0); // Os coeficientes do problema original são 0

    for (int i = 1 ; i <= numVarArtificiais ; i++) // As variáveis artificiais entram à direita e são 1
        C_artificial.push_back(1);

    int k = 0;

    for (int i = 0 ; i < linhas ; i++)
    {
        for (int j = numVars ; j < colunas ; j++)
        {
            if (A[i][j] == 1) // Se há 1 nessa linha, a variável é de folga ou artificial, deve entrar na base.
            {
                base.push_back( {j, B[k]} );
                k++;
                break;
            }
        }
    } 

    for (int i = 0 ; i < (int) ondeAdicionar.size() ; i++) // Coloca na forma canônica o tableau
    {
        for (int j = 0 ; j < colunas ; j++)
            C_artificial[j] = C_artificial[j] - A[ondeAdicionar[i]][j]; // Pivoteia a função objetivo artificial com as linhas da base que são da variável artificial
        
        solucaoOtimaPrimeiraFase -= B[ondeAdicionar[i]];
    }

    return realizaPrimeiraFase(); // Função objetivo auxiliar criada e matriz A ajustada. Pronto para começar o procedimento da primeira fase.

}

bool SimplexInteiro::realizaPrimeiraFase()
{
    bool fim = false;
    int iteracao = 1;

    while ( !fim )
    {
        bool resultado = calculaIteracaoSimplex(iteracao);
        iteracao++;           

        if (resultado)
            fim = true;
    }

    double (*funcComp)(double);

    if (solucaoOtimaPrimeiraFase > 0)
        funcComp = std::floor;
    else
        funcComp = std::ceil;
    
    auto resultadoComparacaoZero = funcComp(solucaoOtimaPrimeiraFase * 10e5) / 10e5;

    if (resultadoComparacaoZero == 0 || resultadoComparacaoZero == -0) // Problema original tem solução
    {
        C_artificial.clear();

        for (int i = 0 ; i < numVarArtificiais ; i++)
            C.pop_back(); // Remove as variáveis artificiais da função objetivo original

        for (int i = 0 ; i < linhas ; i++)
        {
            for (int j = 0 ; j < numVarArtificiais ; j++)
                A[i].pop_back(); // Remove as variáveis artificiais da função objetivo original
        }

        colunas = C.size(); // Número de variáveis sem as artificiais
        
        eDuasFases = false; // Encerramos a primeira fase

        return true;
    }

    else
    {
        std::cout << "\nO problema não possui solução.\n"; // Não há por que continuar, encerramos
        std::cout << "====================================================" << std::endl;
        semSolucao = true;
        return false;
    }
}

double (*retornaFuncaoComparacao(double solucaoOtima))(double valor)
{
    if (solucaoOtima > 0) // Se o número é positivo, arredonda-se para baixo
        return std::floor;
    else // Arredonda-se para cima caso contrário
        return std::ceil;
}

void controlaProblemasInteiros(double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal)
{
    while (!fila.empty()) // Enquanto houver problema para ser analisado, prossegue na árvore
    {
        SimplexInteiro problemaMaisAntigo = fila.front().first; // Pega o problema mais antigo na fila

        std::cout << "Problema " << problemaMaisAntigo.getNumeroProblema(false) << std::endl;
        std::cout << "====================================================" << std::endl;
        problemaMaisAntigo.aplicaSimplex(fila.front().second); 

        fila.pop(); // Remove da fila
        verificaSolucaoInteira(problemaMaisAntigo, solucaoOtimaGlobal, solucaoGlobal); // Verifica se irá podar a sub-árvore ou criar novos problemas     
    }
}

bool eInteiro(double num)
{
    return std::ceil(num * 10e7) / 10e7 == std::ceil(num); // Assume como número inteiro se esse tem 7 casas decimais nulas
}

int retornaPosicaoNaoInteiro(std::vector<double> solucao)
{
    for (int i = 0 ; i < solucao.size(); i++)
    {
        if (!eInteiro(solucao[i])) // Se essa coordenada não é inteira, a solução não é inteira. Retornará o índice dela
            return i;
    }
    return -1;
}

void verificaSolucaoInteira(SimplexInteiro problema, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal)
{
    std::vector<std::vector<double>> A = problema.getMatrizAOriginal(); // Retorna a matriz A do problema desse nó antes da resolução
    std::vector<double> B = problema.getVetorBOriginal(); // Retorna o vetor B do problema desse nó antes da resolução
    std::vector<double> C = problema.getVetorCOriginal(); // Retorna o vetor C do problema desse nó antes da resolução
    double solucaoOtimaTeste = problema.getSolucaoOtima(); // Retorna a solução ótima encontrada
    std::vector<std::pair<int, double>> base = problema.getBase(); // Retorna as variáveis básicas desse problema após a resolução  
    bool tipoProblema = problema.getTipoProblema(); // Retorna o tipo de problema
    std::vector<double> solucao(numVariaveisCanonica, 0); // Vetor solução contendo zeros
    double (*funcComp)(double) = retornaFuncaoComparacao(solucaoOtimaTeste); // Função que será usada para arredondar para cima ou para baixo a solução encontrada
    bool comparacaoSolucao; // Indicador se a solução encontrada é menor que a solução incumbente

    for (int i = 0 ; i < base.size() ; i++)
    {
        if (base[i].first < numVariaveisCanonica) // Se o índice for menor que o número de variáveis na forma canônica, ele faz parte da forma canônica
            solucao[base[i].first] = base[i].second; // Coloca no vetor de soluções o valor mapeado
    }

    int posicaoFracionario = retornaPosicaoNaoInteiro(solucao); // Contém a posição da primeira coordenada fracionária encontrada

    if (eInteiro(solucaoOtimaGlobal)) // Se a solução atual é inteira, iremos arredondar a que encontramos para verificação da capacidade de poda.
        comparacaoSolucao = comparaSolucoesExclusive(funcComp(solucaoOtimaTeste), solucaoOtimaGlobal);

    else // Caso contrário, é uma comparação comum
        comparacaoSolucao = comparaSolucoesExclusive(solucaoOtimaTeste, solucaoOtimaGlobal);

    if (comparacaoSolucao || problema.getSemSolucao() || problema.getEIlimitado()) // Poda por inviabilidade ou solução pior que a atual
    {
        std::cout << std::endl << "O problema " << problema.getNumeroProblema(false) << " encerrou por inviabilidade ou por limitação da solução.\n" << std::endl;

        return;
    }

    else if (posicaoFracionario == -1) // Poda de solução inteira encontrada
    {
        /* Se maximização, será a comparação solucaoOtimaGlobal <= solucaoOtimaTeste. Caso contrário, solucaoOtimaGlobal >= solucaoOtimaTeste */

        if (comparaSolucoesInclusive(solucaoOtimaGlobal, solucaoOtimaTeste))
        {
            /* Atualiza a solução incumbente */
            solucaoOtimaGlobal = solucaoOtimaTeste;
            solucaoGlobal = solucao;

            std::cout << std::endl << "O problema " << problema.getNumeroProblema(false) << " encerrou por ter solução inteira e melhor que a atual.\n" << std::endl;
            return;
        }
        std::cout << std::endl << "O problema " << problema.getNumeroProblema(false) << " encerrou por ter uma solução inteira, porém é pior que a atual.\n" << std::endl;
        return;
    }
    
    std::cout << std::endl << "O problema " << problema.getNumeroProblema(false) << " irá se dividir nos problemas " << problema.getNumeroProblema(true)
                << " e " << problema.getNumeroProblema(true) + 1 << std::endl << std::endl;

    criaNovosProblemas(A, B, C, posicaoFracionario, tipoProblema, solucao, solucaoOtimaGlobal, solucaoGlobal);    
}

SimplexInteiro retornaProblema(std::vector<std::vector<double>> A, std::vector<double> B, std::vector<double> C, std::vector<double> solucao,
                                int posicaoNaoInteiro, bool tipoProblema, bool eMenor, std::vector<int> &ondeAdicionar)
{        
    std::vector<double> novaRestricao; // Nova restrição da ramificação

    for (int j = 0 ; j < A[0].size() ; j++) // Coloca os valores na coluna da nova restrição
    {
        if (j == posicaoNaoInteiro) // Se a coluna é a da coordenada fracionária, colocamos 1
            novaRestricao.push_back(1);            
        else // 0 caso contrário
            novaRestricao.push_back(0);
    }

    if (eMenor) // Se a restrição é <=, colocamos 1 como variável de folga e o valor no vetor de soluções será o piso do valor da coordenada
    {
        novaRestricao.push_back(1);
        B.push_back(std::floor(solucao[posicaoNaoInteiro]));
    }

    else // Se a restrição é >=, colocamos -1 como variável de folga e o valor no vetor de soluções será o piso do valor da coordenada acrescido de 1
    {
        novaRestricao.push_back(-1);
        B.push_back(std::floor(solucao[posicaoNaoInteiro]) + 1);
    }    

    for (int i = 0 ; i < A.size() ; i++) // Coloca 0 para representar a variável de folga nas demais restrições
        A[i].push_back(0);    

    C.push_back(0); // Coloca 0 para representar a variável de folga na função objetivo
    A.push_back(novaRestricao);

    auto preparacao = retornaPreparacaoSimplex(A, A.size(), A[0].size(), numVariaveisCanonica); // Prepará para o método de duas fases, se necessário
    ondeAdicionar = preparacao.ondeAdicionar;

    int tamanhoColuna = A[0].size();

    ElementosOriginais e = {realizaCopiaProfunda(A), realizaCopiaProfunda(B), realizaCopiaProfunda(C)}; // Cópia do problema antes da resolução para ser replicado nos nós filhos

    adicionaVariaveisArtificiais(A, C, tamanhoColuna, A.size(), preparacao);

    SimplexInteiro p(A, B, C, tipoProblema, preparacao.eDuasFases, preparacao.numVarArtificiais, numVariaveisCanonica, e);      

    return p;
}

void criaNovosProblemas(std::vector<std::vector<double>> A, std::vector<double> B, std::vector<double> C, int posicaoNaoInteiro, bool tipoProblema, std::vector<double> solucao, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal)
{
    /* Cópia dos elementos, pois a criação de p1 irá modificar. */
    auto aOriginal = realizaCopiaProfunda(A);
    auto bOriginal = realizaCopiaProfunda(B);
    auto cOriginal = realizaCopiaProfunda(C);

    /* Vetores contendo os índices das linhas com variáveis artificiais, que serão repassados para a resolução do problema */
    std::vector<int> ondeAdicionarP1;
    std::vector<int> ondeAdicionarP2;

    /* Cria os problemas */
    SimplexInteiro p1 = retornaProblema(A, B, C, solucao, posicaoNaoInteiro, tipoProblema, true, ondeAdicionarP1);
    SimplexInteiro p2 = retornaProblema(aOriginal, bOriginal, cOriginal, solucao, posicaoNaoInteiro, tipoProblema, false, ondeAdicionarP2);
    
    /* Coloca na fila para busca em largura */
    fila.push({p1, ondeAdicionarP1});
    fila.push({p2, ondeAdicionarP2});    
}

void iniciaProblemaInteiro(Simplex simplex, std::vector<std::vector<double>> aOriginal, std::vector<double> bOriginal, std::vector<double> cOriginal, int numVars)
{
    SimplexInteiro simplexInteiro(simplex, aOriginal, bOriginal, cOriginal); // Inicialização do problema inteiro através do problema original resolvido
    double solucaoOtimaGlobal; // Variável referência para a solução inteira
    std::vector<double> solucaoGlobal(numVars, 0); // Vetor de solução inteira incumbente
    std::vector<double> solucao(numVars, 0); // Vetor de solução do problema original
    std::vector<std::pair<int, double>> base = simplexInteiro.getBase(); // Variáveis básicas do problema original após a resolução
    numVariaveisCanonica = numVars; // Número de variáveis na forma canônica para uso de todas as funções

    if (simplexInteiro.getSemSolucao() || simplexInteiro.getEIlimitado()) // Não há o que analisar, encerramos
    {
        std::cout << "O problema original não possui solução ou é ilimitado.\n" << std::endl;
        return;
    }

    inicializaPonteirosComparacao(simplexInteiro, solucaoOtimaGlobal);  // Inicializa os ponteiros de função para comparação de solução conforme tipo do problema  

    int posicaoFracionario = testaSolucaoOriginal(base, solucao, numVars); // Verifica se a solução original é inteira através do índice retornado

    if (posicaoFracionario == -1) // Se -1, ela é inteira. Encerramos
        return;
    
    /* Caso contrário, iremos ramificar o problema original em busca da solução inteira e iniciar o Branch and Bound */

    criaNovosProblemas(simplexInteiro.getMatrizAOriginal(), simplexInteiro.getVetorBOriginal(), simplexInteiro.getVetorCOriginal(),
                        posicaoFracionario, simplexInteiro.getTipoProblema(), solucao, solucaoOtimaGlobal, solucaoGlobal);

    /* Chama a função que realiza a busca em largura, ou seja, prossegue na árvore */
    controlaProblemasInteiros(solucaoOtimaGlobal, solucaoGlobal);

    /* Exibe os resultados encontrados */
    imprimeSolucaoInteiraFinal(solucaoOtimaGlobal, solucaoGlobal, numVars);    
}

void inicializaPonteirosComparacao(SimplexInteiro simplexInteiro, double &solucaoOtimaGlobal)
{
    if (simplexInteiro.getTipoProblema()) // O problema é de maximização
    {
        solucaoOtimaGlobal = -simplexInteiro.getSolucaoOtima(); // O referencial pode ser inicializado com qualquer valor menor ou igual a solução ótima original
        comparaSolucoesInclusive = [](double num1, double num2){ return num1 <= num2; }; // Expressão que testa a solução incumbente e a solução inteira encontrada
        comparaSolucoesExclusive = [](double num1, double num2){ return num1 < num2; }; // Expressão que testa a solução incumbente e a solução encontrada visando a poda.
    }

    else // O problema é de minimização
    {
        solucaoOtimaGlobal = simplexInteiro.getSolucaoOtima(); // O referencial para minimização é a solução atual. Nenhuma solução nova pode ser maior.
        comparaSolucoesInclusive = [](double num1, double num2){ return num1 >= num2; };
        comparaSolucoesExclusive = [](double num1, double num2){ return num1 > num2; };
    }
}

int testaSolucaoOriginal(std::vector<std::pair<int, double>> base, std::vector<double> &solucao, int numVars)
{
    /* Segue a mesma estratégia da função verificaSolucaoInteira */
    
    for (int i = 0 ; i < base.size() ; i++)
    {
        if (base[i].first < numVars)
            solucao[base[i].first] = base[i].second;
    }

    int posicaoFracionario = retornaPosicaoNaoInteiro(solucao);
    
    if (posicaoFracionario == -1)
        std::cout << "O problema já possui solução inteira.\n";

    return posicaoFracionario;
}

void imprimeSolucaoInteiraFinal(double solucaoOtimaGlobal, std::vector<double> solucaoGlobal, int numVars)
{
    int contadorZero = 0;

    for (int i = 0 ; i < solucaoGlobal.size() ; i++)
    {
        if (solucaoGlobal[i] == 0)
            contadorZero++;
    }

    if (contadorZero == numVars) // Como o vetor é inicializado com zeros, se todas as coordenadas são 0, então ele não foi modificado. Atribuímos zero à solução ótima.
        solucaoOtimaGlobal = 0;

    std::cout << "Solução ótima inteira para o problema: ";

    for (int i = 0 ; i < solucaoGlobal.size() ; i++)
        std::cout << solucaoGlobal[i] << " ";

    std::cout << std::endl;

    std::cout << "Solução ótima aproximada para a solução inteira: ";

    std::cout << solucaoOtimaGlobal << std::endl;
}