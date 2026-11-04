// Interface Windows. Toda a logica da AVL permanece em AVLTree.cpp.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "AVLTree.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr char WindowClass[] = "AVLInteractiveWindow";
constexpr char CanvasClass[] = "AVLTreeCanvas";
constexpr COLORREF Background = RGB(245, 247, 251);
constexpr COLORREF Ink = RGB(30, 41, 59);
constexpr COLORREF Blue = RGB(37, 99, 235);
enum Control { Input = 100, Insert, Remove, Search, Traversal, ShowTraversal,
               NodeInfo, Clear, Exit, About, InOrder, PreOrder, PostOrder,
               ZoomOut, ZoomIn, ZoomReset };

struct VisualNode { const Node* node; int x; int y; };
struct Edge { POINT parent; POINT child; };

struct Application {
    std::ostringstream rotations;
    AVLTree tree{rotations};
    HWND window = nullptr;
    HWND input = nullptr;
    HWND canvas = nullptr;
    HWND summary = nullptr;
    HWND historyBox = nullptr;
    HWND status = nullptr;
    HWND canvasLabel = nullptr;
    HWND historyLabel = nullptr;
    HWND zoomLabel = nullptr;
    HWND zoomOut = nullptr;
    HWND zoomIn = nullptr;
    HWND zoomReset = nullptr;
    HFONT regular = nullptr;
    HFONT title = nullptr;
    HFONT mono = nullptr;
    HFONT nodeFont = nullptr;
    HFONT small = nullptr;
    HFONT zoomNodeFont = nullptr;
    HFONT zoomInfoFont = nullptr;
    HBRUSH background = nullptr;
    std::vector<HWND> sidebar;
    std::vector<VisualNode> nodes;
    std::vector<Edge> edges;
    std::vector<int> searchPath;
    std::optional<int> found;
    std::string history;
    int contentWidth = 0;
    int contentHeight = 0;
    int zoomPercent = 100;
    int wheelRemainder = 0;
};

Application* application(HWND window) {
    return reinterpret_cast<Application*>(GetWindowLongPtrA(window, GWLP_USERDATA));
}

HFONT makeFont(int size, int weight, const char* family) {
    return CreateFontA(-size, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH, family);
}

HWND control(Application& app, const char* type, const char* text, DWORD style,
             int id, int x, int y, int width, int height, HFONT font = nullptr) {
    HWND result = CreateWindowExA(0, type, text, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, app.window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleA(nullptr), nullptr);
    SendMessageA(result, WM_SETFONT, reinterpret_cast<WPARAM>(font ? font : app.regular), TRUE);
    return result;
}

void appendLog(Application& app, const std::string& text) {
    for (char character : text) {
        if (character == '\n') { app.history += '\r'; }
        app.history += character;
    }
    if (app.history.size() > 100000) {
        const auto cut = app.history.find('\n', app.history.size() - 80000);
        app.history.erase(0, cut == std::string::npos ? 20000 : cut + 1);
    }
    SetWindowTextA(app.historyBox, app.history.c_str());
    const auto length = static_cast<WPARAM>(app.history.size());
    SendMessageA(app.historyBox, EM_SETSEL, length, static_cast<LPARAM>(length));
    SendMessageA(app.historyBox, EM_SCROLLCARET, 0, 0);
}

POINT layoutNode(Application& app, const Node* node, int depth, int& rank) {
    POINT left{};
    POINT right{};
    if (node->left) { left = layoutNode(app, node->left, depth + 1, rank); }
    POINT point{80 + rank * 140, 52 + depth * 100};
    ++rank;
    app.nodes.push_back({node, static_cast<int>(point.x), static_cast<int>(point.y)});
    if (node->right) { right = layoutNode(app, node->right, depth + 1, rank); }
    if (node->left) { app.edges.push_back({point, left}); }
    if (node->right) { app.edges.push_back({point, right}); }
    return point;
}

int scaled(const Application& app, int value) {
    return MulDiv(value, app.zoomPercent, 100);
}

