#define NOMINMAX
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <psapi.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <memory>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Psapi.lib")

// ============================================================
// MODULO 1: CATALOGO
// ============================================================

struct Producto {
    int id{};
    int categoria{};
    double precio{};
    bool esPremium{};
    std::string nombre;
};

class Catalogo {
    std::vector<Producto> productos_;
    std::vector<std::size_t> porPrecio_;
    std::array<std::array<std::vector<std::size_t>, 2>, 20> porCategoriaNivel_;

    void reconstruirIndices() {
        std::sort(productos_.begin(), productos_.end(),
            [](const Producto& a, const Producto& b) { return a.id < b.id; });

        porPrecio_.resize(productos_.size());
        for (std::size_t i = 0; i < productos_.size(); ++i) porPrecio_[i] = i;
        std::sort(porPrecio_.begin(), porPrecio_.end(), [&](std::size_t a, std::size_t b) {
            if (productos_[a].precio != productos_[b].precio)
                return productos_[a].precio < productos_[b].precio;
            return productos_[a].id < productos_[b].id;
            });

        for (auto& categoria : porCategoriaNivel_)
            for (auto& nivel : categoria) nivel.clear();

        for (std::size_t i = 0; i < productos_.size(); ++i) {
            const auto& p = productos_[i];
            if (p.categoria >= 0 && p.categoria < 20)
                porCategoriaNivel_[p.categoria][p.esPremium ? 1 : 0].push_back(i);
        }
    }

public:
    std::size_t size() const { return productos_.size(); }

    void generar(std::size_t cantidad) {
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> precio(5.0, 2000.0);
        std::vector<Producto> nuevos;
        nuevos.reserve(cantidad);
        for (std::size_t i = 0; i < cantidad; ++i) {
            int id = static_cast<int>(i * 3 + 1);
            nuevos.push_back({ id, static_cast<int>(rng() % 20), precio(rng),
                (rng() % 2) == 0, "Producto-" + std::to_string(id) });
        }
        productos_ = std::move(nuevos);
        reconstruirIndices();
    }

    const Producto* buscarID(int id) const {
        auto it = std::lower_bound(productos_.begin(), productos_.end(), id,
            [](const Producto& p, int valor) { return p.id < valor; });
        return (it != productos_.end() && it->id == id) ? &*it : nullptr;
    }

    const Producto* buscarLineal(int id) const {
        for (const auto& p : productos_) {
            if (p.id == id) return &p;
        }
        return nullptr;
    }

    std::vector<const Producto*> buscarRango(double minimo, double maximo, std::size_t limite = 500) const {
        std::vector<const Producto*> resultado;
        if (minimo > maximo) return resultado;
        auto primero = std::lower_bound(porPrecio_.begin(), porPrecio_.end(), minimo,
            [&](std::size_t i, double v) { return productos_[i].precio < v; });
        for (auto it = primero; it != porPrecio_.end() &&
            productos_[*it].precio <= maximo && resultado.size() < limite; ++it) {
            resultado.push_back(&productos_[*it]);
        }
        return resultado;
    }

    std::vector<const Producto*> buscarCategoria(int categoria, bool premium, std::size_t limite = 500) const {
        std::vector<const Producto*> resultado;
        if (categoria < 0 || categoria >= 20) return resultado;
        const auto& indices = porCategoriaNivel_[categoria][premium ? 1 : 0];
        resultado.reserve((std::min)(limite, indices.size()));
        for (std::size_t i : indices) {
            if (resultado.size() >= limite) break;
            resultado.push_back(&productos_[i]);
        }
        return resultado;
    }
};

// ============================================================
// MODULO 2: CONFIGURADOR
// ============================================================

struct Componente { int categoria; int indice; double precio; bool premium; };
struct ReglaDependencia { int categoriaA; int opcionA; int categoriaB; int opcionB; };

class Configurador {
public:
    std::vector<std::vector<Componente>> categorias;
    std::vector<ReglaDependencia> dependencias;
    double presupuesto = 0.0;
    int nivelRequerido = -1;
    std::uint64_t nodos = 0;

    bool parcialValida(const std::vector<const Componente*>& seleccion) const {
        double costo = 0.0;
        for (const auto* c : seleccion) {
            costo += c->precio;
            if (costo > presupuesto) return false;
            if (!seleccion.empty() && c->premium != seleccion.front()->premium) return false;
            if (nivelRequerido >= 0 && c->premium != (nivelRequerido == 1)) return false;
        }
        for (const auto& regla : dependencias) {
            bool tieneA = false, tieneB = false;
            const Componente* a = nullptr;
            const Componente* b = nullptr;
            for (const auto* c : seleccion) {
                if (c->categoria == regla.categoriaA) { tieneA = true; a = c; }
                if (c->categoria == regla.categoriaB) { tieneB = true; b = c; }
            }
            if (tieneA && a->indice == regla.opcionA && tieneB && b->indice != regla.opcionB)
                return false;
        }
        return true;
    }

