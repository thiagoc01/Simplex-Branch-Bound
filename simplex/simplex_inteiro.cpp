#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>

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

static std::mutex mutexFila; // Mutex para acesso à fila
static std::mutex mutexSolucao; // Mutex para acesso às variáveis de solução incumbente
static std::mutex mutexNumProblema; // Mutex para acessar o membro estático da classe SimplexInteiro
static std::mutex mutexProblemas; // Mutex para alterar o número de problemas em execução
static std::mutex mutexFim; // Mutex para acessar a variável de fim
static std::mutex mutexVetorProblemas; // Mutex para acessar o vetor de problemas encerrados
static std::condition_variable temElemento; // Condicional para avisar que há elemento na fila

static bool fim = false; // Indicador que todos os nós foram podados

int problemasExecutando = 0; // Contador para indicar o número de problemas em aberto

std::vector<std::thread> threads; // Vetor que contém as 5 threads que concorrem pela fila

std::vector<SimplexInteiro> problemasEncerrados; // Vetor que contém todos os problemas encerrados para informação futura

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

int* SimplexInteiro::getDivisoesProblema()
{
    return divisoesProblema;
}

short int SimplexInteiro::getTipoPoda()
{
    return tipoPoda;
}

void SimplexInteiro::setTipoPoda(int tipo)
{
    tipoPoda = tipo;
}

void SimplexInteiro::setDivisoesProblema(int divisoes[2])
{
    divisoesProblema[0] = divisoes[0];
    divisoesProblema[1] = divisoes[1];
}

void SimplexInteiro::setNumeroProblema(int id)
{
    idProblema = id;
}

void SimplexInteiro::aumentaQuantidadeProblemas()
{
    numTotalProblemas += 2;
}

void SimplexInteiro::imprimeInformacao(std::string informacao)
{

}

void SimplexInteiro::imprimeInformacao(double informacao)
{

}

void SimplexInteiro::printMatrizes()
{

}

void SimplexInteiro::printMatrizesFinais()
{
    Simplex::printMatrizes();
}

