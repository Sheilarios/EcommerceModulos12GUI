#pragma once
// MODULO 3: Grafos y redes de transporte/logistica.
// BFS, DFS iterativo, deteccion de ciclos, orden topologico (Kahn), Dijkstra, Bellman-Ford,
// Kruskal (con DSU), Prim, Edmonds-Karp y Dinic.
#include <algorithm>
#include <climits>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>
#include <vector>
#include "modulo2_estructuras.hpp"

using ListaAdy = std::vector<std::vector<int>>;
constexpr long long INF_LL = std::numeric_limits<long long>::max() / 4;

// ---------------- 8.1 DFS/BFS en microservicios ----------------
inline std::vector<int> bfs(const ListaAdy& g, int s) {
    std::vector<int> d(g.size(), -1);
    std::queue<int> q; d[s] = 0; q.push(s);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : g[u]) if (d[v] < 0) { d[v] = d[u] + 1; q.push(v); }
    }
    return d;
}
inline int dfsAlcanzables(const ListaAdy& g, int s) { // pila explicita
    std::vector<char> vis(g.size(), 0);
    std::vector<int> pila{ s }; vis[s] = 1; int c = 0;
    while (!pila.empty()) {
        int u = pila.back(); pila.pop_back(); ++c;
        for (int v : g[u]) if (!vis[v]) { vis[v] = 1; pila.push_back(v); }
    }
    return c;
}
inline bool tieneCiclo(const ListaAdy& g) { // DFS iterativo con colores
    int n = static_cast<int>(g.size());
    std::vector<char> color(n, 0);
    std::vector<std::pair<int, std::size_t>> pila;
    for (int s = 0; s < n; ++s) {
        if (color[s]) continue;
        pila.push_back({ s, 0 }); color[s] = 1;
        while (!pila.empty()) {
            auto& top = pila.back(); int u = top.first;
            if (top.second < g[u].size()) {
                int v = g[u][top.second++];
                if (color[v] == 1) return true;
                if (color[v] == 0) { color[v] = 1; pila.push_back({ v, 0 }); }
            } else { color[u] = 2; pila.pop_back(); }
        }
    }
    return false;
}
inline std::vector<int> ordenTopologico(const ListaAdy& g) { // Kahn
    int n = static_cast<int>(g.size());
    std::vector<int> indeg(n, 0), orden;
    for (auto& a : g) for (int v : a) ++indeg[v];
    std::queue<int> q; for (int i = 0; i < n; ++i) if (!indeg[i]) q.push(i);
    while (!q.empty()) { int u = q.front(); q.pop(); orden.push_back(u); for (int v : g[u]) if (--indeg[v] == 0) q.push(v); }
    return orden; // si orden.size() < n hay ciclo
}

// ---------------- 8.2 Dijkstra y Bellman-Ford ----------------
struct AristaP { int a, b; long long w; };
using GrafoP = std::vector<std::vector<std::pair<int, long long>>>;

inline std::vector<long long> dijkstra(const GrafoP& g, int s, std::vector<int>* previo = nullptr) {
    std::vector<long long> d(g.size(), INF_LL);
    if (previo) previo->assign(g.size(), -1);
    using P = std::pair<long long, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    d[s] = 0; pq.push({ 0, s });
    while (!pq.empty()) {
        auto [du, u] = pq.top(); pq.pop();
        if (du != d[u]) continue;
        for (auto [v, w] : g[u]) if (du + w < d[v]) { d[v] = du + w; if (previo) (*previo)[v] = u; pq.push({ d[v], v }); }
    }
    return d;
}
// Devuelve false si hay ciclo negativo alcanzable.
inline bool bellmanFord(int n, const std::vector<AristaP>& e, int s, std::vector<long long>& d) {
    d.assign(n, INF_LL); d[s] = 0;
    for (int i = 0; i < n - 1; ++i) {
        bool cambio = false;
        for (auto& x : e) if (d[x.a] != INF_LL && d[x.a] + x.w < d[x.b]) { d[x.b] = d[x.a] + x.w; cambio = true; }
        if (!cambio) return true;
    }
    for (auto& x : e) if (d[x.a] != INF_LL && d[x.a] + x.w < d[x.b]) return false;
    return true;
}

