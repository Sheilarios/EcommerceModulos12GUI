#pragma once
// MODULO 1 - Componente 1: Catalogo eficiente con busqueda O(log N)
// RN-C01 unicidad de ID, RN-C02 categoria 0-19, RN-C03 Premium/Economico, RN-C04 orden por ID.
#include <algorithm>
#include <array>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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
            if (productos_[a].precio != productos_[b].precio) return productos_[a].precio < productos_[b].precio;
            return productos_[a].id < productos_[b].id;
        });
        for (auto& c : porCategoriaNivel_) for (auto& n : c) n.clear();
        for (std::size_t i = 0; i < productos_.size(); ++i) {
            const auto& p = productos_[i];
            if (p.categoria >= 0 && p.categoria < 20) porCategoriaNivel_[p.categoria][p.esPremium ? 1 : 0].push_back(i);
        }
    }

public:
    std::size_t size() const { return productos_.size(); }
    // Memoria estimada del catalogo e indices (bytes).
    std::size_t memoriaBytes() const {
        std::size_t b = productos_.capacity() * sizeof(Producto) + porPrecio_.capacity() * sizeof(std::size_t);
        for (auto& p : productos_) if (p.nombre.capacity() > 15) b += p.nombre.capacity() + 1;
        for (auto& c : porCategoriaNivel_) for (auto& n : c) b += n.capacity() * sizeof(std::size_t);
        return b;
    }
    // Recorrido lineal por rango de precio (linea base AS-IS para comparar).
    std::size_t contarRangoLineal(double minimo, double maximo) const {
        std::size_t c = 0; for (auto& p : productos_) if (p.precio >= minimo && p.precio <= maximo) ++c; return c;
    }
    std::size_t contarRangoIndice(double minimo, double maximo) const {
        auto lo = std::lower_bound(porPrecio_.begin(), porPrecio_.end(), minimo, [&](std::size_t i, double v) { return productos_[i].precio < v; });
        auto hi = std::upper_bound(porPrecio_.begin(), porPrecio_.end(), maximo, [&](double v, std::size_t i) { return v < productos_[i].precio; });
        return hi > lo ? static_cast<std::size_t>(hi - lo) : 0;
    }
    const std::vector<Producto>& productos() const { return productos_; }
    const Producto& enPosicion(std::size_t i) const { return productos_[i]; }

    void generar(std::size_t cantidad, unsigned semilla = 42) {
        std::mt19937 rng(semilla);
        std::uniform_real_distribution<double> precio(5.0, 2000.0);
        static const char* bases[] = { "laptop", "laptop gamer", "lampara", "libro", "licuadora", "mouse", "monitor",
            "mochila", "teclado", "telefono", "tablet", "audifonos", "auricular", "camara", "cargador", "cable" };
        std::vector<Producto> nuevos;
        nuevos.reserve(cantidad);
        for (std::size_t i = 0; i < cantidad; ++i) {
            int id = static_cast<int>(i * 3 + 1);
            nuevos.push_back({ id, static_cast<int>(rng() % 20), precio(rng), (rng() % 2) == 0,
                std::string(bases[rng() % 16]) + " " + std::to_string(id) });
        }
        productos_ = std::move(nuevos);
        reconstruirIndices();
    }

    // Importa CSV con encabezado id,nombre,categoria,precio,nivel. Valida antes de reemplazar.
    bool importarCSV(const std::string& ruta, std::string& error) {
        std::ifstream in(ruta);
        if (!in) { error = "No se pudo abrir el archivo."; return false; }
        std::string linea;
        if (!std::getline(in, linea)) { error = "Archivo vacio."; return false; }
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        if (linea.rfind("\xEF\xBB\xBF", 0) == 0) linea = linea.substr(3);
        if (linea != "id,nombre,categoria,precio,nivel") { error = "Encabezado invalido."; return false; }
        std::vector<Producto> nuevos;
        std::set<int> ids;
        int nl = 1;
        while (std::getline(in, linea)) {
            ++nl;
            if (!linea.empty() && linea.back() == '\r') linea.pop_back();
            if (linea.empty()) continue;
            std::vector<std::string> c;
            std::stringstream ss(linea); std::string campo;
            while (std::getline(ss, campo, ',')) c.push_back(campo);
            if (c.size() != 5) { error = "Linea " + std::to_string(nl) + ": se esperaban 5 campos."; return false; }
            try {
                Producto p;
                std::size_t pos = 0;
                p.id = std::stoi(c[0], &pos); if (pos != c[0].size() || p.id < 0) throw 1;
                p.nombre = c[1];
                p.categoria = std::stoi(c[2], &pos); if (pos != c[2].size() || p.categoria < 0 || p.categoria > 19) throw 1;
                p.precio = std::stod(c[3], &pos); if (pos != c[3].size() || p.precio < 0) throw 1;
                p.esPremium = (c[4] == "Premium" || c[4] == "premium" || c[4] == "1");
                if (!ids.insert(p.id).second) { error = "Linea " + std::to_string(nl) + ": ID duplicado."; return false; }
                nuevos.push_back(p);
            } catch (...) { error = "Linea " + std::to_string(nl) + ": valor fuera de rango o invalido."; return false; }
        }
        if (nuevos.empty()) { error = "El CSV no contiene productos."; return false; }
        productos_ = std::move(nuevos);
        reconstruirIndices();
        return true;
    }

    const Producto* buscarID(int id) const {
        auto it = std::lower_bound(productos_.begin(), productos_.end(), id,
            [](const Producto& p, int v) { return p.id < v; });
        return (it != productos_.end() && it->id == id) ? &*it : nullptr;
    }
    const Producto* buscarLineal(int id) const {
        for (const auto& p : productos_) if (p.id == id) return &p;
        return nullptr;
    }
    std::vector<const Producto*> buscarRango(double minimo, double maximo, std::size_t limite = 500) const {
        std::vector<const Producto*> r;
        if (minimo > maximo) return r;
        auto it = std::lower_bound(porPrecio_.begin(), porPrecio_.end(), minimo,
            [&](std::size_t i, double v) { return productos_[i].precio < v; });
        for (; it != porPrecio_.end() && productos_[*it].precio <= maximo && r.size() < limite; ++it)
            r.push_back(&productos_[*it]);
        return r;
    }
    std::vector<const Producto*> buscarCategoria(int categoria, bool premium, std::size_t limite = 500) const {
        std::vector<const Producto*> r;
        if (categoria < 0 || categoria >= 20) return r;
        for (std::size_t i : porCategoriaNivel_[categoria][premium ? 1 : 0]) {
            if (r.size() >= limite) break;
            r.push_back(&productos_[i]);
        }
        return r;
    }
};
