#include "AVLTree.h"

#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::optional<int> readInteger(const std::string& prompt) {
    std::string line;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) {
            return std::nullopt;
        }
        std::istringstream input(line);
        long long value = 0;
        char extra = '\0';
        if ((input >> value) && !(input >> extra) &&
            value >= std::numeric_limits<int>::lowest() &&
            value <= std::numeric_limits<int>::max()) {
            return static_cast<int>(value);
        }
        std::cout << "Entrada invalida. Digite um numero inteiro.\n";
    }
}

std::optional<bool> confirmClear() {
    std::string line;
    while (true) {
        std::cout << "Tem certeza que deseja remover todos os elementos? (S/N): ";
        if (!std::getline(std::cin, line)) {
            return std::nullopt;
        }
        std::istringstream input(line);
        std::string answer;
        std::string extra;
        if ((input >> answer) && !(input >> extra)) {
            if (answer == "S" || answer == "s") { return true; }
            if (answer == "N" || answer == "n") { return false; }
        }
        std::cout << "Resposta invalida. Digite S ou N.\n";
    }
}

void printMenu(const AVLTree& tree) {
    std::cout << "\n================================================\n"
              << "            ARVORE AVL INTERATIVA\n"
              << "================================================\n"
              << "Quantidade de nos: " << tree.size()
              << "  |  Altura atual: " << tree.height() << "\nRaiz: ";
    if (tree.root()) {
        std::cout << tree.root()->value;
    } else {
        std::cout << "(vazia)";
    }
    std::cout << "\n------------------------------------------------\n"
              << "[1] Inserir valor       [2] Remover valor\n"
              << "[3] Buscar valor        [4] Visualizar arvore\n\n"
              << "[5] Em ordem            [6] Pre-ordem\n"
              << "[7] Pos-ordem           [8] Informacoes dos nos\n\n"
              << "[9] Limpar arvore       [0] Sair\n"
              << "------------------------------------------------\n";
}

void printTraversal(const std::string& title, const std::vector<int>& values) {
    std::cout << title << ": ";
    if (values.empty()) {
        std::cout << "(arvore vazia)";
    }
    for (std::size_t index = 0; index < values.size(); ++index) {
        std::cout << (index == 0 ? "" : " ") << values[index];
    }
    std::cout << '\n';
}

} // namespace

int main() {
    AVLTree tree;
    while (true) {
        printMenu(tree);
        const auto option = readInteger("Escolha uma opcao: ");
        if (!option || *option == 0) {
            break;
        }
        if (*option >= 1 && *option <= 3) {
            const auto value = readInteger("Digite o valor inteiro: ");
            if (!value) {
                break;
            }
            if (*option == 1) {
                std::cout << "Inserindo " << *value << "...\n";
                if (tree.insert(*value)) {
                    std::cout << "Valor " << *value << " inserido com sucesso.\n";
                } else {
                    std::cout << "O valor " << *value << " ja existe na arvore.\n";
                }
            } else if (*option == 2) {
                std::cout << "Removendo " << *value << "...\n";
                if (tree.remove(*value)) {
                    std::cout << "Valor " << *value << " removido com sucesso.\n";
                } else {
                    std::cout << "Valor nao encontrado na arvore.\n";
                }
            } else {
                std::cout << "Valor " << *value
                          << (tree.contains(*value) ? " encontrado.\n" : " nao encontrado.\n");
            }
            if (*option != 3) {
                std::cout << "\nArvore atual:\n";
                tree.printTree(std::cout);
            }
        } else if (*option == 4) {
            tree.printTree(std::cout);
        } else if (*option == 5) {
            printTraversal("Em ordem", tree.inOrder());
        } else if (*option == 6) {
            printTraversal("Pre-ordem", tree.preOrder());
        } else if (*option == 7) {
            printTraversal("Pos-ordem", tree.postOrder());
        } else if (*option == 8) {
            tree.printNodeInfo(std::cout);
        } else if (*option == 9) {
            const auto confirmed = confirmClear();
            if (!confirmed) {
                break;
            }
            if (*confirmed) {
                tree.clear();
                std::cout << "Arvore limpa.\n";
            } else {
                std::cout << "Operacao cancelada.\n";
            }
        } else {
            std::cout << "Opcao invalida. Escolha um numero de 0 a 9.\n";
        }
    }
    std::cout << "\nPrograma encerrado. Memoria da arvore liberada automaticamente.\n";
    return 0;
}
