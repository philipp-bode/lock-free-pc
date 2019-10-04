#include <armadillo>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>


#include "constraint.hpp"
#include "graph.hpp"
#include "./worker.h"
#include "./concurrency.h"
#include "./skeleton.h"


// -------------
// pure C++ code
// -------------

std::shared_ptr<PCAlgorithm> run_pc(
    arma::Mat<double> &data,
    double alpha,
    int nr_threads
) {
    auto alg = make_shared<PCAlgorithm>(data.n_cols, alpha, data.n_rows, nr_threads);

    auto colp = data.colptr(0);

    alg->build_correlation_matrix(data);
    alg->build_graph();

    return alg;
}


// ----------------
// Python interface
// ----------------

namespace py = pybind11;

// wrap C++ function with NumPy array IO
py::array py_skeleton(
  py::array_t<double, py::array::c_style | py::array::forcecast> array,
  double alpha,
  int nr_threads
) {
  // check input dimensions
  if ( array.ndim()     != 2 )
    throw std::runtime_error("Input should be 2-D NumPy array");

  arma::Mat<double> mat(array.shape()[0], array.shape()[1], arma::fill::zeros);

  // copy py::array -> arma::Mat
  for (int x = 0; x < array.shape()[0]; x++) {
    for (int y = 0; y < array.shape()[1]; y++) {
      mat(x, y) = array.at(x, y);
    }
  }


  auto alg = run_pc(mat, alpha, nr_threads);
  auto edges = alg->get_edges();

  size_t              ndim    = 2;
  std::vector<size_t> shape   = { edges.size()/2 , 2};
  std::vector<size_t> strides = { sizeof(int)*2 , sizeof(int)};

  // return 2-D NumPy array
  return py::array(py::buffer_info(
    edges.data(),                           /* data as contiguous array  */
    sizeof(int),                            /* size of one scalar        */
    py::format_descriptor<int>::format(),   /* data type                 */
    ndim,                                   /* number of dimensions      */
    shape,                                  /* shape of the matrix       */
    strides                                 /* strides for each axis     */));
}

// wrap as Python module
PYBIND11_MODULE(lockfreepc, m) {
  m.doc() = "pybind11 example plugin";

  m.def("_skeleton", &py_skeleton, "Calculate the length of an array of vectors");
}
