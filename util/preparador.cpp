#include "preparador.hpp"


PreparacaoSimplex retornaPreparacaoSimplex(std::vector< std::vector<double>> a, int tamanhoLinhaA, int tamanhoColunaA, int numVars)
{
    PreparacaoSimplex ret; 

    bool eDesigualdade = false;

    for (int i = 0 ; i < tamanhoLinhaA ; i++)
    {
        for (int j = 0 ; j < tamanhoColunaA ; j++)
        {
            if (a[i][j] < 0 && j >= numVars) // Se houver algum coeficiente negativo nas variáveis de folga, temos uma restrição >=
            {
                ret.eDuasFases = true;
                ret.ondeAdicionar.push_back(i);
                eDesigualdade = true;
                ret.numVarArtificiais++;              
            }
            else if (a[i][j] == 1 && j >= numVars)
            {
                eDesigualdade = true;
            }
        }

        if (!eDesigualdade) // Se todas as variáveis de folga são 0, então temos uma restrição =.
        {
            ret.eDuasFases = true;
            ret.ondeAdicionar.push_back(i);
            ret.numVarArtificiais++;
        }

        eDesigualdade = false;
    }

    return ret;
}

void adicionaVariaveisArtificiais(std::vector<std::vector<double>> &a, std::vector<double> &c, int &tamanhoColunaA, int tamanhoLinhaA, PreparacaoSimplex preparador)
{
    for (int k = 0 ; k < (int) preparador.ondeAdicionar.size() ; k++)
    {
        for (int i = 0 ; i < tamanhoLinhaA ; i++)
        {
            if (i != preparador.ondeAdicionar[k]) // Se não estiver no vetor, é uma restrição de menor ou igual que
                a[i].push_back(0);
            else // Caso contrário, é uma restrição maior ou igual que ou uma igualdade
                a[i].push_back(1);
        }
        tamanhoColunaA++; // Aumenta o número de coeficientes no problema
        c.push_back(0);
    }
}
