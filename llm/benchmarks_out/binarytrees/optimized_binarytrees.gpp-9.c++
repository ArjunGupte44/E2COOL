#include <iostream>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <algorithm>
#include <cstring>
// Removed OpenMP for debugging output differences

const size_t LINE_SIZE = 128;

struct Node {
    Node* l = nullptr;
    Node* r = nullptr;

    int check() const {
        return (l ? l->check() : 0) + (r ? r->check() : 0) + 1;
    }
};

class NodePool {
public:
    std::vector<Node*> pool;

    Node* allocate() {
        if (!pool.empty()) {
            Node* n = pool.back();
            pool.pop_back();
            n->l = nullptr;
            n->r = nullptr;
            return n;
        }
        return new Node();
    }

    void deallocate(Node* n) {
        pool.push_back(n);
    }

    ~NodePool() {
        for (Node* n : pool) {
            delete n;
        }
    }
};

Node* make(NodePool& pool, int depth) {
    Node* root = pool.allocate();
    if (depth > 0) {
        root->l = make(pool, depth - 1);
        root->r = make(pool, depth - 1);
    }
    return root;
}

void deleteTree(NodePool& pool, Node* node) {
    if (!node) return;
    deleteTree(pool, node->l);
    deleteTree(pool, node->r);
    pool.deallocate(node);
}

int main(int argc, char* argv[]) {
    int min_depth = 4;
    int max_depth = std::max(min_depth + 2, (argc == 2 ? atoi(argv[1]) : 12));
    int stretch_depth = max_depth + 1;

    NodePool pool;

    // Stretch tree
    Node* stretch_tree = make(pool, stretch_depth);
    std::cout << "stretch tree of depth " << stretch_depth << "\t check: " << stretch_tree->check() << std::endl;
    deleteTree(pool, stretch_tree);

    // Long-lived tree
    Node* long_lived_tree = make(pool, max_depth);

    std::vector<std::string> results;
    // std::mutex results_mutex;  // Mutex for threading, not used here

    for (int d = min_depth; d <= max_depth; d += 2) {
        NodePool local_pool;  // Each depth level gets its own pool to avoid cross-level issues
        int iterations = 1 << (max_depth - d + min_depth);
        int c = 0;

        for (int i = 1; i <= iterations; ++i) {
            Node* a = make(local_pool, d);
            c += a->check();
            deleteTree(local_pool, a);
        }

        char buffer[LINE_SIZE];
        snprintf(buffer, LINE_SIZE, "%d\t trees of depth %d\t check: %d\n", iterations, d, c);
        results.push_back(std::string(buffer));
    }

    for (auto& result : results) {
        std::cout << result;
    }

    std::cout << "long lived tree of depth " << max_depth << "\t check: " << long_lived_tree->check() << "\n";
    deleteTree(pool, long_lived_tree);

    return 0;
}
