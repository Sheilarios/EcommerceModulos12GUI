#pragma once
// Verificaciones con ejemplos de resultado conocido (secciones 6 a 9 del informe)
// y pruebas de volumen. Cada funcion devuelve una Tabla (3 columnas) usada por la GUI y la consola.
#include <climits>
#include <random>
#include <string>
#include "comun.hpp"
#include "modulo1_catalogo.hpp"
#include "modulo1_configurador.hpp"
#include "modulo1_greedy.hpp"
#include "modulo2_estructuras.hpp"
#include "modulo3_grafos.hpp"
#include "modulo4_dp.hpp"

// ======================= MODULO 1 =======================
inline Tabla pruebaCatalogo(const Catalogo& cat) {
    Tabla t;
    if (cat.size() == 0) { t.push_back({ "Catalogo", "vacio: genere o importe primero", "-" }); return t; }
    const int Q = 10000, QL = 100;
    std::mt19937 rng(42); std::vector<int> ids(Q);
    for (auto& x : ids) x = cat.enPosicion(rng() % cat.size()).id;
    Cronometro c; volatile int enc = 0;
    for (int id : ids) if (cat.buscarID(id)) ++enc;
    double uB = c.us() / Q; c.reiniciar();
    for (int i = 0; i < QL; ++i) if (cat.buscarLineal(ids[i])) ++enc;
    double uL = c.us() / QL;
    t.push_back({ "Busqueda binaria O(log N)", fmt(uB, 3) + " us/consulta", std::to_string(Q) + " consultas" });
    t.push_back({ "Busqueda lineal O(N)", fmt(uL, 3) + " us/consulta", std::to_string(QL) + " consultas" });
    t.push_back({ "Aceleracion", fmt(uL / (uB > 0 ? uB : 1e-9), 0) + "x", "N = " + std::to_string(cat.size()) });
    t.push_back({ "IND-C01 meta < 2 us", uB < 2.0 ? "CUMPLE" : "NO CUMPLE", "-" });
    double mb = cat.memoriaBytes() / 1048576.0;
    t.push_back({ "IND-C04 memoria catalogo + indices", fmt(mb, 1) + " MB", std::string(mb <= 100 ? "CUMPLE (<= 100 MB)" : (cat.size() > 1000000 ? "N > 1M (meta definida para 1M)" : "NO CUMPLE")) + " | proceso " + fmt(memoriaActualMB(), 1) + " MB" });
    // Rango de precio: recorrido lineal (AS-IS) vs indice ordenado (TO-BE)
    c.reiniciar(); std::size_t k1 = cat.contarRangoLineal(100, 500); double mLin = c.ms();
    c.reiniciar(); std::size_t k2 = 0; for (int i = 0; i < 1000; ++i) k2 = cat.contarRangoIndice(100, 500); double usIdx = c.us() / 1000;
    t.push_back({ "Rango precio 100-500: lineal vs indice", fmt(mLin, 2) + " ms vs " + fmt(usIdx, 3) + " us", std::to_string(k2) + " productos " + ok(k1 == k2) });
    // Segment Tree sobre el catalogo: precio minimo entre posiciones (trabajo futuro 11.4 integrado)
    { std::vector<double> precios(cat.size()); for (std::size_t i = 0; i < cat.size(); ++i) precios[i] = cat.enPosicion(i).precio;
      SegmentTree st(precios); int N = static_cast<int>(cat.size()); bool iguales = true; volatile double z = 0;
      c.reiniciar(); for (int i = 0; i < 10000; ++i) { int l = rng() % N, r = rng() % N; if (l > r) std::swap(l, r); z = z + st.minimo(l, r); } double usSt = c.us() / 10000;
      c.reiniciar(); for (int i = 0; i < 20; ++i) { int l = rng() % N, r = rng() % N; if (l > r) std::swap(l, r);
          double m = 1e18; for (int k = l; k <= r; ++k) m = std::min(m, precios[k]); iguales &= (m == st.minimo(l, r)); z = z + m; } double usLn = c.us() / 20;
      t.push_back({ "Precio minimo entre IDs: Segment Tree", fmt(usSt, 3) + " us vs lineal " + fmt(usLn, 1) + " us", "resultados iguales " + ok(iguales) }); }
    return t;
}

