#include "AVLTree.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Auditoria independente: calcula alturas reais, limites BST e quantidade.
int audit(const Node* node, std::optional<int> lower, std::optional<int> upper,
          std::size_t& count) {
    if (!node) { return 0; }
    require(!lower || node->value > *lower, "Limite inferior BST violado");
    require(!upper || node->value < *upper, "Limite superior BST violado");
    const int left = audit(node->left, lower, node->value, count);
    const int right = audit(node->right, node->value, upper, count);
    ++count;
    const int actualHeight = 1 + std::max(left, right);
    require(node->height == actualHeight, "Altura armazenada incorreta");
    require(std::abs(left - right) <= 1, "No desbalanceado");
    require(AVLTree::balanceFactor(node) == left - right, "FB incorreto");
    return actualHeight;
}

void check(const AVLTree& tree, const std::vector<int>& expected) {
    require(tree.isValid(), "Validador interno detectou arvore invalida");
    std::size_t count = 0;
    require(audit(tree.root(), std::nullopt, std::nullopt, count) == tree.height(),
            "Altura da arvore incorreta");
    require(count == tree.size() && count == expected.size(), "Quantidade incorreta");
    const auto ordered = tree.inOrder();
    require(ordered == expected, "Percurso em ordem difere da referencia ordenada");
    require(std::is_sorted(ordered.begin(), ordered.end()), "Percurso nao ordenado");
}

void fill(AVLTree& tree, const std::vector<int>& values) {
    std::vector<int> reference;
    for (int value : values) {
        require(tree.insert(value), "Insercao deveria criar no");
        reference.insert(std::lower_bound(reference.begin(), reference.end(), value), value);
        check(tree, reference);
    }
}

void testEmptyAndSingleton() {
    std::ostringstream log;
    AVLTree tree(log);
    check(tree, {});
    require(tree.root() == nullptr && tree.height() == 0, "Estado vazio incorreto");
    require(!tree.contains(5) && !tree.remove(5), "Operacao em arvore vazia incorreta");
    require(tree.preOrder().empty() && tree.postOrder().empty(), "Percursos vazios");
    std::ostringstream output;
    tree.printTree(output);
    tree.printNodeInfo(output);
    require(output.str().find("Arvore vazia") != std::string::npos, "Aviso de vazia");
    require(tree.insert(5), "Insercao unica");
    check(tree, {5});
    require(tree.height() == 1 && tree.contains(5) && !tree.contains(-5), "Busca/altura");
    require(!tree.insert(5), "Duplicata aceita");
    check(tree, {5});
    require(!tree.remove(6), "Remocao inexistente aceita");
    check(tree, {5});
    require(tree.remove(5), "Remocao da unica raiz");
    check(tree, {});
    tree.clear();
    tree.clear();
    require(log.str().empty(), "Rotacao indevida");
}

void testInsertionRotation(const std::vector<int>& values, const std::string& type) {
    std::ostringstream log;
    AVLTree tree(log);
    fill(tree, values);
    check(tree, {10, 20, 30});
    require(tree.root()->value == 20 && tree.root()->left->value == 10 &&
            tree.root()->right->value == 30, "Estrutura da rotacao " + type);
    require(tree.height() == 2, "Altura apos rotacao " + type);
    require(log.str().find("Rotacao " + type + " no no") != std::string::npos,
            "Mensagem ausente: " + type);
    const auto messages = log.str();
    require(std::count(messages.begin(), messages.end(), '\n') == 1,
            "Esperada uma mensagem por caso de rotacao");
}

void testDeletionRotation(const std::vector<int>& values, int removed,
                          const std::string& type) {
    std::ostringstream log;
    AVLTree tree(log);
    fill(tree, values);
    log.str("");
    require(tree.remove(removed), "Remocao para rotacao " + type);
    auto expected = values;
    expected.erase(std::find(expected.begin(), expected.end(), removed));
    std::sort(expected.begin(), expected.end());
    check(tree, expected);
    require(log.str().find("Rotacao " + type + " no no") != std::string::npos,
            "Rotacao de remocao ausente: " + type);
}

