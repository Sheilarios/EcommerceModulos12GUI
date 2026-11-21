#pragma once
// MODULO 4: Programacion dinamica y optimizacion de costos.
// Knapsack 0/1 (con reconstruccion) vs Greedy por ratio, Bitmask DP de turnos, KMP y Trie.
#include <algorithm>
#include <array>
#include <climits>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

// ---------------- 9.1 Knapsack empresarial ----------------
struct Item { int peso; int valor; };
struct ResultadoMochila { long long valor = 0; std::vector<int> elegidos; };

inline ResultadoMochila mochilaDP(const std::vector<Item>& it, int W) {
    int n = static_cast<int>(it.size());
    std::vector<std::vector<long long>> dp(n + 1, std::vector<long long>(W + 1, 0));
    for (int i = 1; i <= n; ++i)
        for (int w = 0; w <= W; ++w) {
            dp[i][w] = dp[i - 1][w];
            if (it[i - 1].peso <= w) dp[i][w] = std::max(dp[i][w], dp[i - 1][w - it[i - 1].peso] + it[i - 1].valor);
        }
    ResultadoMochila r; r.valor = dp[n][W];
    for (int i = n, w = W; i > 0; --i) if (dp[i][w] != dp[i - 1][w]) { r.elegidos.push_back(i - 1); w -= it[i - 1].peso; }
    std::reverse(r.elegidos.begin(), r.elegidos.end());
    return r;
}
inline long long mochilaDP1D(const std::vector<Item>& it, int W) { // memoria O(W) para volumen
    std::vector<long long> dp(W + 1, 0);
    for (auto& x : it) for (int w = W; w >= x.peso; --w) dp[w] = std::max(dp[w], dp[w - x.peso] + x.valor);
    return dp[W];
}
inline ResultadoMochila mochilaGreedy(const std::vector<Item>& it, int W) {
    std::vector<int> idx(it.size()); std::iota(idx.begin(), idx.end(), 0);
    std::stable_sort(idx.begin(), idx.end(), [&](int a, int b) {
        return static_cast<double>(it[a].valor) / it[a].peso > static_cast<double>(it[b].valor) / it[b].peso; });
    ResultadoMochila r; int usado = 0;
    for (int i : idx) if (usado + it[i].peso <= W) { usado += it[i].peso; r.valor += it[i].valor; r.elegidos.push_back(i); }
    return r;
}

// ---------------- 9.2 Bitmask DP en turnos ----------------
inline int popcount32(unsigned x) { int c = 0; while (x) { x &= x - 1; ++c; } return c; }
// costo[i][j]: costo de que el trabajador i cubra el turno j. O(2^n * n), n <= 20.
inline long long turnosBitmask(const std::vector<std::vector<int>>& c, std::vector<int>* asign = nullptr) {
    int n = static_cast<int>(c.size());
    std::vector<long long> dp(1u << n, LLONG_MAX / 4);
    std::vector<int> elec(1u << n, -1);
    dp[0] = 0;
    for (unsigned m = 0; m < (1u << n); ++m) {
        if (dp[m] >= LLONG_MAX / 4) continue;
        int i = popcount32(m);
        if (i >= n) continue;
        for (int j = 0; j < n; ++j) if (!(m & (1u << j))) {
            unsigned nm = m | (1u << j);
            if (dp[m] + c[i][j] < dp[nm]) { dp[nm] = dp[m] + c[i][j]; elec[nm] = j; }
        }
    }
    unsigned full = (1u << n) - 1;
    if (asign) {
        asign->assign(n, -1); unsigned m = full;
        for (int i = n - 1; i >= 0; --i) { int j = elec[m]; (*asign)[i] = j; m ^= (1u << j); }
    }
    return dp[full];
}
inline long long turnosFuerzaBruta(const std::vector<std::vector<int>>& c) {
    int n = static_cast<int>(c.size()); std::vector<int> p(n); std::iota(p.begin(), p.end(), 0);
    long long best = LLONG_MAX;
    do { long long s = 0; for (int i = 0; i < n; ++i) s += c[i][p[i]]; best = std::min(best, s); } while (std::next_permutation(p.begin(), p.end()));
    return best;
}

