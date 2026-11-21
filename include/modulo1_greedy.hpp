#pragma once
// MODULO 1 - Componente 3: Motor de asignacion Greedy (First Fit).
// RN-A01 urgencia/fecha/ratio, RN-A02 capacidad, RN-A03 First Fit, RN-A04 sin duplicar, RN-A05 urgencia.
#include <algorithm>
#include <random>
#include <vector>

struct Solicitud {
    int id{};
    int urgencia{};     // mayor = mas urgente
    int fechaEntrega{}; // dia; menor = mas proxima
    int costo{};        // capacidad demandada
    double ganancia{};
    double ratio() const { return costo > 0 ? ganancia / costo : 0.0; }
};
struct Recurso { int id{}; int capacidad{}; int usado{}; };
struct Asignacion { int solicitud; int recurso; };

enum class OrdenGreedy { Prioridad, Ratio };

struct ResultadoGreedy {
    std::vector<Asignacion> asignaciones;
    double valorTotal = 0.0;
    std::vector<int> noAsignadas;
};

inline void ordenarSolicitudes(std::vector<Solicitud>& s, OrdenGreedy orden) {
    if (orden == OrdenGreedy::Prioridad) {
        std::stable_sort(s.begin(), s.end(), [](const Solicitud& a, const Solicitud& b) {
            if (a.urgencia != b.urgencia) return a.urgencia > b.urgencia;          // RN-A05
            if (a.fechaEntrega != b.fechaEntrega) return a.fechaEntrega < b.fechaEntrega;
            return a.ratio() > b.ratio();                                        // RN-A01 desempate
        });
    } else {
        std::stable_sort(s.begin(), s.end(), [](const Solicitud& a, const Solicitud& b) { return a.ratio() > b.ratio(); });
    }
}

// First Fit: cada solicitud va al primer recurso con capacidad suficiente.
inline ResultadoGreedy asignarFirstFit(std::vector<Solicitud> sols, std::vector<Recurso>& recursos, OrdenGreedy orden) {
    ResultadoGreedy r;
    ordenarSolicitudes(sols, orden);
    for (const auto& s : sols) {
        bool hecho = false;
        for (auto& rec : recursos) {
            if (rec.usado + s.costo <= rec.capacidad) { // RN-A02 / RN-A04
                rec.usado += s.costo;
                r.asignaciones.push_back({ s.id, rec.id });
                r.valorTotal += s.ganancia;
                hecho = true;
                break;
            }
        }
        if (!hecho) r.noAsignadas.push_back(s.id);
    }
    return r;
}

// Best Fit (variante de comparacion; cambia RN-A03): recurso con menor holgura suficiente.
inline ResultadoGreedy asignarBestFit(std::vector<Solicitud> sols, std::vector<Recurso>& recursos, OrdenGreedy orden) {
    ResultadoGreedy r;
    ordenarSolicitudes(sols, orden);
    for (const auto& s : sols) {
        Recurso* mejor = nullptr;
        for (auto& rec : recursos) {
            int libre = rec.capacidad - rec.usado;
            if (libre >= s.costo && (!mejor || libre < mejor->capacidad - mejor->usado)) mejor = &rec;
        }
        if (mejor) { mejor->usado += s.costo; r.asignaciones.push_back({ s.id, mejor->id }); r.valorTotal += s.ganancia; }
        else r.noAsignadas.push_back(s.id);
    }
    return r;
}

// Optimo exacto por fuerza bruta (solo instancias pequenas): cada solicitud a un recurso o a ninguno.
inline double optimoExacto(const std::vector<Solicitud>& s, std::vector<int> cap, std::size_t i = 0) {
    if (i == s.size()) return 0.0;
    double best = optimoExacto(s, cap, i + 1);
    for (std::size_t r = 0; r < cap.size(); ++r) {
        if (cap[r] >= s[i].costo) {
            cap[r] -= s[i].costo;
            best = std::max(best, s[i].ganancia + optimoExacto(s, cap, i + 1));
            cap[r] += s[i].costo;
        }
    }
    return best;
}

inline std::vector<Solicitud> generarSolicitudes(int n, unsigned semilla, int cmin = 1, int cmax = 10) {
    std::mt19937 rng(semilla);
    std::vector<Solicitud> v; v.reserve(n);
    for (int i = 0; i < n; ++i) {
        int costo = cmin + static_cast<int>(rng() % (cmax - cmin + 1));
        v.push_back({ i, 1 + static_cast<int>(rng() % 5), 1 + static_cast<int>(rng() % 30), costo,
            costo * (0.5 + (rng() % 1000) / 500.0) });
    }
    return v;
}
inline std::vector<Recurso> generarRecursos(int n, int capacidad) {
    std::vector<Recurso> v; for (int i = 0; i < n; ++i) v.push_back({ i, capacidad, 0 }); return v;
}
