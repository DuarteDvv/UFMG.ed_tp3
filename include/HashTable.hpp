#ifndef HASH_HPP
#define HASH_HPP

#include "RechargeStation.hpp"

enum Estado
{
   VAZIO,
   OCUPADO,
   REMOVIDO
};

struct ElementoTabela
{
   RechargeStation *endereco; // Ponteiro para economizar memória
   Estado estado;

   ElementoTabela() : endereco(nullptr), estado(VAZIO) {}
   ~ElementoTabela() { delete endereco; } // Desaloca o ponteiro
};

class HashTable
{
public:
   HashTable(int tamanho);
   ~HashTable();

   void Inserir(const RechargeStation &s);
   bool Pesquisa(const std::string &s, RechargeStation &result);

private:
   int tamanhoTabela;
   ElementoTabela *tabela;

   int Hash(const std::string &s);
   int Probing(int pos, int i);
};

int HashTable::Hash(const std::string &s)
{
   int chave = 0;
   for (int i = 0; i < (int)(s.size()); i++)
   {
      chave += ((int)(s[i])) * (i + 1);
   }
   return chave % tamanhoTabela;
}

int HashTable::Probing(int pos, int i)
{
   // Probing quadrático para reduzir colisões e melhorar performance
   return (pos + i * i) % tamanhoTabela;
}

HashTable::HashTable(int tamanho = 100)
{
   tabela = new ElementoTabela[tamanho];
   tamanhoTabela = tamanho;
}

HashTable::~HashTable()
{
   delete[] tabela; // Desaloca a tabela inteira
}

void HashTable::Inserir(const RechargeStation &s)
{
   int pos = Hash(s.id_end);
   int i = 0;

   while (i < tamanhoTabela)
   {
      int newPos = Probing(pos, i);
      if (tabela[newPos].estado == VAZIO || tabela[newPos].estado == REMOVIDO)
      {
         tabela[newPos].endereco = new RechargeStation(s);
         tabela[newPos].estado = OCUPADO;
         return;
      }
      i++;
   }
   // Tabela cheia, mas sem rehashing, nada pode ser feito
}

bool HashTable::Pesquisa(const std::string &s, RechargeStation &result)
{
   int pos = Hash(s);
   int i = 0;

   while (i < tamanhoTabela)
   {
      int newPos = Probing(pos, i);
      if (tabela[newPos].estado == VAZIO) // Se encontrou uma posição vazia, o item não está na tabela
         return false;

      if (tabela[newPos].estado == OCUPADO && tabela[newPos].endereco->id_end == s)
      {
         result = *(tabela[newPos].endereco);
         return true;
      }
      i++;
   }
   return false;
}

#endif