inline Tabla pruebaConfigurador(double presupuesto) {
    Tabla t;
    Configurador cf = crearConfigurador(4, 10, presupuesto);
    Cronometro c; auto best = cf.mejor(true); double usBT = c.us(); auto nBT = cf.nodos;
    c.reiniciar(); auto bestFB = cf.mejor(false); double usFB = c.us(); auto nFB = cf.nodos;
    if (best.empty()) t.push_back({ "Resultado", "Sin configuracion valida", "-" });
    for (auto* x : best)
        t.push_back({ "Categoria " + std::to_string(x->categoria), "Opcion " + std::to_string(x->indice) + " | S/ " + fmt(x->precio),
            x->premium ? "Premium" : "Economico" });
    if (!best.empty()) t.push_back({ "TOTAL", "S/ " + fmt(Configurador::costo(best)), "<= S/ " + fmt(presupuesto) });
    t.push_back({ "Backtracking con poda", fmt(usBT, 0) + " us", std::to_string(nBT) + " nodos" });
    t.push_back({ "Fuerza bruta sin poda", fmt(usFB, 0) + " us", std::to_string(nFB) + " nodos" });
    t.push_back({ "Reduccion de nodos", fmt(nFB ? 100.0 * (double)(nFB - nBT) / nFB : 0) + " %", "-" });
    t.push_back({ "Mismo optimo (BT = FB)", ok(Configurador::costo(best) == Configurador::costo(bestFB)), "-" });
    return t;
}

inline Tabla pruebaGreedy() {
    Tabla t;
    for (auto orden : { OrdenGreedy::Prioridad, OrdenGreedy::Ratio }) {
        auto s = generarSolicitudes(10000, 11); auto r = generarRecursos(300, 30);
        Cronometro c; auto res = asignarFirstFit(s, r, orden); double ms = c.ms();
        t.push_back({ std::string("10 000 sol / 300 rec, orden ") + (orden == OrdenGreedy::Prioridad ? "prioridad" : "ratio"),
            std::to_string(res.asignaciones.size()) + " asignadas, valor " + fmt(res.valorTotal, 1),
            fmt(ms, 2) + " ms (" + fmt(ms * 1000 / 10000, 3) + " us/sol)" });
    }
    { auto s = generarSolicitudes(1000000, 21); auto r = generarRecursos(500, 30);
      Cronometro c; auto res = asignarFirstFit(s, r, OrdenGreedy::Prioridad); double ms = c.ms();
      t.push_back({ "1 000 000 sol / 500 rec, orden prioridad", std::to_string(res.asignaciones.size()) + " asignadas", fmt(ms, 1) + " ms (" + fmt(ms * 1000 / 1000000, 3) + " us/sol)" }); }
    // Verificacion RN-A02: ningun recurso supera su capacidad
    { auto s = generarSolicitudes(5000, 3); auto r = generarRecursos(50, 30); asignarFirstFit(s, r, OrdenGreedy::Prioridad);
      bool okc = true; for (auto& x : r) okc &= x.usado <= x.capacidad;
      t.push_back({ "RN-A02 capacidad respetada", ok(okc), "50 recursos" }); }
    // Optimalidad frente al optimo exacto (IND-A02)
    double sumP = 0, sumR = 0, minP = 1, minR = 1, sumB = 0, minB = 1; const int I = 300;
    for (int k = 0; k < I; ++k) {
        auto s = generarSolicitudes(10, 1000 + k, 5, 15);
        std::mt19937 rng(k); std::vector<int> cap(3); for (auto& x : cap) x = 20 + rng() % 11;
        auto mk = [&]() { std::vector<Recurso> r; for (int i = 0; i < 3; ++i) r.push_back({ i, cap[i], 0 }); return r; };
        double opt = optimoExacto(s, cap);
        auto r1 = mk(); double p = asignarFirstFit(s, r1, OrdenGreedy::Prioridad).valorTotal / opt;
        auto r2 = mk(); double q = asignarFirstFit(s, r2, OrdenGreedy::Ratio).valorTotal / opt;
        auto r3 = mk(); double b = asignarBestFit(s, r3, OrdenGreedy::Ratio).valorTotal / opt; sumB += b; minB = std::min(minB, b);
        sumP += p; sumR += q; minP = std::min(minP, p); minR = std::min(minR, q);
    }
    t.push_back({ "Optimalidad prioridad + First Fit", fmt(100 * sumP / I, 1) + " % media", "peor " + fmt(100 * minP, 1) + " %" });
    t.push_back({ "Optimalidad ratio + First Fit", fmt(100 * sumR / I, 1) + " % media", "peor " + fmt(100 * minR, 1) + " % (meta >= 90 %)" });
    t.push_back({ "Optimalidad ratio + Best Fit (comparacion)", fmt(100 * sumB / I, 1) + " % media", "peor " + fmt(100 * minB, 1) + " % (cambia RN-A03)" });
    t.push_back({ "IND-A02 (ratio + First Fit >= 90 %)", sumR / I >= 0.90 ? "CUMPLE" : "NO CUMPLE", "costo de la urgencia: " + fmt(100 * (sumR - sumP) / I, 1) + " puntos" });
    return t;
}

