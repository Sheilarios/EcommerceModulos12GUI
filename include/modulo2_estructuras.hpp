#pragma once
// MODULO 2: Estructuras de datos para alta disponibilidad.
// map/set (stock y segmento Premium), priority_queue (cola de solicitudes),
// DSU (regiones de centros), Segment Tree (min/suma por rango),
// Fenwick Tree (pedidos por hora), Segment Tree con Lazy Propagation (descuento masivo).
#include <algorithm>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <vector>
#include "modulo1_greedy.hpp"

// ---------------- 7.1 Maps, Sets y Priority Queues ----------------
class Inventario {
    std::map<int, int> stock_;      // idProducto -> unidades
    std::set<int> premium_;         // pertenencia al segmento Premium
public:
    void registrar(int id, int unidades, bool premium) { stock_[id] += unidades; if (premium) premium_.insert(id); }
    bool reservar(int id, int unidades) {
        auto it = stock_.find(id);
        if (it == stock_.end() || it->second < unidades) return false;
        it->second -= unidades;
        return true;
    }
    int stock(int id) const { auto it = stock_.find(id); return it == stock_.end() ? 0 : it->second; }
    bool esPremium(int id) const { return premium_.count(id) > 0; }
    std::size_t productos() const { return stock_.size(); }
};

struct ComparadorPrioridad { // RN-A05 + RN-A01 para la cola
    bool operator()(const Solicitud& a, const Solicitud& b) const {
        if (a.urgencia != b.urgencia) return a.urgencia < b.urgencia;
        if (a.fechaEntrega != b.fechaEntrega) return a.fechaEntrega > b.fechaEntrega;
        return a.ratio() < b.ratio();
    }
};
using ColaSolicitudes = std::priority_queue<Solicitud, std::vector<Solicitud>, ComparadorPrioridad>;

// Motor Greedy alimentado por la cola de prioridad (integracion Modulo 1 + Modulo 2).
inline ResultadoGreedy asignarDesdeCola(ColaSolicitudes cola, std::vector<Recurso>& recursos) {
    ResultadoGreedy r;
    while (!cola.empty()) {
        Solicitud s = cola.top(); cola.pop();
        bool hecho = false;
        for (auto& rec : recursos)
            if (rec.usado + s.costo <= rec.capacidad) { rec.usado += s.costo; r.asignaciones.push_back({ s.id, rec.id }); r.valorTotal += s.ganancia; hecho = true; break; }
        if (!hecho) r.noAsignadas.push_back(s.id);
    }
    return r;
}

// ---------------- 7.2 Disjoint Set Union ----------------
class DSU {
    std::vector<int> padre_, tam_;
    int componentes_;
public:
    explicit DSU(int n = 0) : padre_(n), tam_(n, 1), componentes_(n) { std::iota(padre_.begin(), padre_.end(), 0); }
    int encontrar(int x) {
        int r = x;
        while (padre_[r] != r) r = padre_[r];
        while (padre_[x] != r) { int s = padre_[x]; padre_[x] = r; x = s; } // compresion iterativa
        return r;
    }
    bool unir(int a, int b) {
        a = encontrar(a); b = encontrar(b);
        if (a == b) return false;
        if (tam_[a] < tam_[b]) std::swap(a, b);
        padre_[b] = a; tam_[a] += tam_[b]; --componentes_;
        return true;
    }
    bool conectados(int a, int b) { return encontrar(a) == encontrar(b); }
    int componentes() const { return componentes_; }
    int tamano(int x) { return tam_[encontrar(x)]; }
};

