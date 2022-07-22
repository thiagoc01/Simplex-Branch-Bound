#include <iostream>
#include <cmath>
#include <vector>
#include <iterator>
#include <limits>
#include <algorithm>

#include "../util/preparador.hpp"
#include "simplex.hpp"

Simplex::Simplex (std::vector <std::vector<double>> coeficientes, std::vector<double> b, std::vector<double> c, bool tipoProblema, bool eDuasFases, int numVarArtificiais, int numVars)
{
    solucaoOtima = solucaoOtimaPrimeiraFase = 0;
    eIlimitado = false;
    semSolucao = false;
    this->eDuasFases = eDuasFases;
    this->numVarArtificiais = numVarArtificiais;
    this->numVars = numVars;
    linhas = coeficientes.size();
    this->tipoProblema = tipoProblema;
    colunas = coeficientes[0].size(); // O tamanho de uma linha indica o número de variáveis no problema.
    
    B.assign(b.begin(), b.end()); // Inicializa o vetor B.

    if (tipoProblema) // Se o problema for de maximização, basta copiar o vetor C e setar a booleana.
    {
        C.assign(c.begin(), c.end());
        eMaximizacao = true;
    }

    else // Caso contrário, é necessário multiplicar a linha inteira por -1 para obtermos o problema equivalente e mantermos o mesmo código.
    {
        for (int i = 0 ; i < (int) c.size() ; i++)
        {
            if (c[i] != 0)
                C.push_back(c[i] * -1);
            else
                C.push_back(0);
        }
        
        eMaximizacao = false;
    }
    
    for (int i = 0 ; i < linhas ; i++)
        A.push_back(coeficientes[i]); // Inicializa a matriz A.

    
    if (!eDuasFases) // Se o método tem primeira fase, a função iniciaPrimeiraFase trata as bases
    {
        int j = 0;

        for (int i = numVars; i < (int) C.size() ; i++)
        {
            if (!C[i]) // As variáveis básicas têm coeficiente 0 no início do problema.
            {
                base.push_back( {i, B[j]} );
                j++;
            }
        }

        if (base.size() < linhas)
            throw std::runtime_error("O conjunto de vetores na base é insuficiente para a resolução do problema. Verifique a entrada.");
    }
}

bool Simplex::calculaIteracaoSimplex(int iteracao)
{
    if (verificarSolucaoOtima())
        return true;

    int colunaNumPivo = achaColunaPivo();

    int linhaPivo = achaLinhaPivo(colunaNumPivo);

    if (eIlimitado)
    {
        imprimeInformacao("Solução ilimitada.\n\n");
        return true;
    }

    realizaPivoteamento(linhaPivo, colunaNumPivo);

    if (semSolucao)
    {
        imprimeInformacao("O problema não possui solucão.\n");
        return true;
    }

    if (eDuasFases)
    {
        imprimeInformacao("Matriz de coeficientes e vetores B, C e C auxiliar na iteração " + std::to_string(iteracao) + "\n");
        imprimeInformacao("====================================================\n");
    }

    else
    {
        imprimeInformacao("Matriz de coeficientes e vetores B e C na iteração " + std::to_string(iteracao) + "\n");
        imprimeInformacao("====================================================\n");
    }    

    printMatrizes();

    imprimeInformacao("\n");

    imprimeInformacao("Variáveis básicas na iteração " + std::to_string(iteracao) + "\n");
    imprimeInformacao("====================================================\n");

    auto it = base.begin();

    while (it != base.end())
    {
        imprimeInformacao("x");
        imprimeInformacao(it->first + 1);
        imprimeInformacao(" ");
        imprimeInformacao(it->second);
        imprimeInformacao(" \n");
        it++;
    }

    imprimeInformacao("\n");

    if (eDuasFases)
    {
        imprimeInformacao("Solução do PPL auxiliar na iteração " + std::to_string(iteracao) + "\n");
        imprimeInformacao("====================================================\n");     
        imprimeInformacao(solucaoOtimaPrimeiraFase);
        imprimeInformacao("\n\n");
    }

    imprimeInformacao("Solução do PPL na iteração " + std::to_string(iteracao) + "\n");
    imprimeInformacao("====================================================\n");

    if (!eMaximizacao && solucaoOtima != 0)
        imprimeInformacao(solucaoOtima * -1);
    else
        imprimeInformacao(solucaoOtima);

    imprimeInformacao("\n\n");    

    return false;
}

