#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "RechargeStation.hpp"
#include "HashTable.hpp"
#include "PointQuadTree.hpp"
#include "quicksort.hpp"
#include "PointID.hpp"

using namespace std;

// Função para inserção recursiva na QuadTree
void insertRecursive(PointID *stations, QuadTree &quadtree, int l, int r)
{
   if (l > r)
   {
      return;
   }

   int mid = l + (r - l) / 2;
   quadtree.insert({stations[mid].p.x, stations[mid].p.y}, stations[mid].id);

   insertRecursive(stations, quadtree, l, mid - 1);
   insertRecursive(stations, quadtree, mid + 1, r);
}

// Função para inserir na QuadTree de forma balanceada
void Insert_Quadtree(PointID *Stations, QuadTree &quadtree, int l, int r)
{
   MedianInsQuickSort(Stations, l, r);
   insertRecursive(Stations, quadtree, l, r);
}

int main()
{

   cout << fixed << setprecision(3); // Precisão 3 casas decimais

   // LEITURA DOS DADOS DAS ESTAÇÔES

   std::string filename = "geracarga.base";
   std::ifstream file(filename);

   if (!file.is_open()) // ABRE ARQUIVO
   {
      std::cerr << "Erro ao abrir o arquivo de estações" << std::endl;
      return -1;
   }

   std::string firstLine;
   std::getline(file, firstLine, '\n');
   int numLines = std::stoi(firstLine); // NUMERO DE LINHAS

   HashTable tabela((int)(1.4 * numLines));

   QuadTree quadtree(numLines);
   PointID *Stations = new PointID[numLines];

   std::string line;
   for (int i = 0; i < numLines; i++)
   {
      RechargeStation station;
      std::getline(file, line);

      std::stringstream ss(line);
      std::string notstring;

      std::getline(ss, station.id_end, ';'); // para cada campo, lê a string até o próximo ';'
      std::getline(ss, notstring, ';');
      station.id_lograd = std::stol(notstring);
      std::getline(ss, station.sigla_tipo, ';');
      std::getline(ss, station.nome_lograd, ';');
      std::getline(ss, notstring, ';');
      station.num_imovel = std::stoi(notstring);
      std::getline(ss, station.nome_bairro, ';');
      std::getline(ss, station.nome_regiao, ';');
      std::getline(ss, notstring, ';');
      station.cep = std::stoi(notstring);
      std::getline(ss, notstring, ';');
      station.x = std::stod(notstring);
      std::getline(ss, notstring, ';');
      station.y = std::stod(notstring);

      tabela.Inserir(station);
      Stations[i] = {station.id_end, {station.x, station.y}};
   }

   file.close();

   Insert_Quadtree(Stations, quadtree, 0, numLines - 1); // Insere de forma balanceada na árvore

   delete[] Stations;

   // LEITURA DAS CONSULTAS
   ifstream queryFile("geracarga.ev"); // ABRE ARQUIVO
   if (!queryFile.is_open())
   {
      cerr << "Erro ao abrir o arquivo de consultas" << endl;
      return -1;
   }

   std::getline(queryFile, firstLine);
   numLines = std::stoi(firstLine); // NUMERO DE LINHAS

   for (int i = 0; i < numLines; i++)
   {
      std::getline(queryFile, line);
      stringstream ss(line);
      string action, id, notstring;
      double x, y;
      int n;

      getline(ss, action, ' ');

      cout << action << " ";

      if (action == "A")
      {
         getline(ss, id, ' '); // Pega id

         cout << id << endl;
         RechargeStation station;
         tabela.Pesquisa(id, station); // Pesquisa id na tabela para pegar coordenadas em O(1)
         if (quadtree.activate({station.x, station.y}, id))
         {
            cout << "Ponto de recarga " << id << " ativado." << endl;
         }
         else
         {
            cout << "Ponto de recarga " << id << " já estava ativo." << endl;
         }
      }
      else if (action == "D")
      {
         getline(ss, id, ' '); // Pega id
         cout << id << endl;
         RechargeStation station;
         tabela.Pesquisa(id, station); // Pesquisa id na tabela para pegar coordenadas em O(1)

         if (quadtree.deactivate({station.x, station.y}, id))
         {
            cout << "Ponto de recarga " << id << " desativado." << endl;
         }
         else
         {
            cout << "Ponto de recarga " << id << " já estava desativado." << endl;
         }
      }
      else if (action == "C")
      {
         getline(ss, notstring, ' ');
         x = stod(notstring);
         getline(ss, notstring, ' ');
         y = stod(notstring);
         getline(ss, notstring, ' ');
         n = stoi(notstring);
         Point p = {x, y};

         cout << x << " " << y << " " << n << endl;

         Distpair *nn = quadtree.knn(p, n);

         for (int i = 0; i < n; i++)
         {
            RechargeStation station;
            tabela.Pesquisa(nn[i].id, station);

            cout << station.sigla_tipo << ' ' << station.nome_lograd << ", " << station.num_imovel << ", "
                 << station.nome_bairro << ", " << station.nome_regiao << ", " << station.cep
                 << " (" << nn[i].dist << ")" << endl;
         }
      }
   }

   queryFile.close();

   return 0;
}