// ---------------- 9.3 KMP y Trie ----------------
inline std::vector<int> funcionPrefijo(const std::string& p) {
    std::vector<int> pi(p.size(), 0);
    for (std::size_t i = 1; i < p.size(); ++i) {
        int k = pi[i - 1];
        while (k > 0 && p[i] != p[k]) k = pi[k - 1];
        if (p[i] == p[k]) ++k;
        pi[i] = k;
    }
    return pi;
}
inline std::vector<int> kmpBuscar(const std::string& t, const std::string& p) {
    std::vector<int> r; if (p.empty()) return r;
    auto pi = funcionPrefijo(p); int k = 0;
    for (int i = 0; i < static_cast<int>(t.size()); ++i) {
        while (k > 0 && t[i] != p[k]) k = pi[k - 1];
        if (t[i] == p[k]) ++k;
        if (k == static_cast<int>(p.size())) { r.push_back(i - k + 1); k = pi[k - 1]; }
    }
    return r;
}
inline std::vector<int> busquedaIngenua(const std::string& t, const std::string& p) {
    std::vector<int> r;
    if (p.size() > t.size()) return r;
    for (std::size_t i = 0; i + p.size() <= t.size(); ++i) {
        std::size_t j = 0; while (j < p.size() && t[i + j] == p[j]) ++j;
        if (j == p.size()) r.push_back(static_cast<int>(i));
    }
    return r;
}

class Trie { // alfabeto: a-z, digitos y espacio (38 simbolos)
    static constexpr int A = 38;
    struct Nodo { std::array<int, A> hijo; bool fin = false; int cuenta = 0; Nodo() { hijo.fill(-1); } };
    std::vector<Nodo> nodos_{ Nodo() };
    static int idx(char ch) {
        if (ch >= 'a' && ch <= 'z') return ch - 'a';
        if (ch >= 'A' && ch <= 'Z') return ch - 'A';
        if (ch >= '0' && ch <= '9') return 26 + (ch - '0');
        return 36; // espacio y otros
    }
    static char sym(int i) { return i < 26 ? static_cast<char>('a' + i) : i < 36 ? static_cast<char>('0' + i - 26) : ' '; }
    void recolectar(int u, std::string& act, std::vector<std::string>& out, std::size_t lim) const {
        if (out.size() >= lim) return;
        if (nodos_[u].fin) out.push_back(act);
        for (int i = 0; i < A && out.size() < lim; ++i) if (nodos_[u].hijo[i] >= 0) {
            act.push_back(sym(i)); recolectar(nodos_[u].hijo[i], act, out, lim); act.pop_back(); }
    }
public:
    void insertar(const std::string& s) {
        int u = 0;
        for (char ch : s) {
            int c = idx(ch);
            if (nodos_[u].hijo[c] < 0) { nodos_[u].hijo[c] = static_cast<int>(nodos_.size()); nodos_.emplace_back(); }
            u = nodos_[u].hijo[c]; ++nodos_[u].cuenta;
        }
        nodos_[u].fin = true;
    }
    int contarPrefijo(const std::string& p) const {
        int u = 0; for (char ch : p) { u = nodos_[u].hijo[idx(ch)]; if (u < 0) return 0; } return nodos_[u].cuenta;
    }
    std::vector<std::string> sugerir(const std::string& p, std::size_t lim = 10) const {
        std::vector<std::string> out; int u = 0; std::string act;
        for (char ch : p) { u = nodos_[u].hijo[idx(ch)]; if (u < 0) return out; act.push_back(sym(idx(ch))); }
        recolectar(u, act, out, lim);
        return out;
    }
    std::size_t nodos() const { return nodos_.size(); }
};