// ---------------- 7.3 Segment Tree (minimo y suma) ----------------
class SegmentTree {
    int n_;
    std::vector<double> mn_, sm_;
    void construir(const std::vector<double>& a, int nd, int l, int r) {
        if (l == r) { mn_[nd] = sm_[nd] = a[l]; return; }
        int m = (l + r) / 2;
        construir(a, 2 * nd, l, m); construir(a, 2 * nd + 1, m + 1, r);
        mn_[nd] = std::min(mn_[2 * nd], mn_[2 * nd + 1]); sm_[nd] = sm_[2 * nd] + sm_[2 * nd + 1];
    }
    void act(int nd, int l, int r, int p, double v) {
        if (l == r) { mn_[nd] = sm_[nd] = v; return; }
        int m = (l + r) / 2;
        if (p <= m) act(2 * nd, l, m, p, v); else act(2 * nd + 1, m + 1, r, p, v);
        mn_[nd] = std::min(mn_[2 * nd], mn_[2 * nd + 1]); sm_[nd] = sm_[2 * nd] + sm_[2 * nd + 1];
    }
    double qmin(int nd, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) return std::numeric_limits<double>::infinity();
        if (ql <= l && r <= qr) return mn_[nd];
        int m = (l + r) / 2;
        return std::min(qmin(2 * nd, l, m, ql, qr), qmin(2 * nd + 1, m + 1, r, ql, qr));
    }
    double qsum(int nd, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) return 0.0;
        if (ql <= l && r <= qr) return sm_[nd];
        int m = (l + r) / 2;
        return qsum(2 * nd, l, m, ql, qr) + qsum(2 * nd + 1, m + 1, r, ql, qr);
    }
public:
    explicit SegmentTree(const std::vector<double>& a) : n_(static_cast<int>(a.size())), mn_(4 * a.size() + 4), sm_(4 * a.size() + 4) {
        if (n_) construir(a, 1, 0, n_ - 1);
    }
    void actualizar(int p, double v) { act(1, 0, n_ - 1, p, v); }
    double minimo(int l, int r) const { return qmin(1, 0, n_ - 1, l, r); }
    double suma(int l, int r) const { return qsum(1, 0, n_ - 1, l, r); }
};

// ---------------- 7.4 Fenwick Tree ----------------
class Fenwick {
    std::vector<long long> t_;
public:
    explicit Fenwick(int n) : t_(n + 1, 0) {}
    void sumar(int i, long long v) { for (++i; i < static_cast<int>(t_.size()); i += i & -i) t_[i] += v; }
    long long prefijo(int i) const { long long s = 0; for (++i; i > 0; i -= i & -i) s += t_[i]; return s; }
    long long rango(int l, int r) const { return prefijo(r) - (l ? prefijo(l - 1) : 0); }
};

// ---------------- 7.4 Segment Tree con Lazy Propagation ----------------
class LazySegmentTree { // suma de rango + actualizacion de rango (descuento)
    int n_;
    std::vector<double> sm_, lz_;
    void aplicar(int nd, int l, int r, double v) { sm_[nd] += v * (r - l + 1); lz_[nd] += v; }
    void bajar(int nd, int l, int r) {
        if (lz_[nd] != 0) { int m = (l + r) / 2; aplicar(2 * nd, l, m, lz_[nd]); aplicar(2 * nd + 1, m + 1, r, lz_[nd]); lz_[nd] = 0; }
    }
    void construir(const std::vector<double>& a, int nd, int l, int r) {
        if (l == r) { sm_[nd] = a[l]; return; }
        int m = (l + r) / 2; construir(a, 2 * nd, l, m); construir(a, 2 * nd + 1, m + 1, r);
        sm_[nd] = sm_[2 * nd] + sm_[2 * nd + 1];
    }
    void upd(int nd, int l, int r, int ql, int qr, double v) {
        if (qr < l || r < ql) return;
        if (ql <= l && r <= qr) { aplicar(nd, l, r, v); return; }
        bajar(nd, l, r); int m = (l + r) / 2;
        upd(2 * nd, l, m, ql, qr, v); upd(2 * nd + 1, m + 1, r, ql, qr, v);
        sm_[nd] = sm_[2 * nd] + sm_[2 * nd + 1];
    }
    double qry(int nd, int l, int r, int ql, int qr) {
        if (qr < l || r < ql) return 0.0;
        if (ql <= l && r <= qr) return sm_[nd];
        bajar(nd, l, r); int m = (l + r) / 2;
        return qry(2 * nd, l, m, ql, qr) + qry(2 * nd + 1, m + 1, r, ql, qr);
    }
public:
    explicit LazySegmentTree(const std::vector<double>& a) : n_(static_cast<int>(a.size())), sm_(4 * a.size() + 4), lz_(4 * a.size() + 4) {
        if (n_) construir(a, 1, 0, n_ - 1);
    }
    void sumarRango(int l, int r, double v) { upd(1, 0, n_ - 1, l, r, v); }
    double suma(int l, int r) { return qry(1, 0, n_ - 1, l, r); }
};