    void resolverDesde(std::size_t nivel, std::vector<const Componente*>& actual,
        bool soloPrimera, std::vector<std::vector<const Componente*>>& salida, bool usarPoda) {
        if (nivel == categorias.size()) {
            if (!usarPoda) {
                if (parcialValida(actual)) salida.push_back(actual);
            }
            else {
                salida.push_back(actual);
            }
            return;
        }
        for (const auto& opcion : categorias[nivel]) {
            actual.push_back(&opcion);
            ++nodos;

            if (!usarPoda || parcialValida(actual)) {
                resolverDesde(nivel + 1, actual, soloPrimera, salida, usarPoda);
                if (soloPrimera && !salida.empty()) { actual.pop_back(); return; }
            }
            actual.pop_back();
        }
    }

    std::vector<std::vector<const Componente*>> generar(bool soloPrimera = false, bool usarPoda = true) {
        nodos = 0;
        std::vector<std::vector<const Componente*>> salida;
        std::vector<const Componente*> actual;
        resolverDesde(0, actual, soloPrimera, salida, usarPoda);
        return salida;
    }
};

static Configurador crearConfigurador(int nCategorias, int nOpciones, double presupuesto) {
    Configurador c;
    c.presupuesto = presupuesto;
    c.categorias.resize(nCategorias);
    std::mt19937 rng(7);
    for (int cat = 0; cat < nCategorias; ++cat) {
        for (int op = 0; op < nOpciones; ++op) {
            bool premium = (rng() % 2) == 0;
            double precio = premium ? 150.0 + (rng() % 151) : 50.0 + (rng() % 101);
            c.categorias[cat].push_back({ cat, op, precio, premium });
        }
    }
    if (nCategorias >= 2 && nOpciones > 3)
        c.dependencias.push_back({ 0, 3, 1, 1 });
    return c;
}

// ============================================================
// INTERFAZ GRAFICA
// ============================================================

enum {
    ID_BTN_GENERAR = 1001, ID_BTN_BENCH_CAT, ID_BTN_BUSCAR, ID_BTN_RANGO,
    ID_BTN_CATEGORIA, ID_BTN_CONFIG, ID_EDIT_ID, ID_EDIT_MIN, ID_EDIT_MAX,
    ID_EDIT_CAT, ID_EDIT_PRESUPUESTO, ID_LISTVIEW, ID_ESTADO, ID_EDIT_CANTIDAD,
    ID_BTN_CATEGORIA_PREMIUM = 1101
};

static Catalogo g_catalogo;
static HWND g_lista = nullptr;
static HWND g_estado = nullptr;

static std::wstring aW(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n > 0 ? n : 0, L'\0');
    if (n > 1) {
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
        w.resize(static_cast<std::size_t>(n - 1));
    }
    return w;
}

static std::string leerTexto(HWND h) {
    int n = GetWindowTextLengthA(h);
    std::string s(static_cast<std::size_t>(n) + 1, '\0');
    if (n) GetWindowTextA(h, s.data(), n + 1);
    s.resize(static_cast<std::size_t>(n));
    return s;
}

static HWND crearControl(DWORD ex, const wchar_t* clase, const wchar_t* texto,
    DWORD estilo, int x, int y, int w, int h, HWND padre, int id) {
    return CreateWindowExW(ex, clase, texto, WS_CHILD | WS_VISIBLE | estilo,
        x, y, w, h, padre, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr), nullptr);
}

static void estado(const std::wstring& s) { SetWindowTextW(g_estado, s.c_str()); }

static void configurarTabla(const std::vector<std::wstring>& cabeceras, const std::vector<int>& anchos) {
    SendMessageW(g_lista, LVM_DELETEALLITEMS, 0, 0);
    while (SendMessageW(g_lista, LVM_DELETECOLUMN, 0, 0)) {}

    for (size_t i = 0; i < cabeceras.size(); ++i) {
        LVCOLUMNW col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = const_cast<LPWSTR>(cabeceras[i].c_str());
        col.cx = anchos[i];
        col.iSubItem = static_cast<int>(i);
        SendMessageW(g_lista, LVM_INSERTCOLUMNW, i, reinterpret_cast<LPARAM>(&col));
    }
}

