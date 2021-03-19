#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class LSBF_Euclidean {
public:
  LSBF_Euclidean(double cutoff, double dataDim, size_t byteSize)
      : cutoff(cutoff), dataDim(dataDim), byteSize(byteSize) {}
  void addPoint(py::array_t<double> array) {}
  void addPoints(py::array_t<double> array) {}
  void train(py::array_t<double> array) {}
  size_t getNumCollisions(py::array_t<double> query) {return 0;}
  // py::array_t<size_t> getNumCollisionsBatch(py::array_t<double> queries) {}

private:
  double cutoff, dataDim;
  size_t byteSize;
};


PYBIND11_MODULE(lsbf, m) {
  py::class_<LSBF_Euclidean>(m, "LSBF_Euclidean")
      .def(py::init<double, double, size_t>())
      .def("addPoint", &LSBF_Euclidean::addPoint)
      .def("addPointsBatch", &LSBF_Euclidean::addPoints)
      .def("train", &LSBF_Euclidean::train)
      .def("getNumCollisions", &LSBF_Euclidean::getNumCollisions);
      // .def("getNumCollisionsBatch", &LSBF_Euclidean::getNumCollisionsBatch);
}
