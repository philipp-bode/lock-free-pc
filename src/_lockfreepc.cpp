#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <armadillo>
#include <vector>

#include "concurrency.hpp"
#include "constraint.hpp"
#include "graph.hpp"
#include "skeleton.hpp"
#include "worker.hpp"

// -------------
// pure C++ code
// -------------

std::shared_ptr<PCAlgorithm> run_pc(std::shared_ptr<arma::mat> data, double alpha, int nr_threads, std::string test_name) {
    auto alg = std::make_shared<PCAlgorithm>(data, alpha, nr_threads, test_name);

    auto colp = data->colptr(0);

    alg->build_graph();

    return alg;
}

// ----------------
// Python interface
// ----------------

namespace py = pybind11;

// wrap C++ function with NumPy array IO
py::tuple py_skeleton(py::array_t<double, py::array::f_style> array, double alpha, int nr_threads, std::string test_name = "pearson") {
    // check input dimensions
    if (array.ndim() != 2) throw std::runtime_error("Input should be 2-D NumPy array");

    int number_of_observations = array.shape()[0];
    int number_of_variables = array.shape()[1];

    auto r = array.mutable_unchecked<2>();
    auto mat = std::make_shared<arma::mat>(
        r.mutable_data(0, 0),
        number_of_observations,
        number_of_variables,
        /*copy_aux_mem*/ false,
        /*strict*/ true);

    auto alg = run_pc(mat, alpha, nr_threads, test_name);
    auto edges = alg->get_edges_with_weight();

    size_t ndim = 2;
    std::vector<size_t> shape = {edges.size() / 3, 3};
    std::vector<size_t> strides = {sizeof(double) * 3, sizeof(double)};

    auto edge_array = py::array(
        py::buffer_info(
            edges.data(),                            /* data as contiguous array  */
            sizeof(double),                          /* size of one scalar        */
            py::format_descriptor<double>::format(), /* data type                 */
            ndim,                                    /* number of dimensions      */
            shape,                                   /* shape of the matrix       */
            strides /* strides for each axis     */));

    auto separation_matrix = alg->get_separation_matrix();
    auto separation_sets = py::dict();

    rep(i, number_of_variables) {
        rep(j, number_of_variables) {
            auto pt = (*separation_matrix)[i * number_of_variables + j];
            if (pt != nullptr) {
                auto sep_list = py::list();
                for (auto const s : *pt) {
                    sep_list.append(s);
                }
                separation_sets[py::make_tuple(i, j)] = sep_list;
            }
        }
    }

    return py::make_tuple(edge_array, separation_sets);
}

// wrap as Python module
PYBIND11_MODULE(_lockfreepc, m) {
    m.doc() = "A wrapper module around the lockfreepc C++ implementation";

    m.def("skeleton", &py_skeleton, "Calculate the skeleton for a given np.Array");
}