bool Simplex::verificarSolucaoOtima()
{
    bool eOtima = false;
    int contagemNumPositivos = 0;

    if (eDuasFases)
    {
        for (int i = 0 ; i < (int) C_artificial.size() ; i++)
        {
            if (ceil(C_artificial[i] * 10000000000)/ 10000000000 >= 0)
                contagemNumPositivos++;
        }

        if (contagemNumPositivos == (int) C_artificial.size())
            eOtima = true;
    }

    else
    {
        for (int i = 0 ; i < (int) C.size() ; i++)
        {
            if (ceil(C[i] * 10000000000)/ 10000000000 >= 0)
                contagemNumPositivos++;
        }

        if (contagemNumPositivos == (int) C.size())
            eOtima = true;
    }   

    return eOtima;
}

void Simplex::realizaPivoteamento(int linhaPivo, int colunaNumPivo)
{
    double numPivo = A[linhaPivo][colunaNumPivo];

    if (eDuasFases)
        solucaoOtimaPrimeiraFase = solucaoOtimaPrimeiraFase - (C_artificial[colunaNumPivo] * (B[linhaPivo] / numPivo));

    solucaoOtima = solucaoOtima - (C[colunaNumPivo] * (B[linhaPivo] / numPivo));

    if (!(std::fabs(numPivo - 1) <= std::numeric_limits<double>::epsilon()))
    {
        for (int k = 0 ; k < colunas ; k++)
            A[linhaPivo][k] = A[linhaPivo][k] / numPivo; // Divide a linha pivô pelo número pivô.
    }    

    B[linhaPivo] = B[linhaPivo] / numPivo; // O vetor B está separado, realiza a mesma ação anterior.

    base[linhaPivo] = {colunaNumPivo, B[linhaPivo]};  // Altera a base correspondente a essa linha.

    for (int i = 0 ; i < (int) B.size() ; i++)
    {
        if (i != linhaPivo)
        {
            double multiplicadorLinha =  A[i][colunaNumPivo]; // Capturamos o elemento que faz zerar o elemento B dessa linha.

            if (multiplicadorLinha == 0)
            {
                if (B[i] < 0) // Se algum B[i] é menor que 0, o problema é inviável.
                    semSolucao = true;
                continue;
            }

            B[i] = B[i] - (multiplicadorLinha * B[linhaPivo]); // Atualiza o B_i

            if (B[i] < 0)
                semSolucao = true;

            base[i] = {base[i].first, B[i]}; // Atualiza no vetor de bases o valor de B_i.
        }
    } 

    for (int m = 0 ; m < linhas ; m++)
    {
        double multiplicadorLinha = A[m][colunaNumPivo]; // Capturamos o elemento que faz zerar o elemento da coluna pivô dessa linha.

        if (multiplicadorLinha == 0)
            continue;

        if (m != linhaPivo)
        {
            for (int p = 0 ; p < colunas ; p++)                        
                A[m][p] = A[m][p] - (multiplicadorLinha * A[linhaPivo][p]);  // Atualiza cada elemento da linha, realizando o pivoteamento.                 
        }
    }

    /* Processo análogo para o vetor de coeficientes da função objetivo. */    
    double multiplicadorLinha = C[colunaNumPivo];

    if (multiplicadorLinha != 0)
    {
        for (int i = 0 ; i < colunas ; i++)
            C[i] = C[i] - (multiplicadorLinha * A[linhaPivo][i]);
    }    

    if (eDuasFases) // Se estamos na primeira fase, é necessário trabalhar com a função objetivo artificial.
    {
        multiplicadorLinha = C_artificial[colunaNumPivo];

        if (multiplicadorLinha != 0)
        {
            for (int i = 0 ; i < (int) C_artificial.size() ; i++)
                C_artificial[i] = C_artificial[i] - (multiplicadorLinha * A[linhaPivo][i]);
        }        
    }
}

void Simplex::printMatrizes()
{
    std::cout << "Matriz A: \n";

    for (int i = 0 ; i < linhas ; i++)
    {
        for (int j = 0 ; j < colunas ; j++)
            std::cout << A[i][j] << " | ";
        std::cout << std::endl;
    }

    std::cout << "Vetor B: \n";
    for (int i = 0 ; i < (int) B.size() ; i++)
        std::cout << B[i] << " | ";

    std::cout << std::endl;

    std::cout << "Vetor C: \n";
    for (int i = 0 ; i < (int) C.size() ; i++)
        std::cout << C[i] << " | ";
    std::cout << std::endl;

    if (eDuasFases)
    {
        std::cout << "Vetor C artificial: \n";
        for (int i = 0 ; i < (int) C_artificial.size() ; i++)
            std::cout << C_artificial[i] << " | ";
    }
    std::cout << std::endl;
}