void testRemovalCases() {
    std::ostringstream log;
    {
        AVLTree tree(log);
        fill(tree, {20, 10, 30, 25});
        require(tree.remove(10), "Remocao de folha");
        check(tree, {20, 25, 30});
    }
    {
        AVLTree tree(log);
        fill(tree, {20, 10, 30, 25});
        require(tree.remove(30), "Remocao com um filho esquerdo");
        check(tree, {10, 20, 25});
    }
    {
        AVLTree tree(log);
        fill(tree, {20, 10, 30, 35});
        require(tree.remove(30), "Remocao com um filho direito");
        check(tree, {10, 20, 35});
    }
    {
        AVLTree tree(log);
        fill(tree, {40, 20, 60, 10, 30, 50, 70, 25, 35});
        require(tree.remove(20), "Remocao interna com dois filhos");
        check(tree, {10, 25, 30, 35, 40, 50, 60, 70});
        require(tree.remove(40), "Remocao da raiz com dois filhos");
        check(tree, {10, 25, 30, 35, 50, 60, 70});
    }
    {
        AVLTree tree(log);
        fill(tree, {40, 20, 60, 10, 30, 50, 70, 55});
        const int oldRoot = tree.root()->value;
        auto expected = tree.inOrder();
        expected.erase(std::find(expected.begin(), expected.end(), oldRoot));
        require(tree.remove(oldRoot), "Sucessor com filho direito");
        check(tree, expected);
    }
    {
        AVLTree tree(log);
        fill(tree, {2, 1});
        require(tree.remove(2), "Remocao da raiz com um filho");
        check(tree, {1});
    }
}

void testCascade() {
    std::ostringstream log;
    AVLTree tree(log);
    fill(tree, {7, 14, 4, 12, 1, 6, 15, 10, 9, 11, 13, 16, 3, 8, 5, 2});
    log.str("");
    require(tree.remove(12), "Remocao em cascata");
    check(tree, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16});
    const auto messages = log.str();
    require(std::count(messages.begin(), messages.end(), '\n') == 2,
            "Esperados dois rebalanceamentos em niveis distintos");
    require(messages.find("Rotacao RR no no 14") != std::string::npos &&
            messages.find("Rotacao LL no no 13") != std::string::npos,
            "Rebalanceamentos em cascata incorretos");
}

void testTraversalAndDisplay() {
    std::ostringstream log;
    AVLTree tree(log);
    fill(tree, {30, 20, 40, 10, 25, 50});
    check(tree, {10, 20, 25, 30, 40, 50});
    require(tree.preOrder() == std::vector<int>({30, 20, 10, 25, 40, 50}), "Pre-ordem");
    require(tree.postOrder() == std::vector<int>({10, 25, 20, 50, 40, 30}), "Pos-ordem");
    std::ostringstream output;
    tree.printTree(output);
    require(output.str().find("30 [h=3, fb=0]") != std::string::npos, "Raiz na exibicao");
    require(output.str().find("D: 50 [h=1, fb=0]") != std::string::npos, "Filho direito");
    output.str("");
    tree.printNodeInfo(output);
    require(output.str().find("Altura") != std::string::npos, "Tabela dos nos");
    for (int value : tree.inOrder()) {
        require(tree.contains(value), "Busca existente");
    }
    require(!tree.contains(26), "Busca inexistente");
    tree.clear();
    check(tree, {});
    fill(tree, {-1, 0, 1});
}

void testIntegerLimits() {
    std::ostringstream log;
    AVLTree tree(log);
    const int minimum = std::numeric_limits<int>::lowest();
    const int maximum = std::numeric_limits<int>::max();
    fill(tree, {0, minimum, maximum, -1, 1});
    check(tree, {minimum, -1, 0, 1, maximum});
    require(tree.contains(minimum) && tree.contains(maximum), "Busca dos extremos");
    require(tree.remove(minimum) && tree.remove(maximum), "Remocao dos extremos");
    check(tree, {-1, 0, 1});
}