// ---------------- 8.3 MST: Kruskal y Prim ----------------
inline long long kruskal(int n, std::vector<AristaP> e, std::vector<AristaP>* arbol = nullptr) {
    std::sort(e.begin(), e.end(), [](const AristaP& x, const AristaP& y) { return x.w < y.w; });
    DSU dsu(n); long long total = 0;
    for (auto& x : e) if (dsu.unir(x.a, x.b)) { total += x.w; if (arbol) arbol->push_back(x); }
    return total;
}
inline long long prim(int n, const std::vector<AristaP>& e) {
    GrafoP g(n);
    for (auto& x : e) { g[x.a].push_back({ x.b, x.w }); g[x.b].push_back({ x.a, x.w }); }
    std::vector<char> en(n, 0);
    using P = std::pair<long long, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    long long total = 0; pq.push({ 0, 0 });
    while (!pq.empty()) {
        auto [w, u] = pq.top(); pq.pop();
        if (en[u]) continue;
        en[u] = 1; total += w;
        for (auto [v, c] : g[u]) if (!en[v]) pq.push({ c, v });
    }
    return total;
}

// ---------------- 8.4 Flujo maximo ----------------
class RedFlujo {
    struct E { int to; long long cap; };
    int n_;
    std::vector<E> e_;
    std::vector<std::vector<int>> g_;
    std::vector<int> nivel_, it_;
    bool bfsNiveles(int s, int t) {
        nivel_.assign(n_, -1); std::queue<int> q; nivel_[s] = 0; q.push(s);
        while (!q.empty()) { int u = q.front(); q.pop();
            for (int id : g_[u]) if (e_[id].cap > 0 && nivel_[e_[id].to] < 0) { nivel_[e_[id].to] = nivel_[u] + 1; q.push(e_[id].to); } }
        return nivel_[t] >= 0;
    }
    long long dfsBloqueo(int u, int t, long long f) {
        if (u == t) return f;
        for (int& i = it_[u]; i < static_cast<int>(g_[u].size()); ++i) {
            int id = g_[u][i]; int v = e_[id].to;
            if (e_[id].cap > 0 && nivel_[v] == nivel_[u] + 1) {
                long long d = dfsBloqueo(v, t, std::min(f, e_[id].cap));
                if (d > 0) { e_[id].cap -= d; e_[id ^ 1].cap += d; return d; }
            }
        }
        return 0;
    }
public:
    explicit RedFlujo(int n) : n_(n), g_(n) {}
    void agregar(int a, int b, long long c) {
        g_[a].push_back(static_cast<int>(e_.size())); e_.push_back({ b, c });
        g_[b].push_back(static_cast<int>(e_.size())); e_.push_back({ a, 0 });
    }
    long long dinic(int s, int t) {
        long long f = 0;
        while (bfsNiveles(s, t)) { it_.assign(n_, 0); while (long long x = dfsBloqueo(s, t, INF_LL)) f += x; }
        return f;
    }
    long long edmondsKarp(int s, int t) {
        long long f = 0;
        while (true) {
            std::vector<int> pe(n_, -1); std::queue<int> q; q.push(s);
            std::vector<char> vis(n_, 0); vis[s] = 1;
            while (!q.empty() && !vis[t]) { int u = q.front(); q.pop();
                for (int id : g_[u]) if (e_[id].cap > 0 && !vis[e_[id].to]) { vis[e_[id].to] = 1; pe[e_[id].to] = id; q.push(e_[id].to); } }
            if (!vis[t]) break;
            long long b = INF_LL;
            for (int v = t; v != s; v = e_[pe[v] ^ 1].to) b = std::min(b, e_[pe[v]].cap);
            for (int v = t; v != s; v = e_[pe[v] ^ 1].to) { e_[pe[v]].cap -= b; e_[pe[v] ^ 1].cap += b; }
            f += b;
        }
        return f;
    }
    // Lado del origen en el corte minimo (tras calcular el flujo).
    std::vector<int> ladoOrigen(int s) {
        std::vector<int> r; std::vector<char> vis(n_, 0); std::queue<int> q; q.push(s); vis[s] = 1;
        while (!q.empty()) { int u = q.front(); q.pop(); r.push_back(u);
            for (int id : g_[u]) if (e_[id].cap > 0 && !vis[e_[id].to]) { vis[e_[id].to] = 1; q.push(e_[id].to); } }
        return r;
    }
};