int Simplex::achaColunaPivo()
{
    /* Inicializamos tomando a primeira posição como o menor elemento */

    int localizacao = 0;
    double menor;

    if (eDuasFases)
    {
        menor = C_artificial[0];

        for (int i = 1 ; i < (int) C_artificial.size() ; i++) // Realiza um procedimento comum para encontrar o menor
        {
            if (C_artificial[i] <= menor)
            {
                menor = C_artificial[i];
                localizacao = i;
            }
        }
    }

    else
    {
        menor = C[0];

        for (int i = 1 ; i < (int) C.size() ; i++) // Realiza um procedimento comum para encontrar o menor
        {
            if (C[i] <= menor)
            {
                menor = C[i];
                localizacao = i;
            }
        }
    }    

    return localizacao;
}

int Simplex::achaLinhaPivo(int colunaNumPivo)
{
    int contagemNumNegativos = 0;
    double minimo = std::numeric_limits<double>::max();
    int localizacao = 0;

    for (int i = 0 ; i < linhas ; i++)
    {
        if (A[i][colunaNumPivo] <= 0)
            contagemNumNegativos++;
    }

    if (contagemNumNegativos == linhas) // Critério de parada do Simplex. Se todos os coeficientes são 0 na função objetivo.
    {
        eIlimitado = true;
        return -1;
    }

    for (int i = 0 ; i < linhas ; i++)
    {
        if (A[i][colunaNumPivo] > 0) // Iremos testar apenas linhas que possuem coeficientes positivos na coluna pivô.
        {
            if (B[i] / A[i][colunaNumPivo] <= minimo) // Devemos encontrar o mínimo.
            {              
                minimo =  B[i] / A[i][colunaNumPivo];
                localizacao = i;
            }
        }
    }

    return localizacao;
}

bool Simplex::iniciaPrimeiraFase(std::vector<int> ondeAdicionar)
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

    if (base.size() < linhas)
            throw std::runtime_error("O conjunto de vetores na base é insuficiente para a resolução do problema. Verifique a entrada.");

    for (int i = 0 ; i < (int) ondeAdicionar.size() ; i++) // Coloca na forma canônica o tableau
    {
        for (int j = 0 ; j < colunas ; j++)
            C_artificial[j] = C_artificial[j] - A[ondeAdicionar[i]][j]; // Pivoteia a função objetivo artificial com as linhas da base que são da variável artificial
        
        solucaoOtimaPrimeiraFase -= B[ondeAdicionar[i]];
    }

    printMatrizes();
    imprimeInformacao("\n\n");

    return realizaPrimeiraFase(); // Função objetivo auxiliar criada e matriz A ajustada. Pronto para começar o procedimento da primeira fase.

}

bool Simplex::realizaPrimeiraFase()
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
    imprimeInformacao("Fim da primeira fase.\n\n\n");

    double (*funcComp)(double);

    if (solucaoOtimaPrimeiraFase > 0)
        funcComp = std::floor;
    else
        funcComp = std::ceil;
    
    auto resultadoComparacaoZero = funcComp(solucaoOtimaPrimeiraFase * 10e5) / 10e5;

    if (resultadoComparacaoZero == 0 || resultadoComparacaoZero == -0) // Problema original tem solução
    {
        imprimeInformacao("O problema pode possuir alguma solução.\n\n");
        imprimeInformacao("====================================================\n\n");
        imprimeInformacao("Iniciando a segunda fase...\n\n\n");

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
        imprimeInformacao("O problema não possui solução.\n");
        imprimeInformacao("====================================================\n\n");
        semSolucao = true;
        return false;
    }
}

void Simplex::aplicaSimplex(std::vector<int> ondeAdicionar)
{
    int iteracao = 1;

    if (eDuasFases)
    {
        imprimeInformacao("O método de duas fases deve ser aplicado. Iniciando primeira fase... \n\n\n");
        imprimeInformacao("Matriz de coeficientes e vetores B, C e C artificial iniciais: \n");
        imprimeInformacao("====================================================\n");
        bool temSegundaFase = iniciaPrimeiraFase(ondeAdicionar);

        if (!temSegundaFase)
            return;
    }

    bool fim = false;

    imprimeInformacao("Matriz de coeficientes e vetores B e C iniciais: \n");
    imprimeInformacao("====================================================\n");
    printMatrizes();

    imprimeInformacao("\n");

    while (!fim)
    {
        bool resultado = calculaIteracaoSimplex(iteracao);

        iteracao++;           

        if (resultado)
            fim = true;
    }

    realizaImpressaoFinal();     
}

void Simplex::realizaImpressaoFinal()
{
    if (!semSolucao && !eIlimitado)
    {
        std::cout << "Matriz de coeficientes e vetores B e C finais: " << std::endl;
        std::cout << "====================================================" << std::endl;
        printMatrizesFinais();

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

void Simplex::imprimeInformacao(std::string informacao)
{
    std::cout << informacao;
}

void Simplex::imprimeInformacao(double informacao)
{
    std::cout << informacao;
}

void Simplex::printMatrizesFinais()
{
    printMatrizes();
}