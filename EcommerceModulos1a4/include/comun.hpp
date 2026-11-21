#pragma once
// Utilidades comunes: medicion de tiempo y filas de resultados.
#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

struct Fila {
    std::string a, b, c;
};
using Tabla = std::vector<Fila>;

class Cronometro {
    std::chrono::steady_clock::time_point t0_ = std::chrono::steady_clock::now();
public:
    void reiniciar() { t0_ = std::chrono::steady_clock::now(); }
    double us() const { return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0_).count(); }
    double ms() const { return us() / 1000.0; }
};

inline std::string fmt(double v, int dec = 2) {
    std::ostringstream os; os << std::fixed << std::setprecision(dec) << v; return os.str();
}
template <class T>
inline std::string unir(const std::vector<T>& v, const char* sep = ", ") {
    std::ostringstream os;
    for (std::size_t i = 0; i < v.size(); ++i) { if (i) os << sep; os << v[i]; }
    return os.str();
}
// Memoria residente del proceso (MB): actual y pico.
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")
inline double memoriaActualMB() { PROCESS_MEMORY_COUNTERS c{}; GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof(c)); return c.WorkingSetSize / 1048576.0; }
inline double memoriaPicoMB() { PROCESS_MEMORY_COUNTERS c{}; GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof(c)); return c.PeakWorkingSetSize / 1048576.0; }
#else
#include <fstream>
#include <sys/resource.h>
inline double memoriaActualMB() {
    std::ifstream f("/proc/self/status"); std::string k; double v = 0;
    while (f >> k) { if (k == "VmRSS:") { f >> v; return v / 1024.0; } }
    return 0;
}
inline double memoriaPicoMB() {
    rusage u{}; getrusage(RUSAGE_SELF, &u);
#if defined(__APPLE__)
    return u.ru_maxrss / 1048576.0;
#else
    return u.ru_maxrss / 1024.0;
#endif
}
#endif
inline std::string ok(bool b) { return b ? "OK" : "FALLA"; }