// ======================= MODULO 2 =======================
inline Tabla pruebaModulo2() {
    Tabla t;
    // map / set
    { Inventario inv; inv.registrar(100, 5, true); inv.registrar(101, 2, false);
      bool a = inv.reservar(100, 3), b = !inv.reservar(101, 3);
      t.push_back({ "map/set: stock y segmento Premium", ok(a && b && inv.stock(100) == 2 && inv.esPremium(100) && !inv.esPremium(101)), "reserva 3 de 5; rechaza 3 de 2" }); }
    { Inventario inv; std::mt19937 rng(1); for (int i = 0; i < 200000; ++i) inv.registrar(i, 10, i % 2);
      Cronometro c; volatile int s = 0; for (int i = 0; i < 1000000; ++i) s += inv.stock(rng() % 200000);
      double mMap = c.ms();
      std::vector<std::pair<int, int>> vec; for (int i = 0; i < 200000; ++i) vec.push_back({ i, 10 }); // vector ordenado por ID
      std::mt19937 r2(1); c.reiniciar();
      for (int i = 0; i < 1000000; ++i) { int k = r2() % 200000; auto it = std::lower_bound(vec.begin(), vec.end(), std::make_pair(k, INT_MIN)); s += it->second; }
      double mVec = c.ms();
      t.push_back({ "1M consultas: std::map vs vector ordenado", fmt(mMap, 1) + " ms vs " + fmt(mVec, 1) + " ms", "vector " + fmt(mMap / (mVec > 0 ? mVec : 1e-9), 1) + "x mas rapido" }); }
    // priority_queue
    { ColaSolicitudes q; q.push({ 1, 2, 5, 3, 10 }); q.push({ 2, 5, 9, 3, 10 }); q.push({ 3, 5, 2, 3, 10 }); q.push({ 4, 2, 5, 2, 20 });
      std::vector<int> o; while (!q.empty()) { o.push_back(q.top().id); q.pop(); }
      t.push_back({ "priority_queue (RN-A05/RN-A01)", "orden " + unir(o), ok(o == std::vector<int>{3, 2, 4, 1}) }); }
    // DSU
    { DSU d(8); d.unir(0, 1); d.unir(1, 2); d.unir(3, 4); d.unir(5, 6);
      t.push_back({ "DSU regiones (8 centros)", std::to_string(d.componentes()) + " regiones; 0~2 " + (d.conectados(0, 2) ? "si" : "no") + ", 2~3 " + (d.conectados(2, 3) ? "si" : "no"),
          ok(d.componentes() == 4 && d.conectados(0, 2) && !d.conectados(2, 3)) }); }
    { DSU d(1000000); std::mt19937 rng(5); Cronometro c; for (int i = 0; i < 1000000; ++i) d.unir(rng() % 1000000, rng() % 1000000);
      t.push_back({ "DSU 1M uniones", fmt(c.ms(), 1) + " ms", std::to_string(d.componentes()) + " componentes" }); }
    // Segment tree
    { SegmentTree st({ 120, 80, 300, 45, 210, 95 });
      double a = st.minimo(0, 2), b = st.minimo(2, 5), s = st.suma(1, 3); st.actualizar(3, 500); double c2 = st.minimo(2, 5);
      t.push_back({ "Segment Tree [120,80,300,45,210,95]", "min(0,2)=" + fmt(a, 0) + " min(2,5)=" + fmt(b, 0) + " suma(1,3)=" + fmt(s, 0) + " tras act min(2,5)=" + fmt(c2, 0),
          ok(a == 80 && b == 45 && s == 425 && c2 == 95) }); }
    { const int N = 1000000; std::vector<double> v(N); std::mt19937 rng(3); for (auto& x : v) x = rng() % 100000;
      SegmentTree st(v); Cronometro c; volatile double z = 0;
      for (int i = 0; i < 100000; ++i) { int l = rng() % N, r = rng() % N; if (l > r) std::swap(l, r); z = z + st.minimo(l, r); }
      double us = c.us() / 100000; c.reiniciar();
      for (int i = 0; i < 20; ++i) { int l = rng() % N, r = rng() % N; if (l > r) std::swap(l, r); double m = 1e18; for (int k = l; k <= r; ++k) m = std::min(m, v[k]); z = z + m; }
      double ul = c.us() / 20;
      t.push_back({ "Segment Tree 1M: consulta minimo", fmt(us, 3) + " us/consulta", "lineal " + fmt(ul, 1) + " us/consulta" }); }
    // Fenwick
    { std::vector<int> ped = { 5, 3, 2, 4, 6, 8, 12, 15, 18, 20, 22, 21, 23, 25, 24, 26, 28, 35, 38, 36, 30, 25, 18, 10 };
      Fenwick f(24); long long tot = 0; for (int i = 0; i < 24; ++i) { f.sumar(i, ped[i]); tot += ped[i]; }
      long long a = 0; for (int i = 0; i <= 12; ++i) a += ped[i];
      t.push_back({ "Fenwick pedidos por hora", "h0-12=" + std::to_string(f.rango(0, 12)) + " h17-19=" + std::to_string(f.rango(17, 19)) + " dia=" + std::to_string(f.rango(0, 23)),
          ok(f.rango(0, 12) == a && f.rango(17, 19) == 109 && f.rango(0, 23) == tot) }); }
    // Lazy
    { LazySegmentTree lz(std::vector<double>(10, 100)); lz.sumarRango(2, 6, -15);
      double a = lz.suma(0, 9), b = lz.suma(3, 4);
      t.push_back({ "Lazy: descuento 15 en IDs 2-6", "suma=" + fmt(a, 0) + " suma(3,4)=" + fmt(b, 0), ok(a == 925 && b == 170) }); }
    { const int N = 1000000; LazySegmentTree lz(std::vector<double>(N, 100)); std::mt19937 rng(9); Cronometro c; volatile double z = 0;
      for (int i = 0; i < 100000; ++i) { int l = rng() % N, r = rng() % N; if (l > r) std::swap(l, r); lz.sumarRango(l, r, -1); z = z + lz.suma(l, r); }
      t.push_back({ "Lazy 1M: 100k act + 100k consultas", fmt(c.ms(), 1) + " ms", fmt(c.us() / 200000, 3) + " us/op" }); }
    return t;
}

