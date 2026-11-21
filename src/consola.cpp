// Version de consola (multiplataforma): ejecuta todas las verificaciones, benchmarks y el flujo end-to-end.
// Compilar: g++ -std=c++17 -O2 -Iinclude src/consola.cpp -o ecommerce_consola
// Uso: ecommerce_consola [cantidad_productos] [ruta_csv_opcional]
#include <cstdlib>
#include <iostream>
#include "../include/integracion.hpp"

static void imprimir(const std::string& titulo, const Tabla& t) {
    std::cout << "\n==== " << titulo << " ====\n";
    for (auto& f : t) std::cout << std::left << std::setw(42) << f.a << " | " << std::setw(60) << f.b << " | " << f.c << "\n";
}

int main(int argc, char** argv) {
    std::size_t n = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1000000;
    Catalogo cat;
    Cronometro c;
    if (argc > 2) {
        std::string err;
        if (!cat.importarCSV(argv[2], err)) { std::cerr << "Error CSV: " << err << "\n"; return 1; }
    } else cat.generar(n);
    std::cout << "Catalogo listo: " << cat.size() << " productos en " << fmt(c.ms(), 1) << " ms\n";
    imprimir("MODULO 1.1 Catalogo O(log N)", pruebaCatalogo(cat));
    imprimir("MODULO 1.2 Configurador Backtracking (presupuesto 900)", pruebaConfigurador(900));
    imprimir("MODULO 1.3 Asignacion Greedy First Fit", pruebaGreedy());
    imprimir("MODULO 2 Estructuras de alta disponibilidad", pruebaModulo2());
    imprimir("MODULO 3 Grafos y logistica", pruebaModulo3());
    imprimir("MODULO 4 Programacion dinamica y cadenas", pruebaModulo4());
    imprimir("10. Flujo end-to-end (1 000 solicitudes)", ejecutarFlujoCompleto(cat, 1000).tabla);
    imprimir("10.3 Escalabilidad IND-I03 (1 000 / 10 000 / 100 000)", pruebaEscalabilidad(cat));
    std::cout << "\nMemoria pico del proceso: " << fmt(memoriaPicoMB(), 1) << " MB\n";
    return 0;
}
