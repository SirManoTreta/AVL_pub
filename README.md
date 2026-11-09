# Arvore AVL
Trabalho realizado por:
- Kauan Moreira
- Samuel Sampaio Lins

## Sobre o projeto

Implementacao academica de uma Arvore Binaria de Busca Autobalanceada AVL para
numeros inteiros, em C++17. Os algoritmos de insercao, remocao e rotacao sao
implementados diretamente, com ponteiros e sem bibliotecas prontas de arvores.
No Windows, a interface principal e uma janela com campo de texto, botoes,
menus clicaveis e desenho da arvore. Usa a API nativa Win32, sem bibliotecas
graficas externas.

Todos os textos do programa usam ASCII, sem acentos, para evitar problemas
de codificacao.

## Executaveis prontos para Windows

Os executaveis Windows de 64 bits estao incluidos na pasta `bin/`:

- `bin/avl.exe`: interface grafica com desenho da arvore e zoom.
- `bin/avl_console.exe`: interface de terminal.

Depois de baixar e extrair o repositorio, abra `bin/avl.exe` com duplo clique.
No PowerShell, a partir da pasta do projeto:

```powershell
.\bin\avl.exe
.\bin\avl_console.exe
```

Esses arquivos foram compilados com g++ usando `-O2 -static -s`. O runtime C++
esta embutido; nao e necessario instalar MinGW nem copiar suas DLLs para executar.
As bibliotecas do proprio Windows continuam sendo utilizadas normalmente.

## Funcionalidades

- Janela Windows com entrada de texto, botoes, menus e navegacao por Tab.
- Desenho dos nos e conexoes, com altura, FB, barras de rolagem e zoom de 25% a 250%.
- Busca com destaque do caminho visitado e do valor encontrado.
- Historico de operacoes, rotacoes, percursos e informacoes dos nos.
- Insercao sem duplicatas, remocao e busca pela propriedade BST.
- Balanceamento automatico apos insercoes e remocoes.
- Rotacoes LL, RR, LR e RL, com mensagens indicando caso, no e FB anterior.
- Percursos em ordem, pre-ordem e pos-ordem.
- Quantidade de nos, raiz e altura no menu principal.
- Visualizacao textual dinamica com valor, altura e fator de balanceamento.
- Tabela de informacoes dos nos em pre-ordem.
- Limpeza da arvore mediante confirmacao (Sim/Nao na janela, S/N no terminal).
- Validacao de entradas e limites de `int`; no terminal, EOF encerra o programa.
- Liberacao de memoria na remocao, limpeza e destruicao da arvore.

## Compilacao

Na pasta do projeto, usando g++ com suporte a C++17.

**Janela Windows:**

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/gui.cpp src/AVLTree.cpp -Iinclude -mwindows -lcomctl32 -lgdi32 -luser32 -o avl.exe
```

As bibliotecas vinculadas sao fornecidas pelo Windows e pelo toolchain MinGW.
Nao e necessario instalar Qt, SDL ou outro framework grafico.

**Terminal Windows:**

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/AVLTree.cpp -Iinclude -o avl_console.exe
```

Nao e necessario alterar a pagina de codigo do console: as mensagens nao usam
acentos. A aplicacao de terminal tambem nao depende de cores ANSI.

Tambem ha suporte opcional a CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Em geradores de multiplas configuracoes, use `cmake --build build --config Debug`
e `ctest --test-dir build -C Debug --output-on-failure`; os executaveis ficam em
`build/Debug/`. Com MinGW, pode selecionar `-G "MinGW Makefiles"` na configuracao.
No Windows, CMake gera `avl.exe` (janela) e `avl_console.exe` (terminal).
Em Linux/macOS, gera `avl` (terminal).

## Execucao

Windows (PowerShell):

```powershell
.\avl.exe
```

Tambem pode abrir `avl.exe` com duplo clique no Explorador de Arquivos.
O executavel incluido no repositorio esta em `bin/avl.exe`:

```powershell
.\bin\avl.exe
```

Na janela, digite um inteiro e clique em **Inserir valor**, **Remover** ou
**Buscar**. Enter insere o valor; Tab navega pelos controles. Selecione um
percurso e clique em **Mostrar percurso**, ou use o menu **Consultar**.
**Informacoes dos nos** escreve a tabela no historico. **Limpar arvore** pede
confirmacao. O historico preserva as mensagens de rotacao e os resultados
recentes, com limite de tamanho para nao crescer indefinidamente.

