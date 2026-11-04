#include "AVLTree.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>

AVLTree::AVLTree() : AVLTree(std::cout) {}

AVLTree::AVLTree(std::ostream& rotationOutput) : rotationOutput_(rotationOutput) {}

AVLTree::~AVLTree() {
    clear();
}

int AVLTree::nodeHeight(const Node* node) {
    return node ? node->height : 0;
}

void AVLTree::updateHeight(Node* node) {
    node->height = 1 + std::max(nodeHeight(node->left), nodeHeight(node->right));
}

int AVLTree::balanceFactor(const Node* node) {
    return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0;
}

Node* AVLTree::rotateRight(Node* node) {
    Node* newRoot = node->left;
    Node* transferred = newRoot->right;
    newRoot->right = node;
    node->left = transferred;
    // Atualizar primeiro a antiga raiz: a nova altura depende dela.
    updateHeight(node);
    updateHeight(newRoot);
    return newRoot;
}

Node* AVLTree::rotateLeft(Node* node) {
    Node* newRoot = node->right;
    Node* transferred = newRoot->left;
    newRoot->left = node;
    node->right = transferred;
    updateHeight(node);
    updateHeight(newRoot);
    return newRoot;
}

Node* AVLTree::rebalance(Node* node) {
    if (!node) {
        return nullptr;
    }
    updateHeight(node);
    const int factor = balanceFactor(node);
    if (factor > 1) {
        // Filho com FB zero tambem usa LL (caso importante na remocao).
        const bool doubleRotation = balanceFactor(node->left) < 0;
        rotationOutput_ << "[AVL] Rotacao " << (doubleRotation ? "LR" : "LL")
                        << " no no " << node->value << " (FB=" << factor << ").\n";
        if (doubleRotation) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }
    if (factor < -1) {
        const bool doubleRotation = balanceFactor(node->right) > 0;
        rotationOutput_ << "[AVL] Rotacao " << (doubleRotation ? "RL" : "RR")
                        << " no no " << node->value << " (FB=" << factor << ").\n";
        if (doubleRotation) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }
    return node;
}

Node* AVLTree::insertNode(Node* node, int value, bool& inserted) {
    if (!node) {
        Node* created = new Node(value);
        inserted = true;
        return created;
    }
    if (value < node->value) {
        node->left = insertNode(node->left, value, inserted);
    } else if (value > node->value) {
        node->right = insertNode(node->right, value, inserted);
    } else {
        return node;
    }
    return rebalance(node);
}

bool AVLTree::insert(int value) {
    bool inserted = false;
    root_ = insertNode(root_, value, inserted);
    if (inserted) {
        ++size_;
    }
    return inserted;
}

Node* AVLTree::minimum(Node* node) {
    while (node->left) {
        node = node->left;
    }
    return node;
}

Node* AVLTree::removeNode(Node* node, int value, bool& removed) {
    if (!node) {
        return nullptr;
    }
    if (value < node->value) {
        node->left = removeNode(node->left, value, removed);
    } else if (value > node->value) {
        node->right = removeNode(node->right, value, removed);
    } else {
        removed = true;
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        // Dois filhos: copiar o sucessor e remove-lo da subarvore direita.
        Node* successor = minimum(node->right);
        node->value = successor->value;
        node->right = removeNode(node->right, successor->value, removed);
    }
    // Rebalancear em TODOS os ancestrais, inclusive depois de uma rotacao.
    return rebalance(node);
}

bool AVLTree::remove(int value) {
    bool removed = false;
    root_ = removeNode(root_, value, removed);
    if (removed) {
        --size_;
    }
    return removed;
}

bool AVLTree::contains(int value) const {
    const Node* node = root_;
    while (node) {
        if (value == node->value) {
            return true;
        }
        node = value < node->value ? node->left : node->right;
    }
    return false;
}