static void agregarFila(const std::vector<std::wstring>& valores) {
    LVITEMW item = {};
    item.mask = LVIF_TEXT;
    item.iItem = ListView_GetItemCount(g_lista);
    item.pszText = const_cast<LPWSTR>(valores[0].c_str());
    int index = static_cast<int>(SendMessageW(g_lista, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item)));

    for (size_t i = 1; i < valores.size(); ++i) {
        ListView_SetItemText(g_lista, index, i, const_cast<LPWSTR>(valores[i].c_str()));
    }
}

// ============================================================
// LÓGICA DE LA INTERFAZ
// ============================================================

static void mostrarProductos(const std::vector<const Producto*>& productos) {
    configurarTabla({ L"ID", L"Nombre", L"Categoria", L"Precio (S/)", L"Nivel" }, { 80, 250, 100, 100, 100 });

    for (const Producto* p : productos) {
        std::wostringstream osPrecio;
        osPrecio << std::fixed << std::setprecision(2) << p->precio;

        agregarFila({
            std::to_wstring(p->id),
            aW(p->nombre),
            std::to_wstring(p->categoria),
            osPrecio.str(),
            (p->esPremium ? L"Premium" : L"Economico")
            });
    }
    estado(L"Resultados: " + std::to_wstring(productos.size()) + L" (limite 500). Total catálogo: " + std::to_wstring(g_catalogo.size()));
}

static bool leerDouble(HWND h, double& valor) {
    try {
        std::string s = leerTexto(h);
        if (s.empty()) return false;
        std::size_t pos = 0;
        valor = std::stod(s, &pos);
        return pos == s.size() && std::isfinite(valor);
    }
    catch (...) { return false; }
}

static bool leerInt(HWND h, int& valor) {
    try {
        std::string s = leerTexto(h);
        if (s.empty()) return false;
        std::size_t pos = 0;
        valor = std::stoi(s, &pos);
        return pos == s.size();
    }
    catch (...) { return false; }
}

static void ejecutarBenchmarkCatalogo(HWND hwnd) {
    if (g_catalogo.size() == 0) {
        MessageBoxW(hwnd, L"Primero genera el catalogo.", L"Aviso", MB_ICONINFORMATION);
        return;
    }

    configurarTabla({ L"Algoritmo", L"Tiempo por consulta", L"Total de consultas" }, { 200, 200, 200 });

    int Q = 10000;
    std::vector<int> ids(Q);
    std::mt19937 rng(42);
    for (int i = 0; i < Q; ++i) ids[i] = (rng() % g_catalogo.size()) * 3 + 1;

    auto t0 = std::chrono::steady_clock::now();
    volatile int encontradosBin = 0;
    for (int id : ids) if (g_catalogo.buscarID(id)) encontradosBin++;
    auto t1 = std::chrono::steady_clock::now();
    double usBin = std::chrono::duration<double, std::micro>(t1 - t0).count();

    int Q_Lineal = 100;
    t0 = std::chrono::steady_clock::now();
    volatile int encontradosLin = 0;
    for (int i = 0; i < Q_Lineal; ++i) if (g_catalogo.buscarLineal(ids[i])) encontradosLin++;
    t1 = std::chrono::steady_clock::now();
    double usLin = std::chrono::duration<double, std::micro>(t1 - t0).count();

    auto formatMs = [](double us) {
        std::wostringstream os;
        os << std::fixed << std::setprecision(3) << us << L" us";
        return os.str();
        };

    agregarFila({ L"Busqueda Binaria (O(log N))", formatMs(usBin / Q), std::to_wstring(Q) });
    agregarFila({ L"Busqueda Lineal (O(N))", formatMs(usLin / Q_Lineal), std::to_wstring(Q_Lineal) });

    estado(L"Benchmark de catálogo finalizado.");
}