Nos comuns aparecem em azul; o caminho da busca, em amarelo; o resultado
encontrado, em verde. O resultado da busca tambem aparece por escrito.
Sobre a area da arvore, a roda do mouse amplia ou reduz o desenho em passos de
25%. Os botoes **-**, **+** e **100%**, acima da arvore, tambem controlam o zoom.
O limite e de 25% a 250%; nos, conexoes e textos acompanham a escala. O zoom
pela roda usa a posicao do mouse como referencia; pelos botoes, usa o centro.
Use as barras de rolagem para explorar uma arvore grande; Shift + roda move
na horizontal. A janela pode ser redimensionada. Zoom nao altera os valores,
a estrutura nem o historico da AVL.

Para abrir a versao de terminal no Windows:

```powershell
.\avl_console.exe
```

Na versao de terminal, se compilado em `build`, execute `./build/avl` ou
`.\build\avl_console.exe`.
Escolha a opcao e digite um inteiro por linha. Sao aceitos numeros negativos,
zero e os extremos de `int`. Texto, fracoes, valores fora do intervalo e varios
valores na mesma linha sao rejeitados sem deixar residuos na entrada.

O menu do terminal oferece:

```text
[1] Inserir valor       [2] Remover valor
[3] Buscar valor        [4] Visualizar arvore
[5] Em ordem            [6] Pre-ordem
[7] Pos-ordem           [8] Informacoes dos nos
[9] Limpar arvore       [0] Sair
```

No terminal, apos inserir ou remover, a arvore atual e exibida automaticamente. Nenhuma
operacao exige limpar a tela ou executar comandos externos. O historico das
rotacoes permanece visivel. EOF encerra o programa mesmo durante a leitura de
um valor ou confirmacao; a arvore e liberada pelo destrutor.

Exemplo apos inserir 30, 20 e 10:

```text
[AVL] Rotacao LL no no 30 (FB=2).
Valor 10 inserido com sucesso.

Arvore atual:
20 [h=2, fb=0]
+-- E: 10 [h=1, fb=0]
`-- D: 30 [h=1, fb=0]
E: filho esquerdo | D: filho direito
```

Os rotulos E/D distinguem o lado mesmo quando existe apenas um filho.

## AVL

A propriedade BST exige que todos os valores da subarvore esquerda sejam
menores que o no, e os da direita, maiores. Duplicatas nao geram outro no.
A busca escolhe somente a subarvore compativel com o valor procurado.

A convencao de altura e **arvore vazia = 0; folha = 1**:

```text
altura(nullptr) = 0
altura(no) = 1 + max(altura(esquerda), altura(direita))
FB(no) = altura(esquerda) - altura(direita)
```

Uma AVL mantem `-1 <= FB <= 1` em todos os nos. Ao retornar da recursao de
insercao ou remocao, as alturas sao atualizadas e o no e rebalanceado quando
necessario. As rotacoes preservam a ordenacao BST e atualizam primeiro a altura
da antiga raiz, depois a da nova raiz da subarvore.

Insercao, remocao e busca tem custo `O(log n)`. Percursos, limpeza e validacao
custam `O(n)`. A visualizacao escreve tambem os prefixos de cada nivel e pode
produzir `O(n log n)` caracteres. O retorno dos percursos usa vetores com `O(n)`
de memoria. A pilha recursiva dos algoritmos da AVL usa `O(log n)`.

### Remocao

- Folha: libera o no e retorna ponteiro nulo.
- Um filho: libera o no e retorna seu filho para o pai.
- Dois filhos: copia o menor valor da subarvore direita (sucessor) e remove
  esse sucessor na subarvore direita.
- Raiz: recebe a nova raiz retornada pela operacao recursiva.

O rebalanceamento continua em todos os ancestrais. Uma unica remocao pode
exigir rotacoes em mais de um nivel. A quantidade de nos diminui apenas uma vez,
inclusive no caso com dois filhos. Remover um valor inexistente retorna `false`.

## Rotacoes

- **LL**: excesso a esquerda, com FB do filho esquerdo maior ou igual a zero.
  Executa rotacao simples a direita.
- **RR**: excesso a direita, com FB do filho direito menor ou igual a zero.
  Executa rotacao simples a esquerda.
- **LR**: excesso a esquerda, com FB do filho esquerdo negativo. Executa
  rotacao a esquerda no filho e a direita no no.
- **RL**: excesso a direita, com FB do filho direito positivo. Executa
  rotacao a direita no filho e a esquerda no no.

Os casos com FB do filho igual a zero sao importantes apos remocoes. Uma rotacao
dupla produz uma mensagem com seu caso LR/RL, representando as duas rotacoes
simples que a compoem. Nenhuma rotacao necessaria fica sem mensagem no programa.

## Estrutura do projeto

```text
include/
    AVLTree.h             No, interface da classe e metodos auxiliares
