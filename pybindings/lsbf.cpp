#include "BloomFilter.hpp"
#include <iostream>
#include <memory>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

using namespace std;

namespace py = pybind11;

class LSBF_Euclidean {
public:
  LSBF_Euclidean(double cutoff, double dataDim, size_t oneFilterSize,
                 size_t numFilterReps)
      : cutoff(cutoff), dataDim(dataDim), oneFilterSize(oneFilterSize),
        numFilterReps(numFilterReps), trained(false) {}

  // See https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=numpy#arrays
  // for explanation of why we do py::array::c_style and py::array::forcecase
  // Basically ensures array is in dnense row major order  
  void addPoint(py::array_t<double, py::array::c_style | py::array::forcecast> array) {
    if (!checkTrained(true)) {
      return;
    }
  }

  void addPoints(py::array_t<double, py::array::c_style | py::array::forcecast> array) {
    if (!checkTrained(true)) {
      return;
    }
  }

  void train(py::array_t<double, py::array::c_style | py::array::forcecast> array) {
    if (!checkTrained(false)) {
      return;
    }
    trained = true;
  }
  size_t getNumCollisions(py::array_t<double, py::array::c_style | py::array::forcecast> query) {
    if (!checkTrained(true)) {
      return 0;
    }
    return 0;
  }
  // py::array_t<size_t> getNumCollisionsBatch(py::array_t<double> queries) {}

private:
  double cutoff, dataDim;
  size_t oneFilterSize, numFilterReps;
  bool trained;
  unique_ptr<BloomFilter<double>> filter;

  bool checkTrained(bool goal) {
    if (goal == trained) {
      return true;
    } else if (goal) {
      cerr << "You cannot train this LSBF because it has already been trained, "
              "this is a NOOP."
           << endl;
    } else {
      cerr << "You cannot call this method before the LSBF has been trained, "
              "this is a NOOP."
           << endl;
    }
    return false;
  }
};

PYBIND11_MODULE(lsbf, m) {
  py::class_<LSBF_Euclidean>(m, "LSBF_Euclidean")
      .def(py::init<double, double, size_t, size_t>())
      .def("addPoint", &LSBF_Euclidean::addPoint)
      .def("addPointsBatch", &LSBF_Euclidean::addPoints)
      .def("train", &LSBF_Euclidean::train)
      .def("getNumCollisions", &LSBF_Euclidean::getNumCollisions);
  // .def("getNumCollisionsBatch", &LSBF_Euclidean::getNumCollisionsBatch);
}