static void ejecutarConfigurador(HWND hwnd) {
    double presupuesto = 0;
    if (!leerDouble(GetDlgItem(hwnd, ID_EDIT_PRESUPUESTO), presupuesto) || presupuesto <= 0) {
        MessageBoxW(hwnd, L"Ingresa un presupuesto valido.", L"Error", MB_ICONWARNING);
        return;
    }
    Configurador cf = crearConfigurador(4, 10, presupuesto);

    auto inicio_bt = std::chrono::steady_clock::now();
    auto resultadosBT = cf.generar(false, true);
    auto fin_bt = std::chrono::steady_clock::now();
    std::uint64_t nodosBT = cf.nodos;
    double usBT = std::chrono::duration<double, std::micro>(fin_bt - inicio_bt).count();

    auto inicio_fb = std::chrono::steady_clock::now();
    auto resultadosFB = cf.generar(false, false);
    auto fin_fb = std::chrono::steady_clock::now();
    std::uint64_t nodosFB = cf.nodos;
    double usFB = std::chrono::duration<double, std::micro>(fin_fb - inicio_fb).count();

    configurarTabla({ L"Metrica / Componente", L"Valor / Detalle", L"Nodos explorados" }, { 250, 250, 150 });

    if (resultadosBT.empty()) {
        agregarFila({ L"Resultado", L"No se encontró configuración válida", L"-" });
    }
    else {
        const auto& seleccion = resultadosBT[0];
        double total = 0;
        for (const auto* c : seleccion) {
            total += c->precio;
            std::wostringstream os;
            os << L"Categoria " << c->categoria << L" | Opcion " << c->indice
                << L" | S/ " << std::fixed << std::setprecision(2) << c->precio
                << L" | " << (c->premium ? L"Premium" : L"Economico");
            agregarFila({ L"Componente elegido", os.str(), L"-" });
        }
        std::wostringstream osTotal;
        osTotal << L"S/ " << std::fixed << std::setprecision(2) << total;
        agregarFila({ L"TOTAL CONFIGURACION", osTotal.str(), L"-" });
    }

    agregarFila({ L"---", L"---", L"---" });

    agregarFila({ L"Backtracking (con poda)", std::to_wstring((long long)usBT) + L" us", std::to_wstring(nodosBT) });
    agregarFila({ L"Fuerza Bruta (sin poda)", std::to_wstring((long long)usFB) + L" us", std::to_wstring(nodosFB) });

    double reduccion = (nodosFB > 0) ? (100.0 * (nodosFB - nodosBT) / nodosFB) : 0;
    std::wostringstream osRed;
    osRed << std::fixed << std::setprecision(2) << reduccion << L"%";
    agregarFila({ L"Reduccion de nodos", osRed.str(), L"-" });

    estado(L"Configurador ejecutado. Comparativa mostrada en la tabla.");
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        auto label = [&](const wchar_t* t, int x, int y, int w) {
            HWND h = crearControl(0, L"STATIC", t, 0, x, y, w, 22, hwnd, 0);
            SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            };
        label(L"BACKEND E-COMMERCE | MODULOS 1 Y 2", 20, 15, 500);

        label(L"Modulo 1 - Catalogo", 20, 50, 180);
        crearControl(WS_EX_CLIENTEDGE, L"EDIT", L"1000000", ES_AUTOHSCROLL, 20, 75, 120, 26, hwnd, ID_EDIT_CANTIDAD);
        crearControl(0, L"BUTTON", L"Generar 1 millon", BS_PUSHBUTTON, 150, 75, 145, 26, hwnd, ID_BTN_GENERAR);
        crearControl(0, L"BUTTON", L"Benchmark Busqueda", BS_PUSHBUTTON, 305, 75, 150, 26, hwnd, ID_BTN_BENCH_CAT);

        label(L"ID:", 20, 112, 30);
        crearControl(WS_EX_CLIENTEDGE, L"EDIT", L"100", ES_AUTOHSCROLL, 50, 108, 90, 25, hwnd, ID_EDIT_ID);
        crearControl(0, L"BUTTON", L"Buscar ID", BS_PUSHBUTTON, 150, 108, 100, 25, hwnd, ID_BTN_BUSCAR);

        label(L"Precio desde:", 270, 112, 90);
        crearControl(WS_EX_CLIENTEDGE, L"EDIT", L"100", ES_AUTOHSCROLL, 360, 108, 75, 25, hwnd, ID_EDIT_MIN);
        label(L"Hasta:", 445, 112, 45);
        crearControl(WS_EX_CLIENTEDGE, L"EDIT", L"500", ES_AUTOHSCROLL, 490, 108, 75, 25, hwnd, ID_EDIT_MAX);
        crearControl(0, L"BUTTON", L"Filtrar precio", BS_PUSHBUTTON, 575, 108, 115, 25, hwnd, ID_BTN_RANGO);

        label(L"Categoria (0-19):", 20, 148, 115);
        crearControl(WS_EX_CLIENTEDGE, L"EDIT", L"0", ES_AUTOHSCROLL, 135, 144, 55, 25, hwnd, ID_EDIT_CAT);
        crearControl(0, L"BUTTON", L"Ver economicos", BS_PUSHBUTTON, 200, 144, 120, 25, hwnd, ID_BTN_CATEGORIA);
        crearControl(0, L"BUTTON", L"Ver premium", BS_PUSHBUTTON, 330, 144, 110, 25, hwnd, ID_BTN_CATEGORIA_PREMIUM);

        label(L"Modulo 2 - Configurador | Presupuesto:", 460, 148, 250);
        crearControl(WS_EX_CLIENTEDGE, L"EDIT", L"900", ES_AUTOHSCROLL, 710, 144, 85, 25, hwnd, ID_EDIT_PRESUPUESTO);
        crearControl(0, L"BUTTON", L"Calcular + Benchmark", BS_PUSHBUTTON, 805, 144, 165, 25, hwnd, ID_BTN_CONFIG);

        g_lista = crearControl(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
            LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VSCROLL | WS_HSCROLL,
            20, 185, 950, 390, hwnd, ID_LISTVIEW);

        ListView_SetExtendedListViewStyle(g_lista, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

        g_estado = crearControl(0, L"STATIC", L"Listo. Genera un millon de productos.",
            SS_LEFT, 20, 585, 950, 35, hwnd, ID_ESTADO);

        SendMessageW(g_lista, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(g_estado, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_BTN_GENERAR) {
            int n = 0;
            if (!leerInt(GetDlgItem(hwnd, ID_EDIT_CANTIDAD), n) || n <= 0 || n > 5000000) {
                MessageBoxW(hwnd, L"Cantidad permitida: entre 1 y 5,000,000.", L"Validacion", MB_ICONWARNING);
                return 0;
            }
            estado(L"Generando catalogo e índices...");
            auto t0 = std::chrono::steady_clock::now();
            g_catalogo.generar(static_cast<std::size_t>(n));
            auto t1 = std::chrono::steady_clock::now();
            SendMessageW(g_lista, LVM_DELETEALLITEMS, 0, 0);
            auto ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            estado(L"Catalogo generado: " + std::to_wstring(g_catalogo.size()) +
                L" productos | Tiempo: " + std::to_wstring(static_cast<long long>(ms)) + L" ms");
        }
        else if (id == ID_BTN_BENCH_CAT) {
            ejecutarBenchmarkCatalogo(hwnd);
        }
        else if (id == ID_BTN_BUSCAR) {
            int idProducto = 0;
            if (!leerInt(GetDlgItem(hwnd, ID_EDIT_ID), idProducto)) {
                MessageBoxW(hwnd, L"ID no valido.", L"Validacion", MB_ICONWARNING); return 0;
            }
            const Producto* p = g_catalogo.buscarID(idProducto);
            if (!p) {
                configurarTabla({ L"Resultado" }, { 200 });
                agregarFila({ L"No se encontro el producto." });
            }
            else {
                mostrarProductos({ p });
            }
            estado(L"Búsqueda por ID mediante lower_bound (O(log n)).");
        }
        else if (id == ID_BTN_RANGO) {
            double lo = 0, hi = 0;
            if (!leerDouble(GetDlgItem(hwnd, ID_EDIT_MIN), lo) ||
                !leerDouble(GetDlgItem(hwnd, ID_EDIT_MAX), hi) || lo > hi) {
                MessageBoxW(hwnd, L"Rango de precios no valido.", L"Validacion", MB_ICONWARNING); return 0;
            }
            mostrarProductos(g_catalogo.buscarRango(lo, hi));
            estado(L"Filtro por precio mediante indice ordenado.");
        }
        else if (id == ID_BTN_CATEGORIA || id == ID_BTN_CATEGORIA_PREMIUM) {
            int cat = 0;
            if (!leerInt(GetDlgItem(hwnd, ID_EDIT_CAT), cat) || cat < 0 || cat >= 20) {
                MessageBoxW(hwnd, L"Categoria debe estar entre 0 y 19.", L"Validacion", MB_ICONWARNING); return 0;
            }
            mostrarProductos(g_catalogo.buscarCategoria(cat, id == ID_BTN_CATEGORIA_PREMIUM));
            estado(L"Filtro por categoria y nivel mediante indice precalculado.");
        }
        else if (id == ID_BTN_CONFIG) {
            ejecutarConfigurador(hwnd);
        }
        return 0;
    }
    case WM_SIZE: {
        if (g_lista && g_estado) {
            int w = LOWORD(lp), h = HIWORD(lp);
            MoveWindow(g_lista, 20, 185, (std::max)(300, w - 40), (std::max)(150, h - 245), TRUE);
            MoveWindow(g_estado, 20, h - 45, w - 40, 30, TRUE);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);
    const wchar_t CLASS_NAME[] = L"EcommerceModulos12GUI";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClassW(&wc)) return 1;

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Backend E-commerce - Modulos 1 y 2",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1020, 700,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}