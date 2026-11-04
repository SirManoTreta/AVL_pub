"""Testes do executavel real. Apenas biblioteca padrao do Python 3."""

from pathlib import Path
import subprocess
import sys


DEFAULT_EXECUTABLE = "build/avl_console.exe" if sys.platform == "win32" else "build/avl"
EXECUTABLE = Path(sys.argv[1] if len(sys.argv) > 1 else DEFAULT_EXECUTABLE).resolve()


def run(data):
    result = subprocess.run(
        [str(EXECUTABLE)], input=data, encoding="utf-8",
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=10, check=True,
    )
    assert not result.stderr, result.stderr
    assert result.stdout.isascii(), "Saida deve permanecer sem acentos"
    assert "Programa encerrado." in result.stdout
    return result.stdout


def require(text, *messages):
    for message in messages:
        assert message in text, f"Mensagem ausente: {message}"


for rotation, values in {
    "LL": [30, 20, 10], "RR": [10, 20, 30],
    "LR": [30, 10, 20], "RL": [10, 30, 20],
}.items():
    output = run("".join(f"1\n{value}\n" for value in values) + "4\n5\n6\n7\n8\n0\n")
    require(output, f"[AVL] Rotacao {rotation}", "Raiz: 20", "Altura atual: 2",
            "Em ordem: 10 20 30", "Pre-ordem: 20 10 30", "Pos-ordem: 10 30 20",
            "20 [h=2, fb=0]", "E: 10", "D: 30", "Valor | Altura |   FB")
print("[OK] Menu, quatro rotacoes, percursos, tabela e visualizacao")

output = run("4\n5\n6\n7\n8\n2\n99\n3\n99\n1\n25\n1\n25\n3\n25\n2\n25\n0\n")
require(output, "Arvore vazia.", "Valor nao encontrado na arvore.",
        "Valor 99 nao encontrado.", "Valor 25 encontrado.",
        "O valor 25 ja existe na arvore.", "Valor 25 removido com sucesso.")
print("[OK] Arvore vazia, duplicata, buscas e remocoes")

invalid = ["texto", "", "1.5", "1 2", "1x", "1e2", "2147483648",
           "-2147483649", "999999999999999999999999999999999999999"]
output = run("\n".join(invalid) + "\n42\n1\nabc\n-2147483648\n1\n+2147483647\n5\n0\n")
assert output.count("Entrada invalida. Digite um numero inteiro.") == len(invalid) + 1
require(output, "Opcao invalida.", "Em ordem: -2147483648 2147483647")
print("[OK] Texto, linha vazia, fracao, sufixos, limites e overflow")

output = run("1\n10\n9\ntalvez\nS N\nn\n3\n10\n9\n s \n3\n10\n0\n")
require(output, "Resposta invalida.", "Operacao cancelada.", "Valor 10 encontrado.",
        "Arvore limpa.", "Valor 10 nao encontrado.", "Quantidade de nos: 0")
assert output.count("Resposta invalida.") == 2
print("[OK] Limpeza confirmada, cancelada e resposta invalida")

for data in ["", "1\n", "2\n", "3\n", "9\n", "abc\n", "1\n20\n9\n"]:
    run(data)
print("[OK] EOF no menu, valor, confirmacao e apos entrada invalida")
print("Todos os testes de interface aprovados.")
