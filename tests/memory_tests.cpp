// Instrumentacao exclusiva deste executavel de teste; nao altera a classe AVL.
#include "AVLTree.h"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <streambuf>

namespace {
std::ptrdiff_t liveAllocations = 0;

class DiscardBuffer : public std::streambuf {
    int_type overflow(int_type character) override {
        return traits_type::not_eof(character);
    }
    std::streamsize xsputn(const char*, std::streamsize count) override {
        return count;
    }
};
}

void* operator new(std::size_t size) {
    void* memory = std::malloc(size == 0 ? 1 : size);
    if (!memory) { throw std::bad_alloc(); }
    ++liveAllocations;
    return memory;
}

void operator delete(void* memory) noexcept {
    if (memory) {
        --liveAllocations;
        std::free(memory);
    }
}

void operator delete(void* memory, std::size_t) noexcept {
    ::operator delete(memory);
}

int main() {
    DiscardBuffer buffer;
    std::ostream sink(&buffer);
    sink << "Preparar formatacao " << 42 << '\n';
    const auto baseline = liveAllocations;
    for (int cycle = 0; cycle < 100; ++cycle) {
        {
            AVLTree tree(sink);
            for (int value = 0; value < 1000; ++value) {
                tree.insert(value);
                tree.insert(value); // Duplicatas nao podem alocar novos nos.
            }
            if (liveAllocations != baseline + 1000) { return 1; }
            for (int value = 0; value < 1000; value += 2) {
                tree.remove(value);
                tree.remove(value); // Inexistentes nao podem liberar outro no.
            }
            if (liveAllocations != baseline + 500) { return 2; }
            if (cycle % 2 == 0) {
                tree.clear();
                tree.clear();
                if (liveAllocations != baseline) { return 3; }
                tree.insert(-1);
            }
            // Ciclos impares encerram com 500 nos: testar o destrutor.
        }
        if (liveAllocations != baseline) { return 4; }
    }
    std::cout << "[OK] 100 ciclos: alocacao, duplicatas, remocao, clear e destrutor.\n"
              << "Nenhuma alocacao pendente nos cenarios verificados.\n";
    return 0;
}
