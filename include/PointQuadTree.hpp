#include <iostream>

#define INF 0x3f3f3f3f

#include "MaxHeap.hpp"
#include "Distpair.hpp"
#include "Point.hpp"
#include "smv.h"

using namespace std; // Para função max e sqr

struct QuadNode
{
   Point point;                   // O ponto armazenado neste nó
   bool isActive = true;          // Estado do nó
   std::string id;                // id do endereço
   int NE, NW, SE, SW;            // Índices dos filhos no array de nós
   double maxX, minX, maxY, minY; // limites do nó

   QuadNode(const Point &pt = Point(), const std::string &id = "-1", double maxX = 0, double minX = 0, double maxY = 0, double minY = 0)
       : point(pt), isActive(true), id(id), NE(-1), NW(-1), SE(-1), SW(-1), maxX(maxX), minX(minX), maxY(maxY), minY(minY) {}

   QuadNode &operator=(const QuadNode &other)
   {
      if (this != &other)
      {
         point = other.point;
         isActive = other.isActive;
         id = other.id;
         NE = other.NE;
         NW = other.NW;
         SE = other.SE;
         SW = other.SW;
         maxX = other.maxX;
         minX = other.minX;
         maxY = other.maxY;
         minY = other.minY;
      }
      return *this;
   }
};

class QuadTree
{
private:
   QuadNode *nodes; // Array de nós
   int capacity;    // Capacidade máxima do array
   int n_nodes;     // Tamanho atual do array

   void insert(int nodeIdx, const Point &point, std::string id);
   int find(const Point &p, std::string id);
   void knn_query(int nodeIdx, const Point &target, maxheap<Distpair> &maxHeap, int k);
   bool shouldExplore(int childIdx, const Point &target, double maxDist);

public:
   QuadTree(int capacity);
   ~QuadTree();

   bool activate(const Point &p, std::string id);
   bool deactivate(const Point &p, std::string id);
   void insert(const Point &point, const std::string &id);
   Distpair *knn(Point &p, int k);
};

QuadTree::QuadTree(int capacity = 50) : capacity(capacity), n_nodes(0)
{

   int bytes;
   nodes = (QuadNode *)init_page(&bytes);

   if ((int)(capacity * sizeof(QuadNode)) > bytes)
   {
      fprintf(stderr, "node_initialize: smv did not provide enough memory\n");
      exit(1);
   }
}

QuadTree::~QuadTree()
{
   end_page();
}

int QuadTree::find(const Point &p, std::string id)
{
   int current = 0;

   while (current != -1)
   {
      if (nodes[current].point.x == p.x && nodes[current].point.y == p.y && nodes[current].id == id)
      {
         return current; // Encontrou o ponto
      }

      if (nodes[current].point.x > p.x && nodes[current].point.y < p.y) // NW
      {
         current = nodes[current].NW;
      }
      else if (nodes[current].point.x <= p.x && nodes[current].point.y < p.y) // NE
      {
         current = nodes[current].NE;
      }
      else if (nodes[current].point.x > p.x && nodes[current].point.y >= p.y) // SW
      {
         current = nodes[current].SW;
      }
      else // SE
      {
         current = nodes[current].SE;
      }
   }

   return -1; // Ponto não encontrado
}

void QuadTree::insert(int nodeIdx, const Point &point, std::string id) // ENCONTRA POSIÇÂO CORRETA EM SEM NÒ (-1), ATUALIZA LIMITES E ADICIONA
{
   if (nodes[nodeIdx].point.x > point.x && nodes[nodeIdx].point.y < point.y) // NW
   {
      if (nodes[nodeIdx].NW == -1)
      {
         nodes[n_nodes] = QuadNode(point, id, nodes[nodeIdx].point.x, nodes[nodeIdx].minX, nodes[nodeIdx].maxY, nodes[nodeIdx].point.y);
         nodes[nodeIdx].NW = n_nodes++;
      }
      else
      {
         insert(nodes[nodeIdx].NW, point, id);
      }
   }
   else if (nodes[nodeIdx].point.x <= point.x && nodes[nodeIdx].point.y < point.y) // NE
   {
      if (nodes[nodeIdx].NE == -1)
      {
         nodes[n_nodes] = QuadNode(point, id, nodes[nodeIdx].maxX, nodes[nodeIdx].point.x, nodes[nodeIdx].maxY, nodes[nodeIdx].point.y);
         nodes[nodeIdx].NE = n_nodes++;
      }
      else
      {
         insert(nodes[nodeIdx].NE, point, id);
      }
   }
   else if (nodes[nodeIdx].point.x > point.x && nodes[nodeIdx].point.y >= point.y) // SW
   {
      if (nodes[nodeIdx].SW == -1)
      {
         nodes[n_nodes] = QuadNode(point, id, nodes[nodeIdx].point.x, nodes[nodeIdx].minX, nodes[nodeIdx].point.y, nodes[nodeIdx].minY);
         nodes[nodeIdx].SW = n_nodes++;
      }
      else
      {
         insert(nodes[nodeIdx].SW, point, id);
      }
   }
   else // SE
   {
      if (nodes[nodeIdx].SE == -1)
      {
         nodes[n_nodes] = QuadNode(point, id, nodes[nodeIdx].maxX, nodes[nodeIdx].point.x, nodes[nodeIdx].point.y, nodes[nodeIdx].minY);
         nodes[nodeIdx].SE = n_nodes++;
      }
      else
      {
         insert(nodes[nodeIdx].SE, point, id);
      }
   }
}

