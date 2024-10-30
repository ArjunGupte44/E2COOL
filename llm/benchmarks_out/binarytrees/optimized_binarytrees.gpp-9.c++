#include <iostream>
#include <vector>
#include <stack>
#include <omp.h>

struct Node {
    Node *l, *r;
    int check() const {
        return (l ? l->check() + 1 + r->check() : 1);
    }
};

class NodePool {
    std::vector<Node*> nodes;
public:
    ~NodePool() { clear(); }
    Node* alloc() {
        if (!nodes.empty()) {
            Node* node = nodes.back();
            nodes.pop_back();
            return node;
        }
        return new Node();
    }
    void clear() {
        for (auto n : nodes) delete n;
        nodes.clear();
    }
    void release(Node* node) {
        nodes.push_back(node);
    }
};

Node* makeTree(int depth, NodePool &pool) {
    Node* node = pool.alloc();
    if (depth > 0) {
        node->l = makeTree(depth - 1, pool);
        node->r = makeTree(depth - 1, pool);
    } else {
        node->l = node->r = nullptr;
    }
    return node;
}

int main(int argc, char* argv[]) {
    int min_depth = 4;
    int max_depth = std::max(min_depth + 2, (argc == 2 ? std::atoi(argv[1]) : 10));
    int stretch_depth = max_depth + 1;

    {
        NodePool pool;
        Node* c = makeTree(stretch_depth, pool);
        std::cout << "stretch tree of depth " << stretch_depth << "\t check: " << c->check() << std::endl;
        pool.clear();
    }

    NodePool long_lived_pool;
    Node *long_lived_tree = makeTree(max_depth, long_lived_pool);

    std::vector<std::string> results((max_depth - min_depth) / 2 + 1);

#pragma omp parallel for
    for (int d = min_depth; d <= max_depth; d += 2) {
        int iterations = 1 << (max_depth - d + min_depth);
        int c = 0;
        NodePool store;
        for (int i = 0; i < iterations; ++i) {
            Node* a = makeTree(d, store);
            c += a->check();
            store.clear();
        }
        results[(d - min_depth) / 2] = std::to_string(iterations) + "\t trees of depth " + std::to_string(d) + "\t check: " + std::to_string(c) + "\n";
    }

    for (const auto &result : results) {
        std::cout << result;
    }

    std::cout << "long lived tree of depth " << max_depth << "\t check: " << long_lived_tree->check() << "\n";
    long_lived_pool.clear();

    return 0;
}