void updateScrollbars(Application& app) {
    // Uma barra pode mudar o espaco disponivel para a outra.
    for (int pass = 0; pass < 2; ++pass) {
        RECT client{};
        GetClientRect(app.canvas, &client);
        SCROLLINFO info{};
        info.cbSize = sizeof(info);
        info.fMask = SIF_RANGE | SIF_PAGE;
        info.nMax = std::max(0, scaled(app, app.contentWidth) - 1);
        info.nPage = static_cast<UINT>(std::max(0L, client.right));
        SetScrollInfo(app.canvas, SB_HORZ, &info, TRUE);
        GetClientRect(app.canvas, &client);
        info.nMax = std::max(0, scaled(app, app.contentHeight) - 1);
        info.nPage = static_cast<UINT>(std::max(0L, client.bottom));
        SetScrollInfo(app.canvas, SB_VERT, &info, TRUE);
    }
}

void changeZoom(Application& app, int percent, std::optional<POINT> anchor = std::nullopt) {
    percent = std::clamp(percent, 25, 250);
    if (percent == app.zoomPercent) { return; }
    HFONT nodeFont = makeFont(MulDiv(19, percent, 100), FW_SEMIBOLD, "Segoe UI");
    HFONT infoFont = makeFont(MulDiv(14, percent, 100), FW_NORMAL, "Segoe UI");
    if (!nodeFont || !infoFont) {
        DeleteObject(nodeFont);
        DeleteObject(infoFont);
        return;
    }
    RECT client{};
    GetClientRect(app.canvas, &client);
    const POINT point = anchor.value_or(POINT{client.right / 2, client.bottom / 2});
    const int oldPadding = std::max(0, (static_cast<int>(client.right) - scaled(app, app.contentWidth)) / 2);
    const int oldX = GetScrollPos(app.canvas, SB_HORZ);
    const int oldY = GetScrollPos(app.canvas, SB_VERT);
    const double ratio = static_cast<double>(percent) / app.zoomPercent;
    app.zoomPercent = percent;
    DeleteObject(app.zoomNodeFont);
    DeleteObject(app.zoomInfoFont);
    app.zoomNodeFont = nodeFont;
    app.zoomInfoFont = infoFont;
    updateScrollbars(app);
    GetClientRect(app.canvas, &client);
    const int newPadding = std::max(0, (static_cast<int>(client.right) - scaled(app, app.contentWidth)) / 2);
    // Manter o ponto sob o mouse; nos botoes, usar o centro da visualizacao.
    SetScrollPos(app.canvas, SB_HORZ,
        static_cast<int>(std::lround((point.x + oldX - oldPadding) * ratio + newPadding - point.x)), TRUE);
    SetScrollPos(app.canvas, SB_VERT,
        static_cast<int>(std::lround((point.y + oldY) * ratio - point.y)), TRUE);
    const std::string label = "Zoom: " + std::to_string(percent) + "%";
    SetWindowTextA(app.zoomLabel, label.c_str());
    EnableWindow(app.zoomOut, percent > 25);
    EnableWindow(app.zoomIn, percent < 250);
    InvalidateRect(app.canvas, nullptr, FALSE);
}

void refresh(Application& app) {
    const std::string summary = "Nos: " + std::to_string(app.tree.size())
        + "      Altura: " + std::to_string(app.tree.height()) + "      Raiz: "
        + (app.tree.root() ? std::to_string(app.tree.root()->value) : "--")
        + "      FB = altura(esquerda) - altura(direita)";
    SetWindowTextA(app.summary, summary.c_str());
    app.nodes.clear();
    app.edges.clear();
    int rank = 0;
    if (app.tree.root()) { layoutNode(app, app.tree.root(), 0, rank); }
    app.contentWidth = rank * 140 + 20;
    app.contentHeight = app.tree.height() * 100 + 15;
    updateScrollbars(app);
    const std::string description = "Arvore AVL com " + std::to_string(app.tree.size()) + " nos";
    SetWindowTextA(app.canvas, description.c_str());
    InvalidateRect(app.canvas, nullptr, FALSE);
}

void showTraversal(Application& app, int order) {
    const auto values = order == 1 ? app.tree.preOrder()
                      : order == 2 ? app.tree.postOrder() : app.tree.inOrder();
    std::ostringstream text;
    text << (order == 1 ? "Pre-ordem" : order == 2 ? "Pos-ordem" : "Em ordem") << ":";
    if (values.empty()) { text << " (arvore vazia)"; }
    for (int value : values) { text << ' ' << value; }
    text << '\n';
    appendLog(app, text.str());
    SetWindowTextA(app.status, "Percurso exibido no historico.");
}