void AVLTree::destroy(Node* node) {
    if (node) {
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
}

void AVLTree::clear() {
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
}

int AVLTree::height() const { return nodeHeight(root_); }
std::size_t AVLTree::size() const { return size_; }
const Node* AVLTree::root() const { return root_; }

void AVLTree::collectInOrder(const Node* node, std::vector<int>& values) {
    if (node) {
        collectInOrder(node->left, values);
        values.push_back(node->value);
        collectInOrder(node->right, values);
    }
}

void AVLTree::collectPreOrder(const Node* node, std::vector<int>& values) {
    if (node) {
        values.push_back(node->value);
        collectPreOrder(node->left, values);
        collectPreOrder(node->right, values);
    }
}

void AVLTree::collectPostOrder(const Node* node, std::vector<int>& values) {
    if (node) {
        collectPostOrder(node->left, values);
        collectPostOrder(node->right, values);
        values.push_back(node->value);
    }
}

std::vector<int> AVLTree::inOrder() const {
    std::vector<int> values;
    values.reserve(size_);
    collectInOrder(root_, values);
    return values;
}

std::vector<int> AVLTree::preOrder() const {
    std::vector<int> values;
    values.reserve(size_);
    collectPreOrder(root_, values);
    return values;
}

std::vector<int> AVLTree::postOrder() const {
    std::vector<int> values;
    values.reserve(size_);
    collectPostOrder(root_, values);
    return values;
}

void AVLTree::printChildren(const Node* node, std::ostream& output,
                            const std::string& prefix) {
    if (node->left) {
        printBranch(node->left, output, prefix, !node->right, 'E');
    }
    if (node->right) {
        printBranch(node->right, output, prefix, true, 'D');
    }
}

void AVLTree::printBranch(const Node* node, std::ostream& output,
                          const std::string& prefix, bool last, char side) {
    output << prefix << (last ? "`-- " : "+-- ") << side << ": " << node->value
           << " [h=" << node->height << ", fb=" << balanceFactor(node) << "]\n";
    printChildren(node, output, prefix + (last ? "    " : "|   "));
}

void AVLTree::printTree(std::ostream& output) const {
    if (!root_) {
        output << "Arvore vazia.\n";
        return;
    }
    output << root_->value << " [h=" << root_->height
           << ", fb=" << balanceFactor(root_) << "]\n";
    printChildren(root_, output, "");
    output << "E: filho esquerdo | D: filho direito\n";
}

void AVLTree::printInfo(const Node* node, std::ostream& output) {
    if (node) {
        output << std::setw(12) << node->value << " | " << std::setw(6) << node->height
               << " | " << std::setw(4) << balanceFactor(node) << '\n';
        printInfo(node->left, output);
        printInfo(node->right, output);
    }
}

void AVLTree::printNodeInfo(std::ostream& output) const {
    if (!root_) {
        output << "Arvore vazia.\n";
        return;
    }
    output << "       Valor | Altura |   FB\n"
           << "-------------+--------+-----\n";
    printInfo(root_, output);
    output << "FB = altura(esquerda) - altura(direita).\n";
}

bool AVLTree::validateNode(const Node* node, long long lower, long long upper,
                           int& actualHeight, std::size_t& count) {
    if (!node) {
        actualHeight = 0;
        return true;
    }
    if (node->value <= lower || node->value >= upper) {
        return false;
    }
    int leftHeight = 0;
    int rightHeight = 0;
    if (!validateNode(node->left, lower, node->value, leftHeight, count) ||
        !validateNode(node->right, node->value, upper, rightHeight, count)) {
        return false;
    }
    ++count;
    actualHeight = 1 + std::max(leftHeight, rightHeight);
    return node->height == actualHeight && std::abs(leftHeight - rightHeight) <= 1;
}

bool AVLTree::isValid() const {
    int actualHeight = 0;
    std::size_t count = 0;
    return validateNode(root_, std::numeric_limits<long long>::lowest(),
                        std::numeric_limits<long long>::max(), actualHeight, count)
           && count == size_;
}
