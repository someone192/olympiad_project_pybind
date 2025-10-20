#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Для автоматической конверсии vector, tuple
#include "anneal.h"

namespace py = pybind11;

// PYBIND11_MODULE: Определяет модуль "my_module"
// m — это py::module_ объект для экспорта
PYBIND11_MODULE(my_module, m) {
    // Описание модуля (опционально)
    m.doc() = "Pybind11 module for 3D annealing algorithm";

    // Экспорт функции simulated_annealing как "anneal_3d"
    // py::arg() — для именованных аргументов (как в Python def)
    m.def("anneal_3d", &simulated_annealing,
          py::arg("masses"), py::arg("L"), py::arg("W"), py::arg("H"),
          py::arg("target"), py::arg("axle_limits"), py::arg("initial_loads"),
          py::arg("axle_positions"), py::arg("total_limit"),
          py::arg("threshold"), py::arg("box_size"),
          "Performs 3D simulated annealing for cargo distribution");

    // pybind11 автоматически конвертирует:
    // std::vector<double> ↔ list[float]
    // std::tuple<double,double,double> ↔ tuple[float,float,float]
    // std::vector<std::tuple<double,double>> ↔ list[tuple[float,float]]
    // Возврат: std::vector<Pos> → list[tuple[float,float,float]]
    // Если ошибка — return {{-1,-1,-1}} → list[( -1,-1,-1 )]
}