// Backend E-commerce (caso Amazon) - Interfaz grafica Win32.
// Integra los Modulos 1 a 4 y el flujo end-to-end. La logica vive en include/*.hpp.
#define NOMINMAX
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <fstream>
#include "../include/integracion.hpp"

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Psapi.lib")

enum {
    ID_BTN_GENERAR = 1001, ID_BTN_BENCH_CAT, ID_BTN_BUSCAR, ID_BTN_RANGO, ID_BTN_CATEGORIA, ID_BTN_CONFIG,
    ID_EDIT_ID, ID_EDIT_MIN, ID_EDIT_MAX, ID_EDIT_CAT, ID_EDIT_PRESUPUESTO, ID_LISTVIEW, ID_ESTADO, ID_EDIT_CANTIDAD,
    ID_BTN_CATEGORIA_PREMIUM, ID_BTN_IMPORTAR, ID_BTN_GREEDY, ID_BTN_MOD2, ID_BTN_MOD3, ID_BTN_MOD4, ID_BTN_FLUJO,
    ID_BTN_TODO, ID_BTN_EXPORTAR, ID_EDIT_PREFIJO, ID_BTN_TRIE, ID_BTN_ESCALA
};

static Catalogo g_catalogo;
static Trie g_trie;
static HWND g_lista = nullptr, g_estado = nullptr;
static Tabla g_ultima; // ultima tabla mostrada (para exportar)
static std::vector<std::string> g_cabeceras;

static std::wstring aW(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n > 0 ? n : 0, L'\0');
    if (n > 1) { MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n); w.resize(static_cast<std::size_t>(n - 1)); }
    return w;
}
static std::string aU(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n > 0 ? n : 0, '\0');
    if (n > 1) { WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr); s.resize(static_cast<std::size_t>(n - 1)); }
    return s;
}
static std::string leerTexto(HWND h) {
    int n = GetWindowTextLengthW(h);
    std::wstring s(static_cast<std::size_t>(n) + 1, L'\0');
    if (n) GetWindowTextW(h, s.data(), n + 1);
    s.resize(static_cast<std::size_t>(n));
    return aU(s);
}
static HWND crearControl(DWORD ex, const wchar_t* clase, const wchar_t* texto, DWORD estilo, int x, int y, int w, int h, HWND padre, int id) {
    return CreateWindowExW(ex, clase, texto, WS_CHILD | WS_VISIBLE | estilo, x, y, w, h, padre,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
}
static void estado(const std::wstring& s) { SetWindowTextW(g_estado, s.c_str()); }

static void configurarTabla(const std::vector<std::string>& cab, const std::vector<int>& anchos) {
    g_ultima.clear(); g_cabeceras = cab;
    SendMessageW(g_lista, LVM_DELETEALLITEMS, 0, 0);
    while (SendMessageW(g_lista, LVM_DELETECOLUMN, 0, 0)) {}
    for (std::size_t i = 0; i < cab.size(); ++i) {
        std::wstring w = aW(cab[i]);
        LVCOLUMNW col = {}; col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = w.data(); col.cx = anchos[i]; col.iSubItem = static_cast<int>(i);
        SendMessageW(g_lista, LVM_INSERTCOLUMNW, i, reinterpret_cast<LPARAM>(&col));
    }
}
static void agregarFila(const std::vector<std::string>& v) {
    std::wstring w0 = aW(v[0]);
    LVITEMW item = {}; item.mask = LVIF_TEXT; item.iItem = ListView_GetItemCount(g_lista); item.pszText = w0.data();
    int idx = static_cast<int>(SendMessageW(g_lista, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item)));
    for (std::size_t i = 1; i < v.size(); ++i) { std::wstring w = aW(v[i]); ListView_SetItemText(g_lista, idx, static_cast<int>(i), w.data()); }
    g_ultima.push_back({ v.size() > 0 ? v[0] : "", v.size() > 1 ? v[1] : "", v.size() > 2 ? v[2] : "" });
}
static void mostrarTabla(const std::string& seccion, const Tabla& t, bool limpiar = true) {
    if (limpiar) configurarTabla({ "Prueba / Metrica", "Resultado", "Detalle / Verificacion" }, { 300, 420, 300 });
    agregarFila({ ">> " + seccion, "", "" });
    for (auto& f : t) agregarFila({ f.a, f.b, f.c });
}
static void mostrarProductos(const std::vector<const Producto*>& ps) {
    configurarTabla({ "ID", "Nombre", "Categoria", "Precio (S/)", "Nivel" }, { 90, 260, 90, 110, 100 });
    for (auto* p : ps) agregarFila({ std::to_string(p->id), p->nombre, std::to_string(p->categoria), fmt(p->precio), p->esPremium ? "Premium" : "Economico" });
    estado(L"Resultados: " + std::to_wstring(ps.size()) + L" (limite 500). Total catalogo: " + std::to_wstring(g_catalogo.size()));
}
static bool leerDouble(HWND h, double& v) {
    try { std::string s = leerTexto(h); if (s.empty()) return false; std::size_t p = 0; v = std::stod(s, &p); return p == s.size() && std::isfinite(v); }
    catch (...) { return false; }
}
static bool leerInt(HWND h, int& v) {
    try { std::string s = leerTexto(h); if (s.empty()) return false; std::size_t p = 0; v = std::stoi(s, &p); return p == s.size(); }
    catch (...) { return false; }
}
static void reconstruirTrie() {
    g_trie = Trie();
    std::size_t paso = std::max<std::size_t>(1, g_catalogo.size() / 50000);
    for (std::size_t i = 0; i < g_catalogo.size(); i += paso) g_trie.insertar(g_catalogo.enPosicion(i).nombre);
}
static bool requiereCatalogo(HWND hwnd) {
    if (g_catalogo.size()) return true;
    MessageBoxW(hwnd, L"Primero genera o importa el catalogo.", L"Aviso", MB_ICONINFORMATION);
    return false;
}

