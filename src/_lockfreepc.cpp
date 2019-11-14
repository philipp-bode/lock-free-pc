#include <armadillo>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>


#include "constraint.hpp"
#include "graph.hpp"
#include "worker.hpp"
#include "concurrency.hpp"
#include "skeleton.hpp"


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
py::tuple py_skeleton(
  py::array_t<double, py::array::c_style | py::array::forcecast> array,
  double alpha,
  int nr_threads
) {
  // check input dimensions
  if ( array.ndim()     != 2 )
    throw std::runtime_error("Input should be 2-D NumPy array");

  int number_of_observations = array.shape()[0];
  int number_of_variables = array.shape()[1];


  arma::Mat<double> mat(number_of_observations, number_of_variables, arma::fill::zeros);

  // copy py::array -> arma::Mat
  for (int x = 0; x < number_of_observations; x++) {
    for (int y = 0; y < number_of_variables; y++) {
      mat(x, y) = array.at(x, y);
    }
  }


  auto alg = run_pc(mat, alpha, nr_threads);
  auto edges = alg->get_edges();

  size_t              ndim    = 2;
  std::vector<size_t> shape   = { edges.size()/2 , 2};
  std::vector<size_t> strides = { sizeof(int)*2 , sizeof(int)};

  auto edge_array = py::array(py::buffer_info(
      edges.data(),                           /* data as contiguous array  */
      sizeof(int),                            /* size of one scalar        */
      py::format_descriptor<int>::format(),   /* data type                 */
      ndim,                                   /* number of dimensions      */
      shape,                                  /* shape of the matrix       */
      strides                                 /* strides for each axis     */));

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


  // return 2-D NumPy array
  return py::make_tuple(edge_array, separation_sets);
}

// wrap as Python module
PYBIND11_MODULE(_lockfreepc, m) {
  m.doc() = "A wrapper module around the lockfreepc C++ implementation";

  m.def("skeleton", &py_skeleton, "Calculate the skeleton for a given np.Array");
}
