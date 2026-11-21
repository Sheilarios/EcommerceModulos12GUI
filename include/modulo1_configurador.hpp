#pragma once
// MODULO 1 - Componente 2: Configurador con Backtracking y poda.
// RN-F03 una opcion por categoria, RN-F04 presupuesto, mismo nivel comercial, dependencias.
#include <cstdint>
#include <random>
#include <vector>

struct Componente { int categoria; int indice; double precio; bool premium; int idProducto; };
struct ReglaDependencia { int categoriaA; int opcionA; int categoriaB; int opcionB; };

class Configurador {
public:
    std::vector<std::vector<Componente>> categorias;
    std::vector<ReglaDependencia> dependencias;
    double presupuesto = 0.0;
    int nivelRequerido = -1; // -1 libre, 0 economico, 1 premium
    std::uint64_t nodos = 0;

    bool parcialValida(const std::vector<const Componente*>& sel) const {
        double costo = 0.0;
        for (const auto* c : sel) {
            costo += c->precio;
            if (costo > presupuesto) return false;
            if (c->premium != sel.front()->premium) return false;
            if (nivelRequerido >= 0 && c->premium != (nivelRequerido == 1)) return false;
        }
        for (const auto& r : dependencias) {
            const Componente* a = nullptr; const Componente* b = nullptr;
            for (const auto* c : sel) {
                if (c->categoria == r.categoriaA) a = c;
                if (c->categoria == r.categoriaB) b = c;
            }
            if (a && b && a->indice == r.opcionA && b->indice != r.opcionB) return false;
        }
        return true;
    }

private:
    void resolver(std::size_t nivel, std::vector<const Componente*>& act, bool soloPrimera,
        std::vector<std::vector<const Componente*>>& salida, bool poda) {
        if (nivel == categorias.size()) {
            if (poda || parcialValida(act)) salida.push_back(act);
            return;
        }
        for (const auto& op : categorias[nivel]) {
            act.push_back(&op);
            ++nodos;
            if (!poda || parcialValida(act)) {
                resolver(nivel + 1, act, soloPrimera, salida, poda);
                if (soloPrimera && !salida.empty()) { act.pop_back(); return; }
            }
            act.pop_back();
        }
    }

public:
    std::vector<std::vector<const Componente*>> generar(bool soloPrimera = false, bool poda = true) {
        nodos = 0;
        std::vector<std::vector<const Componente*>> salida;
        std::vector<const Componente*> act;
        resolver(0, act, soloPrimera, salida, poda);
        return salida;
    }

    // Devuelve la configuracion valida de menor costo (o vacia).
    std::vector<const Componente*> mejor(bool poda = true) {
        auto todas = generar(false, poda);
        std::vector<const Componente*> best; double bc = 1e18;
        for (auto& s : todas) {
            double c = 0; for (auto* x : s) c += x->precio;
            if (c < bc) { bc = c; best = s; }
        }
        return best;
    }
    static double costo(const std::vector<const Componente*>& s) { double c = 0; for (auto* x : s) c += x->precio; return c; }
};

// Escenario de demostracion: nCategorias x nOpciones; tres dependencias.
inline Configurador crearConfigurador(int nCategorias, int nOpciones, double presupuesto, unsigned semilla = 7) {
    Configurador c;
    c.presupuesto = presupuesto;
    c.categorias.resize(nCategorias);
    std::mt19937 rng(semilla);
    for (int cat = 0; cat < nCategorias; ++cat)
        for (int op = 0; op < nOpciones; ++op) {
            bool premium = (rng() % 2) == 0;
            double precio = premium ? 150.0 + (rng() % 151) : 50.0 + (rng() % 101);
            c.categorias[cat].push_back({ cat, op, precio, premium, cat * 100 + op });
        }
    if (nCategorias >= 2 && nOpciones > 3) c.dependencias.push_back({ 0, 3, 1, 1 });
    if (nCategorias >= 3 && nOpciones > 5) c.dependencias.push_back({ 1, 5, 2, 2 });
    if (nCategorias >= 4 && nOpciones > 7) c.dependencias.push_back({ 2, 7, 3, 0 });
    return c;
}
