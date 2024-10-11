#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <apr_pools.h>
#include <omp.h>

const size_t LINE_SIZE = 64;

class Apr
{
public:
    Apr() 
    {
        apr_initialize();
    }

    ~Apr() 
    {
        apr_terminate();
    }
};

struct Node 
{
    Node *l, *r;
    int check() const 
    {
        int count = 1;
        if (l) count += l->check();
        if (r) count += r->check();
        return count;
    }
};

class NodePool
{
public:
    NodePool() 
    {
        apr_pool_create_unmanaged(&pool);
    }

    ~NodePool() 
    {
        apr_pool_destroy(pool);
    }

    Node* alloc()
    {
        return (Node *)apr_palloc(pool, sizeof(Node));
    }

    void clear()
    {
        apr_pool_clear(pool);
    }

private:
    apr_pool_t* pool;
};

Node *make_iter(int d, NodePool &store)
{
    std::stack<Node*> s;
    Node* root = store.alloc();
    s.push(root);

    for (int i = 0; i < ((1 << (d+1)) - 1); ++i) {
        Node* current = s.top();
        s.pop();

        if (i < (1 << d) - 1) {
            current->l = store.alloc();
            current->r = store.alloc();
            s.push(current->r);
            s.push(current->l);
        } else {
            current->l = nullptr;
            current->r = nullptr;
        }
    }

    return root;
}

Node *make(int d, NodePool &store) {
    // Using iterative make function to avoid deep recursion issues
    return make_iter(d, store);
}

int main(int argc, char *argv[]) 
{
    Apr apr;
    int min_depth = 4;
    int max_depth = std::max(min_depth+2, (argc == 2 ? atoi(argv[1]) : 10));
    int stretch_depth = max_depth+1;

    // Alloc then dealloc stretchdepth tree
    {
        NodePool store;
        Node *c = make(stretch_depth, store);
        std::cout << "stretch tree of depth " << stretch_depth << "\t "
                  << "check: " << c->check() << std::endl;
    }

    NodePool long_lived_store;
    Node *long_lived_tree = make(max_depth, long_lived_store);

    // buffer to store output of each thread
    char *outputstr = (char*)malloc(LINE_SIZE * (max_depth +1) * sizeof(char));

    #pragma omp parallel for schedule(dynamic)
    for (int d = min_depth; d <= max_depth; d += 2) 
    {
        int iterations = 1 << (max_depth - d + min_depth);
        int c = 0;

        // Create a memory pool for this thread to use.
        NodePool store;

        for (int i = 1; i <= iterations; ++i) 
        {
            Node *a = make(d, store);
            c += a->check();
            store.clear();
        }

        // each thread write to separate location
        snprintf(outputstr + LINE_SIZE * d, LINE_SIZE, "%d\t trees of depth %d\t check: %d\n",
           iterations, d, c);
    }

    // print all results
    for (int d = min_depth; d <= max_depth; d += 2) 
        printf("%s", outputstr + (d * LINE_SIZE) );
    free(outputstr);

    std::cout << "long lived tree of depth " << max_depth << "\t "
              << "check: " << (long_lived_tree->check()) << "\n";

    return 0;
}