// ======================= MODULO 3 =======================
inline Tabla pruebaModulo3() {
    Tabla t;
    { ListaAdy g(6); g[0] = { 1, 2 }; g[1] = { 3 }; g[2] = { 3 }; g[3] = { 4 }; g[4] = { 5 };
      auto d = bfs(g, 0); auto o = ordenTopologico(g);
      t.push_back({ "BFS microservicios desde 0", "dist " + unir(d), ok(d == std::vector<int>{0, 1, 1, 2, 3, 4}) });
      t.push_back({ "DFS ciclo / orden topologico", std::string(tieneCiclo(g) ? "ciclo" : "sin ciclo") + "; orden " + unir(o), ok(!tieneCiclo(g) && o.size() == 6) });
      g[5].push_back(1); auto o2 = ordenTopologico(g);
      t.push_back({ "Con 5->1: deteccion de bloqueo", std::string(tieneCiclo(g) ? "ciclo detectado" : "sin ciclo") + "; ordena " + std::to_string(o2.size()) + " de 6", ok(tieneCiclo(g) && o2.size() == 2) }); }
    { const int N = 1000000; ListaAdy g(N); std::mt19937 rng(1);
      for (int i = 0; i < 4 * N; ++i) { int a = rng() % N, b = rng() % N; if (a > b) std::swap(a, b); if (a != b) g[a].push_back(b); }
      Cronometro c; auto d = bfs(g, 0); double m1 = c.ms(); c.reiniciar(); int al = dfsAlcanzables(g, 0); double m2 = c.ms();
      c.reiniciar(); bool cy = tieneCiclo(g); double m3 = c.ms(); c.reiniciar(); auto o = ordenTopologico(g); double m4 = c.ms();
      t.push_back({ "Volumen 1M nodos / ~4M aristas (DAG)", "BFS " + fmt(m1, 1) + " ms, DFS " + fmt(m2, 1) + " ms", std::to_string(al) + " alcanzados" });
      t.push_back({ "Ciclos + topologico (volumen)", "ciclos " + fmt(m3, 1) + " ms, Kahn " + fmt(m4, 1) + " ms", std::string(cy ? "con ciclo" : "sin ciclo") + ", " + std::to_string(o.size()) + " ordenados" }); }
    { GrafoP g(6); auto add = [&](int a, int b, int w) { g[a].push_back({ b, w }); g[b].push_back({ a, w }); };
      add(0, 1, 7); add(0, 2, 9); add(0, 5, 14); add(1, 2, 10); add(1, 3, 15); add(2, 3, 11); add(2, 5, 2); add(3, 4, 6); add(4, 5, 9);
      auto d = dijkstra(g, 0);
      t.push_back({ "Dijkstra red clasica 6 nodos", "dist " + unir(d), ok(d == std::vector<long long>{0, 7, 9, 20, 20, 11}) }); }
    { std::vector<AristaP> e = { {0,1,4},{0,2,5},{1,2,-3},{2,3,2},{1,3,6} }; std::vector<long long> d;
      bool okb = bellmanFord(4, e, 0, d);
      t.push_back({ "Bellman-Ford con arista negativa", "dist " + unir(d), ok(okb && d == std::vector<long long>{0, 4, 1, 3}) });
      e.push_back({ 3, 1, -4 }); bool neg = !bellmanFord(4, e, 0, d);
      t.push_back({ "Bellman-Ford ciclo negativo 1->2->3->1", neg ? "detectado (peso -5)" : "no detectado", ok(neg) }); }
    { const int N = 5000; std::mt19937 rng(2); GrafoP g(N); std::vector<AristaP> e;
      for (int i = 0; i < 25000; ++i) { int a = rng() % N, b = rng() % N, w = 1 + rng() % 100; if (a == b) continue; g[a].push_back({ b, w }); g[b].push_back({ a, w }); e.push_back({ a, b, w }); e.push_back({ b, a, w }); }
      Cronometro c; auto d1 = dijkstra(g, 0); double m1 = c.ms(); c.reiniciar(); std::vector<long long> d2; bellmanFord(N, e, 0, d2); double m2 = c.ms();
      t.push_back({ "5 000 nodos: Dijkstra vs Bellman-Ford", fmt(m1, 2) + " ms vs " + fmt(m2, 2) + " ms", "mismas distancias: " + ok(d1 == d2) }); }
    { const int N = 100000; std::mt19937 rng(22); GrafoP g(N);
      for (int i = 1; i < N; ++i) { int a = rng() % i; long long w = 1 + rng() % 100; g[a].push_back({ i, w }); g[i].push_back({ a, w }); }
      for (int i = 0; i < 400000; ++i) { int a = rng() % N, b = rng() % N; long long w = 1 + rng() % 100; if (a != b) { g[a].push_back({ b, w }); g[b].push_back({ a, w }); } }
      Cronometro c; auto d = dijkstra(g, 0); double m = c.ms(); long long mx = 0; for (auto x : d) mx = std::max(mx, x);
      t.push_back({ "Dijkstra 100 000 nodos / 500 000 aristas", fmt(m, 1) + " ms", "distancia maxima " + std::to_string(mx) }); }
    { std::vector<AristaP> e = { {0,1,1},{1,2,2},{2,3,3},{3,4,4},{4,5,4},{0,2,5},{1,3,6},{2,5,7} };
      long long k = kruskal(6, e), p = prim(6, e);
      t.push_back({ "MST 6 centros / 8 enlaces", "Kruskal " + std::to_string(k) + ", Prim " + std::to_string(p), ok(k == 14 && p == 14) }); }
    { const int N = 100000; std::mt19937 rng(4); std::vector<AristaP> e;
      for (int i = 1; i < N; ++i) e.push_back({ static_cast<int>(rng() % i), i, 1 + static_cast<long long>(rng() % 1000) });
      while ((int)e.size() < 500000) { int a = rng() % N, b = rng() % N; if (a != b) e.push_back({ a, b, 1 + static_cast<long long>(rng() % 1000) }); }
      Cronometro c; long long k = kruskal(N, e); double m1 = c.ms(); c.reiniciar(); long long p = prim(N, e); double m2 = c.ms();
      t.push_back({ "MST 100 000 nodos / 500 000 aristas", "Kruskal " + fmt(m1, 1) + " ms, Prim " + fmt(m2, 1) + " ms", "peso " + std::to_string(k) + " " + ok(k == p) }); }
    { auto red = []() { RedFlujo f(6); f.agregar(0,1,16); f.agregar(0,2,13); f.agregar(1,2,10); f.agregar(2,1,4); f.agregar(1,3,12);
        f.agregar(3,2,9); f.agregar(2,4,14); f.agregar(4,3,7); f.agregar(3,5,20); f.agregar(4,5,4); return f; };
      auto a = red(); auto b = red(); long long d = a.dinic(0, 5), k = b.edmondsKarp(0, 5);
      t.push_back({ "Flujo maximo red clasica", "Dinic " + std::to_string(d) + ", Edmonds-Karp " + std::to_string(k), ok(d == 23 && k == 23) });
      t.push_back({ "Corte minimo (lado origen)", unir(a.ladoOrigen(0)), "cuello de botella" }); }
    { const int N = 2000; std::mt19937 rng(6); std::vector<std::tuple<int, int, int>> e;
      for (int i = 0; i < 20000; ++i) { int a = rng() % N, b = rng() % N; if (a != b) e.emplace_back(a, b, 1 + rng() % 20); }
      RedFlujo f1(N), f2(N); for (auto& [a, b, c] : e) { f1.agregar(a, b, c); f2.agregar(a, b, c); }
      Cronometro c; long long x = f1.dinic(0, N - 1); double m1 = c.ms(); c.reiniciar(); long long y = f2.edmondsKarp(0, N - 1); double m2 = c.ms();
      t.push_back({ "Flujo 2 000 nodos / ~20 000 aristas", "Dinic " + fmt(m1, 2) + " ms, EK " + fmt(m2, 2) + " ms", "flujo " + std::to_string(x) + " " + ok(x == y) }); }
    { const int N = 20000; std::mt19937 rng(23); RedFlujo f(N);
      for (int i = 0; i < 200000; ++i) { int a = rng() % N, b = rng() % N; if (a != b) f.agregar(a, b, 1 + rng() % 20); }
      Cronometro c; long long x = f.dinic(0, N - 1);
      t.push_back({ "Dinic 20 000 nodos / 200 000 aristas", fmt(c.ms(), 1) + " ms", "flujo " + std::to_string(x) }); }
    return t;
}