static void importarCSV(HWND hwnd) {
    wchar_t ruta[MAX_PATH] = L"";
    OPENFILENAMEW of = {}; of.lStructSize = sizeof(of); of.hwndOwner = hwnd;
    of.lpstrFilter = L"CSV (*.csv)\0*.csv\0Todos\0*.*\0"; of.lpstrFile = ruta; of.nMaxFile = MAX_PATH; of.Flags = OFN_FILEMUSTEXIST;
    if (!GetOpenFileNameW(&of)) return;
    std::string err;
    Catalogo nuevo;
    if (!nuevo.importarCSV(aU(ruta), err)) { MessageBoxW(hwnd, aW(err).c_str(), L"CSV invalido", MB_ICONWARNING); return; }
    g_catalogo = std::move(nuevo); reconstruirTrie();
    std::vector<const Producto*> v; for (auto& p : g_catalogo.productos()) { if (v.size() >= 500) break; v.push_back(&p); }
    mostrarProductos(v);
    estado(L"CSV importado: " + std::to_wstring(g_catalogo.size()) + L" productos validados.");
}

static void exportarResultados(HWND hwnd) {
    if (g_ultima.empty()) { MessageBoxW(hwnd, L"No hay resultados para exportar.", L"Aviso", MB_ICONINFORMATION); return; }
    wchar_t ruta[MAX_PATH] = L"resultados.csv";
    OPENFILENAMEW of = {}; of.lStructSize = sizeof(of); of.hwndOwner = hwnd; of.lpstrFilter = L"CSV (*.csv)\0*.csv\0";
    of.lpstrFile = ruta; of.nMaxFile = MAX_PATH; of.lpstrDefExt = L"csv"; of.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameW(&of)) return;
    std::ofstream out(ruta);
    auto q = [](const std::string& s) { std::string r = "\""; for (char ch : s) { if (ch == '"') r += '"'; r += ch; } return r + "\""; };
    for (std::size_t i = 0; i < g_cabeceras.size() && i < 3; ++i) out << (i ? "," : "") << q(g_cabeceras[i]);
    out << "\n";
    for (auto& f : g_ultima) out << q(f.a) << "," << q(f.b) << "," << q(f.c) << "\n";
    estado(L"Resultados exportados.");
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        auto fuente = [&](HWND h) { SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE); return h; };
        auto label = [&](const wchar_t* t, int x, int y, int w) { fuente(crearControl(0, L"STATIC", t, 0, x, y, w, 22, hwnd, 0)); };
        auto boton = [&](const wchar_t* t, int x, int y, int w, int id) { fuente(crearControl(0, L"BUTTON", t, BS_PUSHBUTTON, x, y, w, 26, hwnd, id)); };
        auto edit = [&](const wchar_t* t, int x, int y, int w, int id) { fuente(crearControl(WS_EX_CLIENTEDGE, L"EDIT", t, ES_AUTOHSCROLL, x, y, w, 26, hwnd, id)); };

        label(L"MOTOR DE PERSONALIZACION Y ASIGNACION LOGISTICA - CASO AMAZON | MODULOS 1 A 4", 20, 12, 700);

        label(L"Modulo 1 - Catalogo:", 20, 45, 130);
        edit(L"1000000", 150, 42, 90, ID_EDIT_CANTIDAD);
        boton(L"Generar", 245, 42, 80, ID_BTN_GENERAR);
        boton(L"Importar CSV", 330, 42, 100, ID_BTN_IMPORTAR);
        boton(L"Benchmark busqueda", 435, 42, 140, ID_BTN_BENCH_CAT);
        label(L"Prefijo (Trie):", 590, 45, 90);
        edit(L"lap", 680, 42, 80, ID_EDIT_PREFIJO);
        boton(L"Sugerir", 765, 42, 80, ID_BTN_TRIE);

        label(L"ID:", 20, 82, 25);
        edit(L"100", 45, 78, 90, ID_EDIT_ID);
        boton(L"Buscar ID", 140, 78, 85, ID_BTN_BUSCAR);
        label(L"Precio desde:", 240, 82, 85);
        edit(L"100", 325, 78, 70, ID_EDIT_MIN);
        label(L"hasta:", 400, 82, 40);
        edit(L"500", 440, 78, 70, ID_EDIT_MAX);
        boton(L"Filtrar precio", 515, 78, 100, ID_BTN_RANGO);
        label(L"Categoria (0-19):", 630, 82, 105);
        edit(L"0", 735, 78, 40, ID_EDIT_CAT);
        boton(L"Economicos", 780, 78, 90, ID_BTN_CATEGORIA);
        boton(L"Premium", 875, 78, 80, ID_BTN_CATEGORIA_PREMIUM);

        label(L"Configurador - Presupuesto:", 20, 118, 170);
        edit(L"900", 190, 114, 70, ID_EDIT_PRESUPUESTO);
        boton(L"Backtracking + benchmark", 265, 114, 175, ID_BTN_CONFIG);
        boton(L"Asignacion Greedy", 445, 114, 140, ID_BTN_GREEDY);
        boton(L"Escalabilidad 1k/10k/100k", 590, 114, 185, ID_BTN_ESCALA);

        boton(L"Modulo 2: Estructuras", 20, 150, 165, ID_BTN_MOD2);
        boton(L"Modulo 3: Grafos", 190, 150, 140, ID_BTN_MOD3);
        boton(L"Modulo 4: Prog. dinamica", 335, 150, 175, ID_BTN_MOD4);
        boton(L"Flujo end-to-end", 515, 150, 130, ID_BTN_FLUJO);
        boton(L"Ejecutar todo", 650, 150, 110, ID_BTN_TODO);
        boton(L"Exportar resultados CSV", 765, 150, 175, ID_BTN_EXPORTAR);

        g_lista = crearControl(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VSCROLL | WS_HSCROLL,
            20, 190, 1040, 420, hwnd, ID_LISTVIEW);
        ListView_SetExtendedListViewStyle(g_lista, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        g_estado = crearControl(0, L"STATIC", L"Listo. Genera o importa el catalogo para comenzar.", SS_LEFT, 20, 620, 1040, 30, hwnd, ID_ESTADO);
        fuente(g_lista); fuente(g_estado);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        HCURSOR anterior = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
        if (id == ID_BTN_GENERAR) {
            int n = 0;
            if (!leerInt(GetDlgItem(hwnd, ID_EDIT_CANTIDAD), n) || n <= 0 || n > 5000000) {
                MessageBoxW(hwnd, L"Cantidad permitida: entre 1 y 5,000,000.", L"Validacion", MB_ICONWARNING); return 0;
            }
            estado(L"Generando catalogo e indices...");
            Cronometro c; g_catalogo.generar(static_cast<std::size_t>(n)); reconstruirTrie();
            SendMessageW(g_lista, LVM_DELETEALLITEMS, 0, 0);
            estado(L"Catalogo generado: " + std::to_wstring(g_catalogo.size()) + L" productos | Tiempo: " + std::to_wstring((long long)c.ms()) + L" ms");
        } else if (id == ID_BTN_IMPORTAR) {
            importarCSV(hwnd);
        } else if (id == ID_BTN_BENCH_CAT) {
            if (requiereCatalogo(hwnd)) { mostrarTabla("Catalogo O(log N)", pruebaCatalogo(g_catalogo)); estado(L"Benchmark de catalogo finalizado."); }
        } else if (id == ID_BTN_TRIE) {
            if (requiereCatalogo(hwnd)) {
                std::string p = leerTexto(GetDlgItem(hwnd, ID_EDIT_PREFIJO));
                Cronometro c; auto s = g_trie.sugerir(p, 20); double us = c.us();
                configurarTabla({ "Sugerencia (Trie)", "Coincidencias con el prefijo", "-" }, { 300, 300, 200 });
                for (auto& x : s) agregarFila({ x, std::to_string(g_trie.contarPrefijo(x)), "-" });
                estado(L"Sugerencias por prefijo en " + aW(fmt(us, 2)) + L" us. Total con prefijo: " + std::to_wstring(g_trie.contarPrefijo(p)));
            }
        } else if (id == ID_BTN_BUSCAR) {
            int idp = 0;
            if (!leerInt(GetDlgItem(hwnd, ID_EDIT_ID), idp)) { MessageBoxW(hwnd, L"ID no valido.", L"Validacion", MB_ICONWARNING); return 0; }
            const Producto* p = g_catalogo.buscarID(idp);
            if (!p) { configurarTabla({ "Resultado" }, { 250 }); agregarFila({ "No se encontro el producto." }); }
            else mostrarProductos({ p });
            estado(L"Busqueda por ID mediante lower_bound (O(log n)).");
        } else if (id == ID_BTN_RANGO) {
            double lo = 0, hi = 0;
            if (!leerDouble(GetDlgItem(hwnd, ID_EDIT_MIN), lo) || !leerDouble(GetDlgItem(hwnd, ID_EDIT_MAX), hi) || lo > hi) {
                MessageBoxW(hwnd, L"Rango de precios no valido.", L"Validacion", MB_ICONWARNING); return 0;
            }
            mostrarProductos(g_catalogo.buscarRango(lo, hi));
        } else if (id == ID_BTN_CATEGORIA || id == ID_BTN_CATEGORIA_PREMIUM) {
            int cat = 0;
            if (!leerInt(GetDlgItem(hwnd, ID_EDIT_CAT), cat) || cat < 0 || cat >= 20) {
                MessageBoxW(hwnd, L"Categoria debe estar entre 0 y 19.", L"Validacion", MB_ICONWARNING); return 0;
            }
            mostrarProductos(g_catalogo.buscarCategoria(cat, id == ID_BTN_CATEGORIA_PREMIUM));
        } else if (id == ID_BTN_CONFIG) {
            double pres = 0;
            if (!leerDouble(GetDlgItem(hwnd, ID_EDIT_PRESUPUESTO), pres) || pres <= 0) { MessageBoxW(hwnd, L"Ingresa un presupuesto valido.", L"Error", MB_ICONWARNING); return 0; }
            mostrarTabla("Configurador Backtracking con poda", pruebaConfigurador(pres));
            estado(L"Configurador ejecutado.");
        } else if (id == ID_BTN_GREEDY) {
            mostrarTabla("Asignacion Greedy First Fit", pruebaGreedy()); estado(L"Motor Greedy evaluado (velocidad y optimalidad).");
        } else if (id == ID_BTN_MOD2) {
            mostrarTabla("Modulo 2 - Estructuras de datos", pruebaModulo2()); estado(L"Modulo 2 verificado.");
        } else if (id == ID_BTN_MOD3) {
            estado(L"Ejecutando grafos (incluye pruebas de volumen)...");
            mostrarTabla("Modulo 3 - Grafos y logistica", pruebaModulo3()); estado(L"Modulo 3 verificado.");
        } else if (id == ID_BTN_MOD4) {
            mostrarTabla("Modulo 4 - Programacion dinamica", pruebaModulo4()); estado(L"Modulo 4 verificado.");
        } else if (id == ID_BTN_FLUJO) {
            if (requiereCatalogo(hwnd)) { mostrarTabla("Flujo end-to-end (1 000 solicitudes)", ejecutarFlujoCompleto(g_catalogo, 1000).tabla); estado(L"Flujo completo ejecutado."); }
        } else if (id == ID_BTN_ESCALA) {
            if (requiereCatalogo(hwnd)) { estado(L"Ejecutando escalabilidad (puede tardar unos segundos)...");
                mostrarTabla("Escalabilidad IND-I03", pruebaEscalabilidad(g_catalogo)); estado(L"Escalabilidad medida."); }
        } else if (id == ID_BTN_TODO) {
            if (requiereCatalogo(hwnd)) {
                estado(L"Ejecutando todos los modulos...");
                mostrarTabla("1.1 Catalogo", pruebaCatalogo(g_catalogo));
                double pres = 900; leerDouble(GetDlgItem(hwnd, ID_EDIT_PRESUPUESTO), pres);
                mostrarTabla("1.2 Configurador", pruebaConfigurador(pres), false);
                mostrarTabla("1.3 Greedy", pruebaGreedy(), false);
                mostrarTabla("2 Estructuras", pruebaModulo2(), false);
                mostrarTabla("3 Grafos", pruebaModulo3(), false);
                mostrarTabla("4 Programacion dinamica", pruebaModulo4(), false);
                mostrarTabla("10 Flujo end-to-end", ejecutarFlujoCompleto(g_catalogo, 1000).tabla, false);
                mostrarTabla("10.3 Escalabilidad", pruebaEscalabilidad(g_catalogo), false);
                agregarFila({ "Memoria pico del proceso", fmt(memoriaPicoMB(), 1) + " MB", "-" });
                estado(L"Todos los modulos ejecutados. Usa 'Exportar resultados CSV' para guardar los benchmarks.");
            }
        } else if (id == ID_BTN_EXPORTAR) {
            exportarResultados(hwnd);
        }
        SetCursor(anterior);
        return 0;
    }
    case WM_SIZE: {
        if (g_lista && g_estado) {
            int w = LOWORD(lp), h = HIWORD(lp);
            MoveWindow(g_lista, 20, 190, (std::max)(300, w - 40), (std::max)(150, h - 240), TRUE);
            MoveWindow(g_estado, 20, h - 40, w - 40, 30, TRUE);
        }
        return 0;
    }
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icc);
    const wchar_t CLASS_NAME[] = L"EcommerceModulos12GUI";
    WNDCLASSW wc{}; wc.lpfnWndProc = WndProc; wc.hInstance = instance; wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); wc.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClassW(&wc)) return 1;
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Motor E-commerce Amazon - Modulos 1 a 4", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 720, nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, show); UpdateWindow(hwnd);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    return static_cast<int>(msg.wParam);
}
