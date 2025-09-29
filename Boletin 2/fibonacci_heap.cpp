#include <cmath>
#include <vector>
#include <limits>
#include <iostream>

class fibonacci_heap {
private:
    struct Node {
        int key;
        int degree;
        Node* parent;
        Node* child;
        Node* left;
        Node* right;
        explicit Node(int k)
            : key(k), degree(0),
              parent(nullptr), child(nullptr), left(this), right(this) {}
    };
    Node* minRoot = nullptr;
    size_t n = 0;



    static void listInsertRight(Node* a, Node* b) {
        b->right = a->right;
        b->left  = a;
        a->right->left = b;
        a->right = b;
    }

    static void listRemove(Node* x) {
        x->left->right = x->right;
        x->right->left = x->left;
        x->left = x->right = x; 
    }

    static void addToRootList(Node*& minRoot, Node* x) {
        if (!minRoot) {
            minRoot = x;
            x->left = x->right = x;
        } else {
            listInsertRight(minRoot, x);
            if (x->key < minRoot->key) minRoot = x;
        }
        x->parent = nullptr;
    }

    static void linkTrees(Node* y, Node* x) {
        listRemove(y);
        y->parent = x;
        if (!x->child) {
            x->child = y;
            y->left = y->right = y;
        } else {
            listInsertRight(x->child, y);
        }
        x->degree++;
    }

    void consolidate() {
        if (!minRoot) return;

        size_t maxDeg = (n > 1) ? (static_cast<size_t>(std::floor(std::log2(n))) + 3) : 3;
        std::vector<Node*> A(maxDeg, nullptr);

        
        std::vector<Node*> roots;
        {
            Node* curr = minRoot;
            if (curr) {
                do {
                    roots.push_back(curr);
                    curr = curr->right;
                } while (curr != minRoot);
            }
        }

        for (Node* w : roots) {
            Node* x = w;
            listRemove(x);
            x->left = x->right = x;

            int d = x->degree;
            while (true) {
                if (d >= static_cast<int>(A.size())) {
                    A.resize(d + 2, nullptr);
                }
                if (!A[d]) {
                    A[d] = x;
                    break;
                }
                Node* y = A[d];
                if (y == x) break;
                if (y->key < x->key) std::swap(x, y);
                linkTrees(y, x); 
                A[d] = nullptr;
                d++;
            }
        }
        minRoot = nullptr;
        for (Node* t : A) {
            if (t) {
                t->left = t->right = t;
                addToRootList(minRoot, t);
            }
        }
    }

public:
    fibonacci_heap() = default;

    explicit fibonacci_heap(const std::vector<int>& v) {
        for (int k : v) insert(k);
    }

    bool empty() const { return n == 0; }

    int top() const {
        if (!minRoot) {
            std::cerr << "There's no top: heap is empty." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return minRoot->key;
    }
    void insert(int key) {
        Node* x = new Node(key);
        addToRootList(minRoot, x);
        n++;
    }

    void extractMin() {
        if (!minRoot) return;

        Node* z = minRoot;
        if (z->child) {
            std::vector<Node*> childs;
            Node* c = z->child;
            do {
                childs.push_back(c);
                c = c->right;
            } while (c != z->child);

            for (Node* x : childs) {
                listRemove(x);
                x->parent = nullptr;
                addToRootList(minRoot, x);
            }
            z->child = nullptr;
            z->degree = 0;
        }
        if (z->right == z) {
            minRoot = nullptr;
        } else {
            Node* next = z->right;
            listRemove(z);
            minRoot = next; 
            consolidate();
        }

        delete z;
        n--;
    }
};