void SimplexInteiro::realizaImpressaoFinal()
{
    Simplex::realizaImpressaoFinal();
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

    while (!fim)
    {
        bool resultado = calculaIteracaoSimplex(iteracao);

        iteracao++;           

        if (resultado)
            fim = true;
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
    while (true) // Continua até haver problemas na fila
    {             
        std::unique_lock<std::mutex> mutexUnico(mutexFila); // Lock para verificar a fila

        if (!fila.empty()) // Enquanto houver problema para ser analisado, prossegue na árvore
        {
            SimplexInteiro problemaMaisAntigo = fila.front().first; // Pega o problema mais antigo na fila
            std::vector<int> ondeAdicionar = fila.front().second;
            fila.pop(); // Remove da fila

            mutexUnico.unlock(); // Acessou a fila, libera

            problemaMaisAntigo.aplicaSimplex(ondeAdicionar); // Aplica o Simplex paralelamente

            mutexProblemas.lock();
            problemasExecutando--; // Simplex aplicado nesse objeto, menos um problema executando
            mutexProblemas.unlock();        
            
            verificaSolucaoInteira(problemaMaisAntigo, solucaoOtimaGlobal, solucaoGlobal); // Verifica se irá podar a sub-árvore ou criar novos problemas
            continue;            
        }

        mutexFim.lock();

        if (!fim) // Se não chegou no fim do Branch and Bound, irá aguardar até a fila possuir elementos, caso haja prevalência de threads pelo consumo
        {
            mutexFim.unlock();
            temElemento.wait(mutexUnico, []{ return fila.size() != 0 || fim; });
        }
        else // Caso contrário, encerrou. Saímos do while incondicional
        {
            mutexFim.unlock();
            break;
        }        
    }

    temElemento.notify_one(); // Efetua efeito dominó, para acordar as threads aguardando pela fila com elementos ou pelo fim
    
}

bool eInteiro(double num)
{
    return std::ceil(num * 10e7) / 10e7 == std::ceil(num); // Assume como número inteiro se esse tem 7 casas decimais nulas
}

int retornaPosicaoNaoInteiro(std::vector<double> solucao)
{
    for (std::vector<double>::size_type i = 0 ; i < solucao.size(); i++)
    {
        if (!eInteiro(solucao[i])) // Se essa coordenada não é inteira, a solução não é inteira. Retornará o índice dela
            return i;
    }
    return -1;
}

/**
 * @brief Reduz a quantidade de problemas executando. Se necessário, altera o estado da variável fim para indicar que todos os nós foram podados.
 * 
 */

static void reduzProblemasExecutando()
{
    mutexProblemas.lock();
    if (problemasExecutando == 0)
    {
        mutexFim.lock();
        fim = true;
        mutexFim.unlock();
    }
    mutexProblemas.unlock();
}

/**
 * @brief Recebe uma solução inteira e verifica se ela é melhor que a incumbente
 * 
 * @param problema O problema que originou a solução
 * @param solucaoOtimaGlobal Solução incumbente
 * @param solucaoGlobal Coordenadas da solução incumbente
 * @param solucaoOtimaTeste Solução do problema a ser testada
 * @param solucao Coordenadas da solução do problema a ser testada
 */

static void realizaTratamentoSolucaoInteira(SimplexInteiro problema, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal, double solucaoOtimaTeste, std::vector<double> solucao)
{
    mutexSolucao.lock(); // Por tratar de uma variável compartilhada, devemos travar

    /* Se maximização, será a comparação solucaoOtimaGlobal <= solucaoOtimaTeste. Caso contrário, solucaoOtimaGlobal >= solucaoOtimaTeste */

    if (comparaSolucoesInclusive(solucaoOtimaGlobal, solucaoOtimaTeste))
    {
        /* Atualiza a solução incumbente */
        solucaoOtimaGlobal = solucaoOtimaTeste;
        solucaoGlobal = solucao;
        mutexSolucao.unlock();

        reduzProblemasExecutando(); 

        problema.setTipoPoda(2); // Poda por ser solução inteira e melhor que a incumbente

        mutexVetorProblemas.lock();
        problemasEncerrados.push_back(problema);
        mutexVetorProblemas.unlock();

        return;
    }

    mutexSolucao.unlock();
    reduzProblemasExecutando();

    problema.setTipoPoda(3); // Poda por ser solução inteira e pior que a incumbente

    mutexVetorProblemas.lock();
    problemasEncerrados.push_back(problema);
    mutexVetorProblemas.unlock();
}

bool deveRealizarPoda(SimplexInteiro problema, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal, std::vector<double> solucao,
                                        double solucaoOtimaTeste, int posicaoFracionario)
{
    double (*funcComp)(double) = retornaFuncaoComparacao(solucaoOtimaTeste); // Função que será usada para arredondar para cima ou para baixo a solução encontrada
    bool comparacaoSolucao; // Indicador se a solução encontrada é menor que a solução incumbente

    mutexSolucao.lock();

    if (eInteiro(solucaoOtimaGlobal)) // Se a solução atual é inteira, iremos arredondar a que encontramos para verificação da capacidade de poda.
        comparacaoSolucao = comparaSolucoesExclusive(funcComp(solucaoOtimaTeste), solucaoOtimaGlobal);

    else // Caso contrário, é uma comparação comum
        comparacaoSolucao = comparaSolucoesExclusive(solucaoOtimaTeste, solucaoOtimaGlobal);
    
    mutexSolucao.unlock();

    if (comparacaoSolucao || problema.getSemSolucao() || problema.getEIlimitado()) // Poda por inviabilidade ou solução pior que a atual
    {
        problema.setTipoPoda(1); // Poda por ser uma solução inviável ou pior que a incumbente

        mutexVetorProblemas.lock();
        problemasEncerrados.push_back(problema);
        mutexVetorProblemas.unlock();

        reduzProblemasExecutando();

        return true;
    }

    else if (posicaoFracionario == -1) // Poda de solução inteira encontrada
    {
        realizaTratamentoSolucaoInteira(problema, solucaoOtimaGlobal, solucaoGlobal, solucaoOtimaTeste, solucao);
        
        return true;
    }    

    return false;
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

    for (std::vector<double>::size_type i = 0 ; i < base.size() ; i++)
    {
        if (base[i].first < numVariaveisCanonica) // Se o índice for menor que o número de variáveis na forma canônica, ele faz parte da forma canônica
            solucao[base[i].first] = base[i].second; // Coloca no vetor de soluções o valor mapeado
    }

    int posicaoFracionario = retornaPosicaoNaoInteiro(solucao); // Contém a posição da primeira coordenada fracionária encontrada

    if (deveRealizarPoda(problema, solucaoOtimaGlobal, solucaoGlobal, solucao, solucaoOtimaTeste, posicaoFracionario)) 
        return; // Algum dos três critérios de poda foi atendido

    problema.setTipoPoda(0); // 0 = não encerrou

    mutexNumProblema.lock();

    int divisoes[2] = {problema.getNumeroProblema(true), problema.getNumeroProblema(true) + 1}; // Ramificações desse problema
    problema.setDivisoesProblema(divisoes); // Guarda a informação das ramificações desse nó
    problema.aumentaQuantidadeProblemas(); // Mais dois novos problemas surgirão

    mutexNumProblema.unlock();

    mutexVetorProblemas.lock();
    problemasEncerrados.push_back(problema); // Coloca na marcação de encerramento
    mutexVetorProblemas.unlock();  

    criaNovosProblemas(A, B, C, posicaoFracionario, tipoProblema, solucao, solucaoOtimaGlobal, solucaoGlobal, divisoes);    
}

SimplexInteiro retornaProblema(std::vector<std::vector<double>> A, std::vector<double> B, std::vector<double> C, std::vector<double> solucao,
                                int posicaoNaoInteiro, bool tipoProblema, bool eMenor, std::vector<int> &ondeAdicionar)
{        
    std::vector<double> novaRestricao; // Nova restrição da ramificação

    for (std::vector<double>::size_type j = 0 ; j < A[0].size() ; j++) // Coloca os valores na coluna da nova restrição
    {
        if ((int) j == posicaoNaoInteiro) // Se a coluna é a da coordenada fracionária, colocamos 1
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

    for (std::vector<double>::size_type i = 0 ; i < A.size() ; i++) // Coloca 0 para representar a variável de folga nas demais restrições
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

void criaNovosProblemas(std::vector<std::vector<double>> A, std::vector<double> B, std::vector<double> C, int posicaoNaoInteiro, bool tipoProblema, std::vector<double> solucao, double &solucaoOtimaGlobal, std::vector<double> &solucaoGlobal, int divisoes[2])
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

    /* Os IDs dos problemas são os fornecidos para a função, já que a concorrência entre as threads prejudica o mapeamento correto. */
    p1.setNumeroProblema(divisoes[0]);
    p2.setNumeroProblema(divisoes[1]);

    mutexProblemas.lock();
    problemasExecutando += 2; // Aumenta a quantidade de problemas em aberto
    mutexProblemas.unlock();

    std::unique_lock<std::mutex> mutexUnico(mutexFila); // Trava para colocar problemas na fila

    /* Coloca na fila para busca em largura */

    fila.push({p1, ondeAdicionarP1});
    fila.push({p2, ondeAdicionarP2});

    temElemento.notify_one(); // Notifica que há problema na fila
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

    int idsPrimeiroNos[] = {1, 2};
    simplexInteiro.aumentaQuantidadeProblemas();
    criaNovosProblemas(simplexInteiro.getMatrizAOriginal(), simplexInteiro.getVetorBOriginal(), simplexInteiro.getVetorCOriginal(),
                        posicaoFracionario, simplexInteiro.getTipoProblema(), solucao, solucaoOtimaGlobal, solucaoGlobal, idsPrimeiroNos);

    /* Cria as 5 threads que irão concorrer pelos problemas na fila, realizando a busca em largura */

    for (int i = 0 ; i < 5 ; i++)
    {
        threads.push_back(std::thread(controlaProblemasInteiros, std::ref(solucaoOtimaGlobal), std::ref(solucaoGlobal)));
        if (!threads[i].joinable())
        {
            std::cout << "Ocorreu um erro ao criar a thread " << i + 1 << std::endl; 
            exit(1);
        }
    }

    for (int i = 0 ; i < 5 ; i++)
        threads[i].join();

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
    
    for (std::vector<double>::size_type i = 0 ; i < base.size() ; i++)
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

    std::sort(problemasEncerrados.begin(), problemasEncerrados.end(),
                [](SimplexInteiro s1, SimplexInteiro s2){ return s1.getNumeroProblema(false) < s2.getNumeroProblema(false); });

    std::cout << std::endl;

    for (auto p : problemasEncerrados)
    {
        std::cout << "Problema " << p.getNumeroProblema(false) << std::endl;

        if (!p.getEIlimitado() && !p.getSemSolucao())
        {            
            std::cout << "====================================================\n" << std::endl;

            p.realizaImpressaoFinal();
        }
        
        else
            std::cout << "====================================================" << std::endl;

        std::cout << std::endl;

        if (p.getTipoPoda() == 0)
            std::cout << "O problema " << p.getNumeroProblema(false) << " se dividiu nos problemas " << p.getDivisoesProblema()[0]
                << " e " << p.getDivisoesProblema()[1] << std::endl << std::endl;
        
        else if (p.getTipoPoda() == 1)
            std::cout << "O problema " << p.getNumeroProblema(false) << " encerrou por inviabilidade ou por limitação da solução.\n" << std::endl;

        else if (p.getTipoPoda() == 2)
            std::cout << "O problema " << p.getNumeroProblema(false) << " encerrou por ter solução inteira e melhor que a atual.\n" << std::endl;

        else if (p.getTipoPoda() == 3)
            std::cout << "O problema " << p.getNumeroProblema(false) << " encerrou por ter uma solução inteira, porém é pior que a atual.\n" << std::endl;
    }

    for (std::vector<double>::size_type i = 0 ; i < solucaoGlobal.size() ; i++)
    {
        if (solucaoGlobal[i] == 0)
            contadorZero++;
    }

    if (contadorZero == numVars) // Como o vetor é inicializado com zeros, se todas as coordenadas são 0, então ele não foi modificado. Atribuímos zero à solução ótima.
        solucaoOtimaGlobal = 0;

    std::cout << "Solução ótima inteira para o problema: ";

    for (std::vector<double>::size_type i = 0 ; i < solucaoGlobal.size() ; i++)
        std::cout << solucaoGlobal[i] << " ";

    std::cout << std::endl;

    std::cout << "Solução ótima aproximada para a solução inteira: ";

    std::cout << solucaoOtimaGlobal << std::endl;
}

