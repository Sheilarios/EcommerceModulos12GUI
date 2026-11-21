#pragma once
// 10. Integracion completa: flujo end-to-end por solicitud.
// (0) Trie -> (1) busqueda binaria -> (2) backtracking -> (3) prioridad -> (4) First Fit en lote
// -> (5) Dijkstra desde el centro asignado -> (6) mochila de complementos -> (7) respuesta.
// RN-I01 flujo secuencial, RN-I02 productos del catalogo, RN-I03 < 10 ms, RN-D01 presupuesto sobrante.
#include <algorithm>
#include <random>
#include "pruebas.hpp"

struct ResultadoFlujo { Tabla tabla; std::vector<std::string> ejemplo; double totalUs = 0; double mediaUs = 0; };

inline ResultadoFlujo ejecutarFlujoCompleto(const Catalogo& cat, int nSolicitudes = 1000) {
    ResultadoFlujo out; Tabla& t = out.tabla;
    if (cat.size() == 0) { t.push_back({ "Catalogo", "vacio: genere o importe primero", "-" }); return out; }
    std::mt19937 rng(2026);

    // Infraestructura: Trie de nombres (muestra), red de 300 centros, 100 recursos de capacidad 30.
    Trie trie; std::size_t paso = std::max<std::size_t>(1, cat.size() / 50000);
    for (std::size_t i = 0; i < cat.size(); i += paso) trie.insertar(cat.enPosicion(i).nombre);
    const int C = 300; GrafoP red(C);
    for (int i = 1; i < C; ++i) { int a = rng() % i; long long w = 5 + rng() % 60; red[a].push_back({ i, w }); red[i].push_back({ a, w }); }
    for (int i = 0; i < 3 * C; ++i) { int a = rng() % C, b = rng() % C; long long w = 5 + rng() % 60; if (a != b) { red[a].push_back({ b, w }); red[b].push_back({ a, w }); } }
    auto recursos = generarRecursos(std::max(100, nSolicitudes / 10), 30); // recursos proporcionales al lote
    std::vector<Item> complementos; for (int i = 0; i < 15; ++i) complementos.push_back({ 40 + (int)(rng() % 160), 5 + (int)(rng() % 80) });
    static const char* pref[] = { "la", "lib", "mo", "te", "ca", "au", "li", "ta" };

    struct Pend { Solicitud s; double lat; double presupuesto; double costoCfg; int cliente; int idProd; };
    std::vector<Pend> pend; std::vector<double> lat;
    double tTrie = 0, tCat = 0, tCfg = 0, tPri = 0; int errores = 0, noEncontrados = 0;

    for (int k = 0; k < nSolicitudes; ++k) {
        Cronometro c;
        auto sug = trie.sugerir(pref[rng() % 8], 5); double a = c.us(); tTrie += a;            // (0)
        int id = cat.enPosicion(rng() % cat.size()).id; c.reiniciar();
        const Producto* p = cat.buscarID(id); double b = c.us(); tCat += b;                   // (1)
        if (!p) { ++noEncontrados; continue; }
        double presupuesto = 450 + rng() % 501;
        Configurador cf = crearConfigurador(4, 10, presupuesto, 7 + (k % 50));
        cf.nivelRequerido = p->esPremium ? 1 : 0; c.reiniciar();
        auto sol = cf.generar(true, true); double d = c.us(); tCfg += d;                     // (2)
        if (sol.empty()) { ++errores; lat.push_back(a + b + d); continue; }                    // rama de error
        c.reiniciar();
        double costo = Configurador::costo(sol[0]);
        Solicitud s{ k, 1 + (int)(rng() % 5), 1 + (int)(rng() % 30), 1 + (int)(rng() % 10), presupuesto - costo + 50 };
        double e = c.us(); tPri += e;                                                          // (3)
        pend.push_back({ s, a + b + d + e, presupuesto, costo, (int)(rng() % C), p->id });
    }
    // (4) asignacion First Fit del lote
    std::vector<Solicitud> lote; for (auto& x : pend) lote.push_back(x.s);
    // La cola de prioridad (Modulo 2) alimenta el motor First Fit (Modulo 1).
    ColaSolicitudes cola; Cronometro c; for (auto& s : lote) cola.push(s);
    auto res = asignarDesdeCola(cola, recursos); double tAsig = c.us();
    { auto rec2 = generarRecursos(std::max(100, nSolicitudes / 10), 30); auto ref = asignarFirstFit(lote, rec2, OrdenGreedy::Prioridad);
      if (ref.asignaciones.size() != res.asignaciones.size()) t.push_back({ "Aviso", "cola y ordenamiento difieren", "-" }); }
    double asigPorSol = pend.empty() ? 0 : tAsig / pend.size();
    std::vector<int> recursoDe(nSolicitudes, -1); for (auto& a : res.asignaciones) recursoDe[a.solicitud] = a.recurso;

    double tDij = 0, tMoch = 0; int nDij = 0; bool rnd01 = true;
    for (auto& x : pend) {
        double l = x.lat + asigPorSol;
        if (recursoDe[x.s.id] >= 0) {
            int centro = (recursoDe[x.s.id] * 3) % C; c.reiniciar();
            auto dist = dijkstra(red, centro); double dd = c.us(); tDij += dd; ++nDij;          // (5)
            int sobrante = static_cast<int>(x.presupuesto - x.costoCfg); c.reiniciar();
            auto m = mochilaDP(complementos, std::max(0, sobrante)); double dm = c.us(); tMoch += dm; // (6)
            int peso = 0; for (int i : m.elegidos) peso += complementos[i].peso; rnd01 &= peso <= sobrante;
            l += dd + dm;
            if (out.ejemplo.empty() && !m.elegidos.empty())
                out.ejemplo = { "Solicitud #" + std::to_string(x.s.id) + " producto ID " + std::to_string(x.idProd),
                    "Configuracion S/ " + fmt(x.costoCfg) + " de S/ " + fmt(x.presupuesto),
                    "Recurso " + std::to_string(recursoDe[x.s.id]) + ", centro " + std::to_string(centro) + ", entrega " + std::to_string(dist[x.cliente]) + " min al cliente " + std::to_string(x.cliente),
                    "Complementos {" + unir(m.elegidos) + "} valor " + std::to_string(m.valor) };
        }
        lat.push_back(l);                                                                      // (7)
    }
    std::sort(lat.begin(), lat.end());
    auto pct = [&](double q) { return lat.empty() ? 0.0 : lat[std::min(lat.size() - 1, (std::size_t)(q * lat.size()))]; };
    double media = 0; for (double v : lat) media += v; media = lat.empty() ? 0 : media / lat.size();
    int n = nSolicitudes;
    t.push_back({ "Solicitudes con configuracion valida", std::to_string(pend.size()) + " de " + std::to_string(n), "-" });
    t.push_back({ "Solicitudes con error (sin config.)", std::to_string(errores) + " (" + fmt(100.0 * errores / n, 1) + " %)", "rama de error" });
    t.push_back({ "Productos no encontrados (RN-I02)", std::to_string(noEncontrados), ok(noEncontrados == 0) });
    t.push_back({ "Asignadas a un recurso", std::to_string(res.asignaciones.size()) + " de " + std::to_string(pend.size()), "100 recursos cap. 30" });
    t.push_back({ "Etapa Trie (media)", fmt(tTrie / n, 2) + " us", "-" });
    t.push_back({ "Etapa catalogo (media)", fmt(tCat / n, 2) + " us", "-" });
    t.push_back({ "Etapa configurador (media)", fmt(tCfg / n, 2) + " us", "-" });
    t.push_back({ "Etapa prioridad (media)", fmt(pend.empty() ? 0 : tPri / pend.size(), 2) + " us", "-" });
    t.push_back({ "Asignacion del lote", fmt(tAsig, 1) + " us", fmt(asigPorSol, 3) + " us/sol" });
    t.push_back({ "Dijkstra entrega (media)", fmt(nDij ? tDij / nDij : 0, 1) + " us", "red de 300 centros" });
    t.push_back({ "Mochila complementos (media)", fmt(nDij ? tMoch / nDij : 0, 2) + " us", "RN-D01 " + ok(rnd01) });
    t.push_back({ "Latencia media / p95 / p99", fmt(media, 2) + " / " + fmt(pct(0.95), 1) + " / " + fmt(pct(0.99), 1) + " us", "max " + fmt(lat.empty() ? 0 : lat.back(), 1) + " us" });
    out.mediaUs = media; for (double v : lat) out.totalUs += v;
    t.push_back({ "Memoria del proceso (actual / pico)", fmt(memoriaActualMB(), 1) + " / " + fmt(memoriaPicoMB(), 1) + " MB", "-" });
    t.push_back({ "RN-I03 < 10 ms por solicitud", (lat.empty() || lat.back() < 10000) ? "CUMPLE" : "NO CUMPLE", "-" });
    for (auto& e : out.ejemplo) t.push_back({ "Ejemplo", e, "-" });
    return out;
}

// IND-I03: escalabilidad del flujo con 1 000, 10 000 y 100 000 solicitudes.
inline Tabla pruebaEscalabilidad(const Catalogo& cat) {
    Tabla t;
    if (cat.size() == 0) { t.push_back({ "Catalogo", "vacio: genere o importe primero", "-" }); return t; }
    double base = 0;
    for (int n : { 1000, 10000, 100000 }) {
        Cronometro c; auto r = ejecutarFlujoCompleto(cat, n); double ms = c.ms();
        if (!base) base = r.mediaUs;
        t.push_back({ std::to_string(n) + " solicitudes", "procesamiento " + fmt(r.totalUs / 1000, 1) + " ms (" + fmt(r.mediaUs, 2) + " us/sol)",
            "pared " + fmt(ms, 0) + " ms | " + (base > 0 ? fmt(100 * (r.mediaUs / base - 1), 0) + " % vs 1 000" : "-") });
    }
    t.push_back({ "IND-I03 crecimiento", "casi lineal si us/sol se mantiene estable", "meta < 10 ms por solicitud" });
    return t;
}