void handleCommand(Application& app, int id) {
    if (id == ZoomIn || id == ZoomOut || id == ZoomReset) {
        changeZoom(app, id == ZoomReset ? 100 : app.zoomPercent + (id == ZoomIn ? 25 : -25));
    } else if (id == Insert || id == Remove || id == Search) {
        char text[128]{};
        GetWindowTextA(app.input, text, static_cast<int>(sizeof(text)));
        std::istringstream input(text);
        long long number = 0;
        char extra = '\0';
        if (!(input >> number) || (input >> extra) ||
            number < std::numeric_limits<int>::lowest() ||
            number > std::numeric_limits<int>::max()) {
            SetWindowTextA(app.status, "Entrada invalida. Digite um numero inteiro.");
            SetFocus(app.input);
            SendMessageA(app.input, EM_SETSEL, 0, -1);
            return;
        }
        const int value = static_cast<int>(number);
        app.searchPath.clear();
        app.found.reset();
        app.rotations.str("");
        app.rotations.clear();
        std::string message;
        if (id == Insert) {
            message = app.tree.insert(value) ? "Valor " + std::to_string(value) + " inserido com sucesso."
                : "O valor " + std::to_string(value) + " ja existe na arvore.";
        } else if (id == Remove) {
            message = app.tree.remove(value) ? "Valor " + std::to_string(value) + " removido com sucesso."
                : "Valor " + std::to_string(value) + " nao encontrado na arvore.";
        } else {
            // Mesmo caminho BST da busca: destacar apenas os nos visitados.
            const Node* node = app.tree.root();
            while (node) {
                app.searchPath.push_back(node->value);
                if (node->value == value) { app.found = value; break; }
                node = value < node->value ? node->left : node->right;
            }
            message = "Valor " + std::to_string(value)
                + (app.found ? " encontrado." : " nao encontrado.");
        }
        appendLog(app, app.rotations.str() + message + "\n");
        SetWindowTextA(app.status, message.c_str());
        refresh(app);
        if (id != Search) { SetWindowTextA(app.input, ""); }
        SetFocus(app.input);
        SendMessageA(app.input, EM_SETSEL, 0, -1);
    } else if (id == ShowTraversal) {
        showTraversal(app, static_cast<int>(SendDlgItemMessageA(app.window, Traversal, CB_GETCURSEL, 0, 0)));
    } else if (id == InOrder || id == PreOrder || id == PostOrder) {
        showTraversal(app, id - InOrder);
    } else if (id == NodeInfo) {
        std::ostringstream text;
        app.tree.printNodeInfo(text);
        appendLog(app, text.str());
        SetWindowTextA(app.status, "Informacoes dos nos exibidas no historico.");
    } else if (id == Clear) {
        if (MessageBoxA(app.window, "Remover todos os elementos da arvore?", "Limpar arvore",
                        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
            app.tree.clear();
            app.searchPath.clear();
            app.found.reset();
            appendLog(app, "Arvore limpa.\n");
            SetWindowTextA(app.status, "Arvore limpa.");
            refresh(app);
        }
    } else if (id == Exit) {
        DestroyWindow(app.window);
    } else if (id == About) {
        MessageBoxA(app.window, "Arvore AVL em C++17\n\n"
            "Insira, remova e busque inteiros. Enter insere o valor.\n"
            "As rotacoes LL, RR, LR e RL aparecem no historico.\n"
            "Roda do mouse sobre a arvore: ampliar ou reduzir.\n"
            "Botoes - / + / 100% tambem controlam o zoom.\n"
            "Use as barras de rolagem para navegar pela arvore.\n\n"
            "Altura: vazia = 0; folha = 1.\n"
            "Azul: no comum. Amarelo: caminho da busca. Verde: encontrado.",
            "Sobre e ajuda", MB_OK | MB_ICONINFORMATION);
    }
}

LRESULT CALLBACK inputProcedure(HWND window, UINT message, WPARAM wParam,
                                 LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (message == WM_GETDLGCODE && lParam &&
        reinterpret_cast<MSG*>(lParam)->wParam == VK_RETURN) { return DLGC_WANTMESSAGE; }
    if (message == WM_KEYDOWN && wParam == VK_RETURN) {
        SendMessageA(GetParent(window), WM_COMMAND, Insert, 0);
        return 0;
    }
    return DefSubclassProc(window, message, wParam, lParam);
}

void createControls(Application& app) {
    control(app, "STATIC", "ARVORE AVL", SS_LEFT, 0, 24, 14, 400, 38, app.title);
    control(app, "STATIC", "Explore insercoes, remocoes e balanceamento em tempo real.",
            SS_LEFT, 0, 26, 54, 900, 26);
    app.summary = control(app, "STATIC", "", SS_LEFT, 0, 26, 94, 1000, 28);
    control(app, "STATIC", "VALOR INTEIRO", SS_LEFT, 0, 26, 148, 230, 24, app.small);
    app.input = control(app, "EDIT", "", WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
                        Input, 24, 176, 230, 36);
    SendMessageA(app.input, EM_SETLIMITTEXT, 64, 0);
    SendMessageA(app.input, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));
    SetWindowSubclass(app.input, inputProcedure, 1, 0);
    app.sidebar.push_back(control(app, "BUTTON", "Inserir valor", WS_TABSTOP | BS_OWNERDRAW,
                                  Insert, 24, 224, 230, 40));
    app.sidebar.push_back(control(app, "BUTTON", "Remover", WS_TABSTOP,
                                  Remove, 24, 274, 110, 36));
    app.sidebar.push_back(control(app, "BUTTON", "Buscar", WS_TABSTOP,
                                  Search, 144, 274, 110, 36));
    control(app, "STATIC", "PERCURSOS", SS_LEFT, 0, 26, 338, 230, 24, app.small);
    HWND combo = control(app, "COMBOBOX", "", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                         Traversal, 24, 365, 230, 150);
    for (const char* text : {"Em ordem", "Pre-ordem", "Pos-ordem"}) {
        SendMessageA(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text));
    }
    SendMessageA(combo, CB_SETCURSEL, 0, 0);
    app.sidebar.push_back(control(app, "BUTTON", "Mostrar percurso", WS_TABSTOP,
                                  ShowTraversal, 24, 408, 230, 36));
    app.sidebar.push_back(control(app, "BUTTON", "Informacoes dos nos", WS_TABSTOP,
                                  NodeInfo, 24, 454, 230, 36));
    app.sidebar.push_back(control(app, "BUTTON", "Limpar arvore", WS_TABSTOP,
                                  Clear, 24, 516, 230, 36));
    control(app, "STATIC", "Enter para inserir.\nRoda sobre a arvore: zoom.\nShift + roda: mover para os lados.",
            SS_LEFT, 0, 26, 580, 230, 80, app.small);
    app.canvasLabel = control(app, "STATIC", "ESTRUTURA DA ARVORE", SS_LEFT,
                              0, 280, 124, 400, 24, app.small);
    app.zoomOut = control(app, "BUTTON", "-", WS_TABSTOP, ZoomOut, 800, 118, 36, 28);
    app.zoomLabel = control(app, "STATIC", "Zoom: 100%", SS_CENTER,
                            0, 840, 124, 94, 24, app.small);
    app.zoomIn = control(app, "BUTTON", "+", WS_TABSTOP, ZoomIn, 938, 118, 36, 28);
    app.zoomReset = control(app, "BUTTON", "100%", WS_TABSTOP, ZoomReset, 978, 118, 76, 28);
    app.canvas = CreateWindowExA(WS_EX_CLIENTEDGE, CanvasClass, "Arvore AVL vazia",
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, 280, 150, 800, 340,
        app.window, nullptr, GetModuleHandleA(nullptr), &app);
    app.historyLabel = control(app, "STATIC", "HISTORICO E RESULTADOS", SS_LEFT,
                               0, 280, 510, 700, 24, app.small);
    app.historyBox = control(app, "EDIT", "", WS_BORDER | WS_TABSTOP | ES_MULTILINE |
        ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
        0, 280, 538, 800, 150, app.mono);
    SendMessageA(app.historyBox, EM_SETLIMITTEXT, 200000, 0);
    app.status = control(app, "STATIC", "Pronto. Digite um inteiro para comecar.", SS_LEFT,
                         0, 24, 700, 1000, 24, app.small);
    HMENU menu = CreateMenu();
    HMENU file = CreatePopupMenu();
    AppendMenuA(file, MF_STRING, Clear, "Limpar arvore...");
    AppendMenuA(file, MF_SEPARATOR, 0, nullptr);
    AppendMenuA(file, MF_STRING, Exit, "Sair");
    AppendMenuA(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(file), "Arquivo");
    HMENU traversal = CreatePopupMenu();
    AppendMenuA(traversal, MF_STRING, InOrder, "Em ordem");
    AppendMenuA(traversal, MF_STRING, PreOrder, "Pre-ordem");
    AppendMenuA(traversal, MF_STRING, PostOrder, "Pos-ordem");
    AppendMenuA(traversal, MF_STRING, NodeInfo, "Informacoes dos nos");
    AppendMenuA(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(traversal), "Consultar");
    AppendMenuA(menu, MF_STRING, About, "Ajuda");
    SetMenu(app.window, menu);
    appendLog(app, "Pronto. Insira valores para construir sua arvore AVL.\n"
                   "h = altura | fb = fator de balanceamento\n");
    refresh(app);
}

