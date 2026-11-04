#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

struct Node {
    int value;
    Node* left = nullptr;
    Node* right = nullptr;
    int height = 1;

    explicit Node(int initialValue) : value(initialValue) {}
};

// AVL de inteiros sem duplicatas. Altura: vazia = 0; folha = 1.
class AVLTree {
public:
    AVLTree();
    explicit AVLTree(std::ostream& rotationOutput);
    ~AVLTree();

    // Cada arvore e dona exclusiva dos seus nos.
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(int value);
    bool remove(int value);
    bool contains(int value) const;
    void clear();

    int height() const;
    std::size_t size() const;
    const Node* root() const;
    static int balanceFactor(const Node* node);

    std::vector<int> inOrder() const;
    std::vector<int> preOrder() const;
    std::vector<int> postOrder() const;
    void printTree(std::ostream& output) const;
    void printNodeInfo(std::ostream& output) const;

    // Verifica BST, alturas reais, balanceamento e quantidade de nos.
    bool isValid() const;

private:
    Node* root_ = nullptr;
    std::size_t size_ = 0;
    std::ostream& rotationOutput_;

    static int nodeHeight(const Node* node);
    static void updateHeight(Node* node);
    static Node* rotateLeft(Node* node);
    static Node* rotateRight(Node* node);
    Node* rebalance(Node* node);
    Node* insertNode(Node* node, int value, bool& inserted);
    Node* removeNode(Node* node, int value, bool& removed);
    static Node* minimum(Node* node);
    static void destroy(Node* node);
    static void collectInOrder(const Node* node, std::vector<int>& values);
    static void collectPreOrder(const Node* node, std::vector<int>& values);
    static void collectPostOrder(const Node* node, std::vector<int>& values);
    static void printBranch(const Node* node, std::ostream& output,
                            const std::string& prefix, bool last, char side);
    static void printChildren(const Node* node, std::ostream& output,
                              const std::string& prefix);
    static void printInfo(const Node* node, std::ostream& output);
    static bool validateNode(const Node* node, long long lower, long long upper,
                             int& actualHeight, std::size_t& count);
};

#endif