void testLongSequences() {
    std::ostringstream log;
    AVLTree tree(log);
    std::vector<int> reference;
    constexpr int count = 3000;
    for (int value = 0; value < count; ++value) {
        require(tree.insert(value), "Insercao crescente");
        reference.push_back(value);
        check(tree, reference);
    }
    for (int value = 0; value < count; ++value) {
        require(tree.remove(value), "Remocao crescente");
        reference.erase(reference.begin());
        check(tree, reference);
    }
    for (int value = count - 1; value >= 0; --value) {
        require(tree.insert(value), "Insercao decrescente");
        reference.insert(reference.begin(), value);
        check(tree, reference);
    }
    // Remover repetidamente a raiz exercita sucessores e troca da raiz.
    while (tree.root()) {
        const int value = tree.root()->value;
        require(tree.remove(value), "Remocao repetida da raiz");
        reference.erase(std::lower_bound(reference.begin(), reference.end(), value));
        check(tree, reference);
    }
}

void testRandomOperations() {
    for (unsigned int seed : {42u, 2026u, 98765u}) {
        std::mt19937 generator(seed);
        std::uniform_int_distribution<int> values(-1000, 1000);
        std::uniform_int_distribution<int> operations(0, 2);
        std::ostringstream log;
        AVLTree tree(log);
        std::vector<int> reference;
        for (int step = 0; step < 10000; ++step) {
            const int value = values(generator);
            const auto position = std::lower_bound(reference.begin(), reference.end(), value);
            const bool exists = position != reference.end() && *position == value;
            switch (operations(generator)) {
                case 0:
                    require(tree.insert(value) == !exists, "Retorno de insercao aleatoria");
                    if (!exists) { reference.insert(position, value); }
                    break;
                case 1:
                    require(tree.remove(value) == exists, "Retorno de remocao aleatoria");
                    if (exists) { reference.erase(position); }
                    break;
                default:
                    require(tree.contains(value) == exists, "Busca aleatoria");
                    break;
            }
            check(tree, reference);
            log.str("");
        }
    }
}

} // namespace

int main() {
    int passed = 0;
    const auto run = [&passed](const std::string& name, const std::function<void()>& test) {
        test();
        ++passed;
        std::cout << "[OK] " << name << '\n';
    };
    try {
        run("Vazia, no unico, duplicatas e inexistentes", testEmptyAndSingleton);
        run("Insercao LL", [] { testInsertionRotation({30, 20, 10}, "LL"); });
        run("Insercao RR", [] { testInsertionRotation({10, 20, 30}, "RR"); });
        run("Insercao LR", [] { testInsertionRotation({30, 10, 20}, "LR"); });
        run("Insercao RL", [] { testInsertionRotation({10, 30, 20}, "RL"); });
        run("Remocao LL com filho FB zero", [] {
            testDeletionRotation({30, 20, 40, 10, 25}, 40, "LL");
        });
        run("Remocao RR com filho FB zero", [] {
            testDeletionRotation({20, 10, 30, 25, 40}, 10, "RR");
        });
        run("Remocao LR", [] { testDeletionRotation({30, 20, 40, 25}, 40, "LR"); });
        run("Remocao RL", [] { testDeletionRotation({30, 20, 40, 35}, 20, "RL"); });
        run("Folha, um filho, dois filhos e raiz", testRemovalCases);
        run("Remocao com dois rebalanceamentos", testCascade);
        run("Percursos, busca, exibicao e limpeza", testTraversalAndDisplay);
        run("Limites de int e numeros negativos", testIntegerLimits);
        run("12000 insercoes/remocoes em sequencias longas", testLongSequences);
        run("30000 operacoes aleatorias, tres sementes", testRandomOperations);
    } catch (const std::exception& error) {
        std::cerr << "[FALHA] " << error.what() << '\n';
        return 1;
    }
    std::cout << passed << " grupos de testes aprovados.\n";
    return 0;
}