void layoutControls(Application& app) {
    if (!app.canvas) { return; }
    RECT client{};
    GetClientRect(app.window, &client);
    const int width = static_cast<int>(client.right);
    const int height = static_cast<int>(client.bottom);
    MoveWindow(app.summary, 26, 94, width - 52, 28, TRUE);
    MoveWindow(app.canvasLabel, 280, 124, width - 564, 24, TRUE);
    MoveWindow(app.zoomOut, width - 278, 118, 36, 28, TRUE);
    MoveWindow(app.zoomLabel, width - 238, 124, 94, 24, TRUE);
    MoveWindow(app.zoomIn, width - 140, 118, 36, 28, TRUE);
    MoveWindow(app.zoomReset, width - 100, 118, 76, 28, TRUE);
    MoveWindow(app.canvas, 280, 150, width - 304, height - 390, TRUE);
    MoveWindow(app.historyLabel, 280, height - 224, width - 304, 24, TRUE);
    MoveWindow(app.historyBox, 280, height - 196, width - 304, 150, TRUE);
    MoveWindow(app.status, 24, height - 30, width - 48, 24, TRUE);
    updateScrollbars(app);
}

void drawText(HDC dc, HFONT font, const std::string& text, RECT rectangle,
              COLORREF color, UINT flags = DT_CENTER | DT_SINGLELINE | DT_VCENTER) {
    HGDIOBJ previous = SelectObject(dc, font);
    SetTextColor(dc, color);
    DrawTextA(dc, text.c_str(), -1, &rectangle, flags);
    SelectObject(dc, previous);
}

