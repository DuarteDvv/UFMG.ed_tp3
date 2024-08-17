#ifndef LRU
#define LRU

struct LRUNode
{
    int index;     // Índice da página em pvet
    LRUNode *prev; // Ponteiro para o nó anterior
    LRUNode *next; // Ponteiro para o próximo nó

    LRUNode(int idx) : index(idx), prev(nullptr), next(nullptr) {}
};

class LRUList
{
private:
    LRUNode *head; // Cabeça da lista LRU
    LRUNode *tail; // Cauda da lista LRU

public:
    LRUList() : head(nullptr), tail(nullptr) {}

    ~LRUList()
    {
        while (head)
        {
            LRUNode *temp = head;
            head = head->next;
            delete temp;
        }
    }

    void moveToHead(LRUNode *node) // MOVE UM NÒ DE ALGUMA POSIÇÂO PARA FRENTE POIS FOI ACESSADO
    {
        if (head == node)
            return;

        // Remove o nó da sua posição atual
        if (node->prev)
            node->prev->next = node->next;
        if (node->next)
            node->next->prev = node->prev;

        if (tail == node)
            tail = node->prev;

        // Insere o nó na cabeça da lista
        node->next = head;
        node->prev = nullptr;

        if (head)
            head->prev = node;
        head = node;

        if (tail == nullptr)
            tail = head;
    }

    LRUNode *removeTail() // RETORNA A PÀGINA LRU (ULTIMA USADA)
    {
        LRUNode *oldTail = tail;
        if (tail)
        {
            if (tail->prev)
                tail->prev->next = nullptr;
            else
                head = nullptr;

            tail = tail->prev;
        }
        return oldTail;
    }

    LRUNode *addToHead(int index) // ADICIONA UMA NOVÀ PÀGINA NO COMEÇO (ACESSADA MAIS RECENTEMENTE)
    {
        LRUNode *node = new LRUNode(index);
        node->next = head;
        node->prev = nullptr;

        if (head)
            head->prev = node;
        head = node;
        if (tail == nullptr)
            tail = head;

        return node;
    }

    bool isEmpty() const
    {
        return head == nullptr;
    }
};

#endif