src/
    AVLTree.cpp           Algoritmos AVL, percursos, exibicao e validacao
    gui.cpp               Janela Windows, controles e desenho da arvore
    main.cpp              Versao de terminal, leitura e feedback
bin/
    avl.exe               Executavel Windows da interface grafica
    avl_console.exe       Executavel Windows da interface de terminal
tests/
    avl_tests.cpp         Testes dos algoritmos e invariantes
    memory_tests.cpp      Contagem de alocacoes em ciclos de uso
    test_cli.py           Testes do executavel com entradas simuladas
CMakeLists.txt            Compilacao e testes opcionais via CMake/CTest
README.md                 Instrucoes e decisoes de implementacao
```

Cada arvore possui exclusivamente seus nos. Copia e atribuicao sao desabilitadas
para impedir compartilhamento acidental e dupla liberacao. `root()` fornece
acesso para consulta/testes: nao altere nem libere nos por esse ponteiro.
O construtor padrao escreve rotacoes em `std::cout`; os testes fornecem outro
`std::ostream` para verificar as mensagens. Esse fluxo deve viver mais que a arvore.

## Testes

Compilacao e execucao direta dos testes C++:

```sh
g++ -std=c++17 -Wall -Wextra -pedantic tests/avl_tests.cpp src/AVLTree.cpp -Iinclude -o avl_tests
g++ -std=c++17 -Wall -Wextra -pedantic tests/memory_tests.cpp src/AVLTree.cpp -Iinclude -o avl_memory_tests
```

Windows:

```powershell
.\avl_tests.exe
.\avl_memory_tests.exe
python tests/test_cli.py avl_console.exe
```

Python 3 e opcional e usa apenas sua biblioteca padrao; nao e necessario para
compilar ou executar a aplicacao. CTest inclui o teste da interface se encontrar
o interpretador durante a configuracao.

A bateria C++ cobre:

- Insercoes LL `30,20,10`, RR `10,20,30`, LR `30,10,20` e RL `10,30,20`,
  verificando raiz 20, filhos 10/30 e a mensagem correspondente.
- Remocoes LL: inserir `30,20,40,10,25`, remover 40; RR: inserir
  `20,10,30,25,40`, remover 10; LR: inserir `30,20,40,25`, remover 40;
  RL: inserir `30,20,40,35`, remover 20.
- Cascata: inserir `7,14,4,12,1,6,15,10,9,11,13,16,3,8,5,2` e remover 12;
  deve ocorrer RR no no 14 e LL no no 13, em niveis distintos.
- Arvore vazia, no unico, duplicatas, buscas, valores inexistentes, negativos
  e extremos de `int`.
- Remocao de folha, nos com um ou dois filhos, sucessor com filho direito,
  raiz e remocoes repetidas da raiz ate esvaziar.
- Tres percursos, informacoes dos nos, visualizacao, limpeza repetida e reuso.
- 200 insercoes/remocoes em sequencias de teste, incluindo ordem crescente,
  decrescente e remocao repetida da raiz.
- 500 operacoes aleatorias reproduziveis, com sementes 7, 21 e 42,
  comparadas com um vetor ordenado de referencia.

Apos cada operacao das sequencias, sao verificadas a ordenacao BST global,
a quantidade de nos, a altura real de cada no, o FB e o limite AVL. Alem de
`isValid()`, ha uma auditoria independente nos testes.

Os testes automaticos de terminal verificam todas as opcoes, mensagens de rotacao,
entradas invalidas, overflow, confirmacao/cancelamento e EOF em cada leitura.
O teste de memoria conta alocacoes e liberacoes em 10 ciclos com 50 valores,
duplicatas, remocoes, limpeza e destruicao de arvores ainda preenchidas.
Ele detecta alocacoes pendentes nesses cenarios; nao substitui um detector de
acessos invalidos de memoria.

Quando o compilador oferecer AddressSanitizer/UndefinedBehaviorSanitizer,
uma verificacao adicional pode ser executada com:

```sh
g++ -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer tests/avl_tests.cpp src/AVLTree.cpp -Iinclude -o avl_tests_sanitized
./avl_tests_sanitized
```

O suporte a esses sanitizadores depende do compilador e da plataforma.