void paintCanvas(Application& app) {
    PAINTSTRUCT paint{};
    HDC target = BeginPaint(app.canvas, &paint);
    RECT client{};
    GetClientRect(app.canvas, &client);
    HDC dc = CreateCompatibleDC(target);
    HBITMAP bitmap = CreateCompatibleBitmap(target, std::max(1L, client.right), std::max(1L, client.bottom));
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    FillRect(dc, &client, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    SetBkMode(dc, TRANSPARENT);
    if (app.nodes.empty()) {
        RECT line{0, client.bottom / 2 - 38, client.right, client.bottom / 2};
        drawText(dc, app.nodeFont, "Sua arvore comeca aqui", line, Ink);
        OffsetRect(&line, 0, 38);
        drawText(dc, app.regular, "Digite um valor e clique em Inserir valor.", line, RGB(100, 116, 139));
    } else {
        const int offsetX = -GetScrollPos(app.canvas, SB_HORZ)
            + std::max(0, (static_cast<int>(client.right) - scaled(app, app.contentWidth)) / 2);
        const int offsetY = -GetScrollPos(app.canvas, SB_VERT);
        HPEN pen = CreatePen(PS_SOLID, std::max(1, scaled(app, 2)), RGB(185, 199, 218));
        HGDIOBJ oldPen = SelectObject(dc, pen);
        for (const auto& edge : app.edges) {
            MoveToEx(dc, scaled(app, edge.parent.x) + offsetX,
                     scaled(app, edge.parent.y + 22) + offsetY, nullptr);
            LineTo(dc, scaled(app, edge.child.x) + offsetX,
                   scaled(app, edge.child.y - 22) + offsetY);
        }
        SelectObject(dc, oldPen);
        DeleteObject(pen);
        for (const auto& entry : app.nodes) {
            const int x = scaled(app, entry.x) + offsetX;
            const int y = scaled(app, entry.y) + offsetY;
            if (x < -scaled(app, 70) || x > client.right + scaled(app, 70) ||
                y < -scaled(app, 60) || y > client.bottom + scaled(app, 60)) { continue; }
            const bool found = app.found && *app.found == entry.node->value;
            const bool visited = std::find(app.searchPath.begin(), app.searchPath.end(),
                                           entry.node->value) != app.searchPath.end();
            const COLORREF color = found ? RGB(22, 130, 92) : visited ? RGB(180, 117, 16) : Blue;
            HBRUSH brush = CreateSolidBrush(color);
            HGDIOBJ oldBrush = SelectObject(dc, brush);
            oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
            RoundRect(dc, x - scaled(app, 62), y - scaled(app, 22),
                      x + scaled(app, 62), y + scaled(app, 22), scaled(app, 18), scaled(app, 18));
            SelectObject(dc, oldBrush);
            SelectObject(dc, oldPen);
            DeleteObject(brush);
            drawText(dc, app.zoomNodeFont, std::to_string(entry.node->value),
                     RECT{x - scaled(app, 61), y - scaled(app, 22),
                          x + scaled(app, 61), y + scaled(app, 22)}, RGB(255, 255, 255));
            const std::string metrics = "h=" + std::to_string(entry.node->height)
                + "  fb=" + std::to_string(AVLTree::balanceFactor(entry.node));
            drawText(dc, app.zoomInfoFont, metrics,
                     RECT{x - scaled(app, 66), y + scaled(app, 25),
                          x + scaled(app, 66), y + scaled(app, 47)}, Ink);
        }
    }
    BitBlt(target, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
    EndPaint(app.canvas, &paint);
}

LRESULT CALLBACK canvasProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTA*>(lParam);
        SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    }
    Application* app = application(window);
    if (!app) { return DefWindowProcA(window, message, wParam, lParam); }
    if (message == WM_PAINT) { paintCanvas(*app); return 0; }
    if (message == WM_ERASEBKGND) { return 1; }
    if (message == WM_SIZE) { InvalidateRect(window, nullptr, FALSE); return 0; }
    if (message == WM_MOUSEWHEEL && !(GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT)) {
        app->wheelRemainder += GET_WHEEL_DELTA_WPARAM(wParam);
        const int steps = app->wheelRemainder / WHEEL_DELTA;
        app->wheelRemainder %= WHEEL_DELTA;
        if (steps != 0) {
            POINT anchor{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(window, &anchor);
            changeZoom(*app, app->zoomPercent + steps * 25, anchor);
        }
        return 0;
    }
    if (message == WM_HSCROLL || message == WM_VSCROLL || message == WM_MOUSEWHEEL) {
        const int bar = message == WM_HSCROLL ||
            (message == WM_MOUSEWHEEL && (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT)) ? SB_HORZ : SB_VERT;
        SCROLLINFO info{};
        info.cbSize = sizeof(info);
        info.fMask = SIF_ALL;
        GetScrollInfo(window, bar, &info);
        if (message == WM_MOUSEWHEEL) {
            info.nPos -= GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA * 60;
        } else {
            switch (LOWORD(wParam)) {
                case SB_LINEUP: info.nPos -= 40; break;
                case SB_LINEDOWN: info.nPos += 40; break;
                case SB_PAGEUP: info.nPos -= static_cast<int>(info.nPage); break;
                case SB_PAGEDOWN: info.nPos += static_cast<int>(info.nPage); break;
                case SB_THUMBTRACK: info.nPos = info.nTrackPos; break;
                default: break;
            }
        }
        info.fMask = SIF_POS;
        SetScrollInfo(window, bar, &info, TRUE);
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTA*>(lParam);
        SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        application(window)->window = window;
    }
    Application* app = application(window);
    if (!app) { return DefWindowProcA(window, message, wParam, lParam); }
    switch (message) {
        case WM_CREATE: createControls(*app); return 0;
        case WM_SIZE: layoutControls(*app); return 0;
        case WM_MOUSEWHEEL: {
            POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            RECT canvas{};
            GetWindowRect(app->canvas, &canvas);
            if (PtInRect(&canvas, point)) {
                return SendMessageA(app->canvas, message, wParam, lParam);
            }
            break;
        }
        case WM_GETMINMAXINFO: {
            auto* limits = reinterpret_cast<MINMAXINFO*>(lParam);
            limits->ptMinTrackSize = {1000, 790};
            return 0;
        }
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == 0) {
                try { handleCommand(*app, LOWORD(wParam)); }
                catch (const std::exception&) {
                    MessageBoxA(window, "Nao foi possivel concluir a operacao.", "Erro", MB_OK | MB_ICONERROR);
                }
            }
            return 0;
        case WM_DRAWITEM: {
            const auto* item = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
            if (item->CtlID != Insert) { break; }
            HBRUSH brush = CreateSolidBrush((item->itemState & ODS_SELECTED) ? RGB(29, 78, 216) : Blue);
            FillRect(item->hDC, &item->rcItem, brush);
            DeleteObject(brush);
            SetBkMode(item->hDC, TRANSPARENT);
            drawText(item->hDC, app->regular, "Inserir valor", item->rcItem, RGB(255, 255, 255));
            if (item->itemState & ODS_FOCUS) {
                RECT focus = item->rcItem;
                InflateRect(&focus, -4, -4);
                DrawFocusRect(item->hDC, &focus);
            }
            return TRUE;
        }
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wParam);
            if (reinterpret_cast<HWND>(lParam) == app->historyBox) {
                SetBkColor(dc, RGB(255, 255, 255));
                SetTextColor(dc, Ink);
                return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
            }
            SetTextColor(dc, Ink);
            SetBkColor(dc, Background);
            return reinterpret_cast<LRESULT>(app->background);
        }
        case WM_DESTROY: PostQuitMessage(0); return 0;
        default: break;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}
} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
    SetProcessDPIAware();
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    Application app;
    app.regular = makeFont(17, FW_NORMAL, "Segoe UI");
    app.title = makeFont(30, FW_BOLD, "Segoe UI");
    app.mono = makeFont(15, FW_NORMAL, "Consolas");
    app.nodeFont = makeFont(19, FW_SEMIBOLD, "Segoe UI");
    app.small = makeFont(14, FW_NORMAL, "Segoe UI");
    app.zoomNodeFont = makeFont(19, FW_SEMIBOLD, "Segoe UI");
    app.zoomInfoFont = makeFont(14, FW_NORMAL, "Segoe UI");
    app.background = CreateSolidBrush(Background);
    WNDCLASSA mainClass{};
    mainClass.lpfnWndProc = windowProcedure;
    mainClass.hInstance = instance;
    mainClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    mainClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    mainClass.hbrBackground = app.background;
    mainClass.lpszClassName = WindowClass;
    RegisterClassA(&mainClass);
    WNDCLASSA canvasClass{};
    canvasClass.lpfnWndProc = canvasProcedure;
    canvasClass.hInstance = instance;
    canvasClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    canvasClass.lpszClassName = CanvasClass;
    RegisterClassA(&canvasClass);
    HWND window = CreateWindowExA(WS_EX_CONTROLPARENT, WindowClass, "Arvore AVL Interativa",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
        1220, 850, nullptr, nullptr, instance, &app);
    int result = 1;
    if (window) {
        layoutControls(app);
        ShowWindow(window, show);
        UpdateWindow(window);
        SetFocus(app.input);
        MSG message{};
        int received = 0;
        while ((received = static_cast<int>(GetMessageA(&message, nullptr, 0, 0))) > 0) {
            // O campo de texto pode consumir a roda quando ainda tem o foco.
            // Sobre o desenho, encaminhar antes de entregar ao controle focado.
            if (message.message == WM_MOUSEWHEEL) {
                POINT point{GET_X_LPARAM(message.lParam), GET_Y_LPARAM(message.lParam)};
                RECT canvas{};
                GetWindowRect(app.canvas, &canvas);
                if (PtInRect(&canvas, point)) {
                    SendMessageA(app.canvas, message.message, message.wParam, message.lParam);
                    continue;
                }
            }
            if (!IsDialogMessageA(window, &message)) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
        }
        result = received == -1 ? 1 : static_cast<int>(message.wParam);
    }
    for (HFONT font : {app.regular, app.title, app.mono, app.nodeFont, app.small,
                      app.zoomNodeFont, app.zoomInfoFont}) { DeleteObject(font); }
    DeleteObject(app.background);
    return result;
}
