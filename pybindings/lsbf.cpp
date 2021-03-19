#include "BloomFilter.hpp"
#include <EuclideanHashFunctionTraining.hpp>
#include <EuclideanHashFunction.hpp>
#include <iostream>
#include <memory>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

using namespace std;

namespace py = pybind11;

#define MAX_CONCATENATIONS 20

class LSBF_Euclidean {
public:
  LSBF_Euclidean(double cutoff, size_t dataDim, size_t oneFilterSize,
                 size_t numFilterReps, size_t numDataPoints,
                 size_t numTrainPoints, uint32_t key)
      : cutoff(cutoff), dataDim(dataDim), oneFilterSize(oneFilterSize),
        numFilterReps(numFilterReps), numDataPoints(numDataPoints),
        numTrainPoints(numTrainPoints), state(0) {

    // Set random seed
    srand(42);

    // Empty hash function for now
    storedHashes = make_shared<EuclideanHashFunctionTraining>(
        key + 1, numFilterReps * MAX_CONCATENATIONS, dataDim,
        numDataPoints + numTrainPoints);
  }

  // See
  // https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=numpy#arrays
  // for explanation of why we do py::array::c_style and py::array::forcecase
  // Basically ensures array is in dense row major order
  void setupAndTrain(
      py::array_t<double, py::array::c_style | py::array::forcecast> data,
      py::array_t<double, py::array::c_style | py::array::forcecast> training,
      py::array_t<bool, py::array::c_style | py::array::forcecast> ground) {

    checkState(0);

    // http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html#More-on-working-with-numpy-arrays
    auto dataBuf = data.request();
    auto trainingBuf = training.request();
    auto groundBuf = ground.request();

    if (dataBuf.ndim != 2 || trainingBuf.ndim != 2 || groundBuf.ndim != 1) {
      // cout << dataBuf.ndim  << " " << trainingBuf.ndim << " " <<
      // groundBuf.ndim << endl;
      throw runtime_error(
          "Data and training must be 2 dimensional Numpy arrays, ground truth "
          "must be a 1 dimensional Numpy array.");
    }
    if ((size_t)dataBuf.shape[0] != numDataPoints ||
        (size_t)trainingBuf.shape[0] != numTrainPoints ||
        (size_t)dataBuf.shape[1] != dataDim ||
        (size_t)trainingBuf.shape[1] != dataDim ||
        (size_t)groundBuf.shape[0] != numTrainPoints) {
      throw runtime_error("Incorrect shape in one of parameters.");
    }

    // Add data to stored hash
    double *dataPtr = (double *)dataBuf.ptr;
#pragma omp parallel
    for (size_t i = 0; i < numDataPoints; i++) {
      storedHashes.get()->storeVal(i, dataPtr + i * dataDim);
    }
    double *trainPtr = (double *)dataBuf.ptr;
#pragma omp parallel
    for (size_t i = 0; i < numTrainPoints; i++) {
      storedHashes.get()->storeVal(i + numDataPoints, trainPtr + i * dataDim);
    }

    storedHashes.get()->setHashParameters(300, 3, 100);
    // TODO: Build filter based off of trained values
    // TODO: Change filter to doubles, rewrite EuclideanHashFunction
    filter = make_shared<BloomFilter<vector<float>>>(new EuclideanHashFunction(300, 42, numFilterReps, 3, dataDim), oneFilterSize);

    state = 1;
  }

  size_t getNumCollisions(
      py::array_t<double, py::array::c_style | py::array::forcecast> query) {
    checkState(1);
    auto queryBuf = query.request();
    if ((size_t)queryBuf.ndim != 1 || (size_t)queryBuf.shape[0] != dataDim) {
      throw runtime_error("Incorrect query shape.");
    }
    return 0;
  }
  // py::array_t<size_t> getNumCollisionsBatch(py::array_t<double> queries) {}

private:
  double cutoff;
  size_t dataDim, oneFilterSize, numFilterReps, numDataPoints, numTrainPoints;
  int state;
  shared_ptr<BloomFilter<vector<float>>> filter;
  shared_ptr<EuclideanHashFunctionTraining> storedHashes;

  bool checkState(int goalState) {
    if (state == goalState) {
      return true;
    } else {
      throw runtime_error(
          "You must call methods on this LSBF in the following order: "
          "setupAndTrain, getNumCollisions. \n For now, "
          "setupAndTrain can only be called once. This call is "
          "treated as a NOOP. ");
      return false;
    }
  }
};

PYBIND11_MODULE(lsbf, m) {
  py::class_<LSBF_Euclidean>(m, "LSBF_Euclidean")
      .def(py::init<double, size_t, size_t, size_t, size_t, size_t, uint32_t>())
      .def("setupAndTrain", &LSBF_Euclidean::setupAndTrain)
      .def("getNumCollisions", &LSBF_Euclidean::getNumCollisions);
  // .def("getNumCollisionsBatch", &LSBF_Euclidean::getNumCollisionsBatch);
}