void QuadTree::insert(const Point &point, const std::string &id)
{

   if (n_nodes == 0)
   {
      nodes[0] = QuadNode(point, id, INF, -INF, INF, -INF); // PRIMEIRO PONTO TEM REGIAO INFINITA
      ++n_nodes;
   }
   else
   {
      insert(0, point, id);
   }
}

bool QuadTree::activate(const Point &p, std::string id) // ATIVA PONTO
{
   int pos = find(p, id);

   if (pos == -1 || nodes[pos].isActive)
   {
      return false;
   }
   else
   {
      nodes[pos].isActive = true;
      return true;
   }
}

bool QuadTree::deactivate(const Point &p, std::string id) // DESATIVA PONTO
{
   int pos = find(p, id);

   if (pos == -1 || !nodes[pos].isActive)
   {
      return false;
   }
   else
   {
      nodes[pos].isActive = false;
      return true;
   }
}

double calculateDistance(const Point &a, const Point &b)
{
   return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

Distpair *QuadTree::knn(Point &p, int k)
{
   // max heap para armazenar os k vizinhos mais próximos
   maxheap<Distpair> maxHeap(k);

   // busca k-NN a partir da raiz da QuadTree
   knn_query(0, p, maxHeap, k);

   // array para armazenar os k vizinhos
   Distpair *knnResults = new Distpair[k];

   for (int i = k - 1; i >= 0; i--)
   {
      knnResults[i] = maxHeap.top();
      maxHeap.dequeue();
   }

   return knnResults;
}

// calcular a distância mínima de um ponto aos limites de um quadrante
double minDistanceToQuadrant(double minX, double maxX, double minY, double maxY, const Point &target)
{
   double dx = std::max(0.0, std::max(minX - target.x, target.x - maxX));
   double dy = std::max(0.0, std::max(minY - target.y, target.y - maxY));
   return std::sqrt(dx * dx + dy * dy);
}

// verificar se um quadrante deve ser explorado
bool QuadTree::shouldExplore(int childIdx, const Point &target, double maxDist)
{
   if (childIdx == -1)
      return false;

   double minDist = minDistanceToQuadrant(
       nodes[childIdx].minX, nodes[childIdx].maxX,
       nodes[childIdx].minY, nodes[childIdx].maxY,
       target);

   // se a distância mínima ao quadrante é menor que a maior distância no heap
   return minDist < maxDist;
}

void QuadTree::knn_query(int nodeIdx, const Point &target, maxheap<Distpair> &maxHeap, int k)
{
   if (nodeIdx == -1)
      return;

   double dist = calculateDistance(nodes[nodeIdx].point, target);

   if (nodes[nodeIdx].isActive)
   {
      if (maxHeap.getSize() < k)
      {
         maxHeap.enqueue({nodes[nodeIdx].id, dist});
      }
      else if (dist < maxHeap.top().dist)
      {
         maxHeap.dequeue();
         maxHeap.enqueue({nodes[nodeIdx].id, dist});
      }
   }

   int primaryChild = -1, secondaryChild1 = -1, secondaryChild2 = -1, secondaryChild3 = -1;

   if (target.x < nodes[nodeIdx].point.x && target.y > nodes[nodeIdx].point.y)
   { // NW
      primaryChild = nodes[nodeIdx].NW;
      secondaryChild1 = nodes[nodeIdx].NE;
      secondaryChild2 = nodes[nodeIdx].SW;
      secondaryChild3 = nodes[nodeIdx].SE;
   }
   else if (target.x >= nodes[nodeIdx].point.x && target.y > nodes[nodeIdx].point.y)
   { // NE
      primaryChild = nodes[nodeIdx].NE;
      secondaryChild1 = nodes[nodeIdx].NW;
      secondaryChild2 = nodes[nodeIdx].SE;
      secondaryChild3 = nodes[nodeIdx].SW;
   }
   else if (target.x < nodes[nodeIdx].point.x && target.y <= nodes[nodeIdx].point.y)
   { // SW
      primaryChild = nodes[nodeIdx].SW;
      secondaryChild1 = nodes[nodeIdx].SE;
      secondaryChild2 = nodes[nodeIdx].NW;
      secondaryChild3 = nodes[nodeIdx].NE;
   }
   else
   { // SE
      primaryChild = nodes[nodeIdx].SE;
      secondaryChild1 = nodes[nodeIdx].SW;
      secondaryChild2 = nodes[nodeIdx].NE;
      secondaryChild3 = nodes[nodeIdx].NW;
   }

   knn_query(primaryChild, target, maxHeap, k);

   double maxDist = maxHeap.getSize() == k ? maxHeap.top().dist : INF;

   if (shouldExplore(secondaryChild1, target, maxDist))
   {
      knn_query(secondaryChild1, target, maxHeap, k);
   }
   if (shouldExplore(secondaryChild2, target, maxDist))
   {
      knn_query(secondaryChild2, target, maxHeap, k);
   }
   if (shouldExplore(secondaryChild3, target, maxDist))
   {
      knn_query(secondaryChild3, target, maxHeap, k);
   }
}