// ======================= MODULO 4 =======================
inline Tabla pruebaModulo4() {
    Tabla t;
    { std::vector<Item> it = { {1,1},{3,4},{4,5},{5,7} }; auto d = mochilaDP(it, 7); auto g = mochilaGreedy(it, 7);
      t.push_back({ "Mochila W=7 (DP vs Greedy)", "DP " + std::to_string(d.valor) + " items {" + unir(d.elegidos) + "}, Greedy " + std::to_string(g.valor),
          ok(d.valor == 9 && g.valor == 8) + " (" + fmt(100.0 * g.valor / d.valor, 1) + " %)" }); }
    { double s = 0, mn = 1; std::mt19937 rng(8);
      for (int k = 0; k < 300; ++k) { std::vector<Item> it; for (int i = 0; i < 10; ++i) it.push_back({ 5 + (int)(rng() % 11), 1 + (int)(rng() % 50) });
        double r = (double)mochilaGreedy(it, 50).valor / mochilaDP(it, 50).valor; s += r; mn = std::min(mn, r); }
      t.push_back({ "Optimalidad Greedy (300 inst. pequenas)", fmt(100 * s / 300, 1) + " % media", "peor " + fmt(100 * mn, 1) + " %" }); }
    { std::vector<Item> it; std::mt19937 rng(10); for (int i = 0; i < 1000; ++i) it.push_back({ 5 + (int)(rng() % 46), 1 + (int)(rng() % 500) });
      Cronometro c; long long d = mochilaDP1D(it, 10000); double m1 = c.ms(); c.reiniciar(); long long g = mochilaGreedy(it, 10000).valor; double m2 = c.ms();
      t.push_back({ "Mochila 1 000 items, W=10 000", "DP " + fmt(m1, 2) + " ms (" + std::to_string(d) + ")", "Greedy " + fmt(m2, 3) + " ms (" + fmt(100.0 * g / d, 2) + " %)" }); }
    { std::vector<std::vector<int>> c = { {9,2,7,8},{6,4,3,7},{5,8,1,8},{7,6,9,4} }; std::vector<int> a; long long v = turnosBitmask(c, &a);
      t.push_back({ "Turnos 4x4 Bitmask DP", "costo " + std::to_string(v) + ", turno por trabajador " + unir(a), ok(v == 13 && v == turnosFuerzaBruta(c)) }); }
    { std::mt19937 rng(12); std::vector<std::vector<int>> c(10, std::vector<int>(10)); for (auto& f : c) for (auto& x : f) x = 1 + rng() % 100;
      Cronometro cr; long long a = turnosBitmask(c); double m1 = cr.ms(); cr.reiniciar(); long long b = turnosFuerzaBruta(c); double m2 = cr.ms();
      t.push_back({ "Turnos n=10: DP vs fuerza bruta (10!)", fmt(m1, 2) + " ms vs " + fmt(m2, 1) + " ms", "mismo costo " + ok(a == b) }); }
    { std::vector<std::vector<int>> c(20, std::vector<int>(20)); std::mt19937 rng(13); for (auto& f : c) for (auto& x : f) x = 1 + rng() % 100;
      Cronometro cr; long long a = turnosBitmask(c);
      t.push_back({ "Turnos n=20 Bitmask DP (2^20 estados)", fmt(cr.ms(), 1) + " ms", "costo " + std::to_string(a) }); }
    { auto r = kmpBuscar("abababca ababab", "abab");
      t.push_back({ "KMP 'abab' en 'abababca ababab'", "posiciones " + unir(r), ok(r == std::vector<int>{0, 2, 9, 11}) }); }
    { std::string tx(1000000, 'a'), p = std::string(300, 'a') + "b";
      Cronometro c; auto a = kmpBuscar(tx, p); double m1 = c.ms(); c.reiniciar(); auto b = busquedaIngenua(tx, p); double m2 = c.ms();
      t.push_back({ "KMP peor caso (10^6 'a', patron 300a+b)", "KMP " + fmt(m1, 2) + " ms", "ingenua " + fmt(m2, 1) + " ms " + ok(a == b) }); }
    { Trie tr; for (auto s : { "laptop", "laptop gamer", "lampara", "libro", "licuadora", "mouse" }) tr.insertar(s);
      auto s = tr.sugerir("la");
      t.push_back({ "Trie sugerir 'la'", unir(s), ok(s.size() == 3 && tr.contarPrefijo("l") == 5) }); }
    { std::vector<std::string> nombres; std::mt19937 rng(14);
      for (int i = 0; i < 100000; ++i) { std::string s; int L = 5 + rng() % 8; for (int k = 0; k < L; ++k) s.push_back(static_cast<char>('a' + rng() % 26)); nombres.push_back(s); }
      Cronometro c; Trie tr; for (auto& s : nombres) tr.insertar(s); double mb = c.ms();
      std::vector<std::string> pref; for (int i = 0; i < 1000; ++i) pref.push_back(nombres[rng() % nombres.size()].substr(0, 3));
      c.reiniciar(); volatile long long tot = 0; for (auto& p : pref) tot += tr.contarPrefijo(p); double mt = c.ms();
      c.reiniciar(); volatile long long tl = 0; for (int i = 0; i < 50; ++i) for (auto& s : nombres) tl += s.compare(0, 3, pref[i]) == 0; double ml = c.ms() * 1000 / 50;
      t.push_back({ "Trie 100 000 nombres, 1 000 prefijos", fmt(mt, 4) + " ms (lineal ~" + fmt(ml, 1) + " ms)", std::to_string(tr.nodos()) + " nodos, construccion " + fmt(mb, 1) + " ms" }); }
    return t;
}
