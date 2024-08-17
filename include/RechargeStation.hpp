#ifndef RECHARGE
#define RECHARGE

#include <bits/stdc++.h>
using namespace std;

struct RechargeStation
{
   string id_end;      // id do endereço
   long id_lograd;     // id do logradouro
   string sigla_tipo;  // tipo logradouro (Avenida, Rua)
   string nome_lograd; // nome do logradouro
   int num_imovel;     // numero do imovel
   string nome_bairro; // nome do bairro
   string nome_regiao; // nome da regiao
   int cep;            // numero do cep
   double x;           // coordenada x
   double y;           // coordenada y
};

#endif