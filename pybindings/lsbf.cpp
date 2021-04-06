#include "BloomFilter.hpp"
#include "EuclideanHashFunction.hpp"
#include "HashFunction.hpp"
#include <iostream>
#include <memory>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <utility>

using namespace std;

namespace py = pybind11;

double getAUCLocal(bool *groundTruth, vector<size_t> thresholdResults) {
  size_t countTotalPositive = 0;
  size_t countTotalNegative = 0;
  for (size_t i = 0; i < thresholdResults.size(); i++) {
    if (groundTruth[i]) {
      countTotalPositive++;
    } else {
      countTotalNegative++;
    }
  }

  vector<pair<size_t, bool>> results;
  for (size_t i = 0; i < thresholdResults.size(); i++) {
    results.push_back(make_pair(thresholdResults.at(i), groundTruth[i]));
  }
  sort(results.begin(), results.end());

  vector<double> fprs;
  vector<double> tprs;
  size_t countTruePositive = 0;
  size_t countFalsePositive = 0;
  size_t currentThreshold = thresholdResults.at(thresholdResults.size() - 1);
  for (int i = thresholdResults.size() - 1; i >= 0; i--) {
    if (currentThreshold != results.at(i).first) {
      currentThreshold = results.at(i).first;
      tprs.push_back((double)countTruePositive / (double)countTotalPositive);
      fprs.push_back((double)countFalsePositive / (double)countTotalNegative);
    }
    if (results.at(i).second) {
      countTruePositive++;
    } else {
      countFalsePositive++;
    }
  }
  tprs.push_back((double)countTruePositive / (double)countTotalPositive);
  fprs.push_back((double)countFalsePositive / (double)countTotalNegative);
  tprs.push_back(1);
  fprs.push_back(1);

  double auc = 0;
  double lastFPR = 0;
  double lastTPR = 0;
  for (size_t i = 0; i < fprs.size(); i++) {
    if (lastTPR + tprs.at(i) != 0) {
      auc += (fprs.at(i) - lastFPR) * (lastTPR + tprs.at(i)) / 2;
      lastFPR = fprs.at(i);
      lastTPR = tprs.at(i);
    }
  }
  return auc;
}
pair<double, double> trainEuclidean(size_t cutoff) {
  throw runtime_error("Unimplemented operation.");
}

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
  }

  // See
  // https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html?highlight=numpy#arrays
  // for explanation of why we do py::array::c_style and py::array::forcecase
  // Basically ensures array is in dense row major order
  void setupAndTrain(
      py::array_t<double, py::array::c_style | py::array::forcecast> data,
      py::array_t<double, py::array::c_style | py::array::forcecast> training,
      py::array_t<bool, py::array::c_style | py::array::forcecast> ground) {

    throw runtime_error("Unimplemented operation.");

    // checkState(0);

    // // http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html#More-on-working-with-numpy-arrays
    // auto dataBuf = data.request();
    // auto trainingBuf = training.request();
    // auto groundBuf = ground.request();

    // if (dataBuf.ndim != 2 || trainingBuf.ndim != 2 || groundBuf.ndim != 1) {
    //   throw runtime_error(
    //       "Data and training must be 2 dimensional Numpy arrays, ground truth "
    //       "must be a 1 dimensional Numpy array.");
    // }
    // if ((size_t)dataBuf.shape[0] != numDataPoints ||
    //     (size_t)trainingBuf.shape[0] != numTrainPoints ||
    //     (size_t)dataBuf.shape[1] != dataDim ||
    //     (size_t)trainingBuf.shape[1] != dataDim ||
    //     (size_t)groundBuf.shape[0] != numTrainPoints) {
    //   throw runtime_error("Incorrect shape in one of parameters.");
    // }

    // pair<double, double> bestParams = trainEuclidean(cutoff);

    // // Create trained bloom filter
    // EuclideanHashFunction *trainedHash = new EuclideanHashFunction(
    //     bestParams.first, 42, numFilterReps, bestParams.second, dataDim);
    // filter = new BloomFilter<double *>(trainedHash, oneFilterSize);
    // vector<double *> pointers;
    // double *dataPtr = (double *)dataBuf.ptr;
    // for (size_t i = 0; i < numDataPoints; i++) {
    //   pointers.push_back(dataPtr + i * dataDim);
    // }
    // filter->addPoints(pointers);

    // state = 1;
  }

  void setup(py::array_t<double, py::array::c_style | py::array::forcecast> data, double r, double concatenationNum) {
    
    // Check conditions
    checkState(0);
    auto dataBuf = data.request();
    if (dataBuf.ndim != 2 || (size_t)dataBuf.shape[1] != dataDim) {
      throw runtime_error(
          "Data must be a 2 dimensional Numpy array and each entry must be "
          "of the dimension passed into the constructor.");
    }
    
    // Create filter
    EuclideanHashFunction *trainedHash = new EuclideanHashFunction(
        r, 42, numFilterReps, concatenationNum, dataDim);
    filter = new BloomFilter<double *>(trainedHash, oneFilterSize);
    vector<double *> pointers;
    double *dataPtr = (double *)dataBuf.ptr;
    for (size_t i = 0; i < numDataPoints; i++) {
      pointers.push_back(dataPtr + i * dataDim);
    }
    filter->addPoints(pointers);

    state = 1;
  }

  double
  getAUC(py::array_t<double, py::array::c_style | py::array::forcecast> queries,
         py::array_t<bool, py::array::c_style | py::array::forcecast> ground) {
    checkState(1);
    auto queryBuf = queries.request();
    auto groundBuf = ground.request();
    if ((size_t)queryBuf.ndim != 2 || (size_t)queryBuf.shape[1] != dataDim) {
      throw runtime_error("Incorrect query shape.");
    }
    if ((size_t)groundBuf.ndim != 1 ||
        groundBuf.shape[0] != queryBuf.shape[0]) {
      throw runtime_error("Incorrect ground shape.");
    }
    vector<size_t> thresholdResults(queryBuf.shape[0]);
    double *queryData = (double *)queryBuf.ptr;
    bool *groundData = (bool *)groundBuf.ptr;

    size_t totalCollisions = 0;
    for (int i = 0; i < queryBuf.shape[0]; i++) {
      thresholdResults.at(i) = filter->numCollisions(queryData + i * dataDim);
      totalCollisions += thresholdResults.at(i);
    }
    // cout << "Average collisions: " << totalCollisions / (double) queryBuf.shape[0] << endl;

    return getAUCLocal(groundData, thresholdResults);
  }

  size_t getNumCollisions(
      py::array_t<double, py::array::c_style | py::array::forcecast> query) {
    checkState(1);
    auto queryBuf = query.request();
    if ((size_t)queryBuf.ndim != 1 || (size_t)queryBuf.shape[0] != dataDim) {
      throw runtime_error("Incorrect query shape.");
    }
    return filter->numCollisions((double *)queryBuf.ptr);
  }

private:
  double cutoff;
  size_t dataDim, oneFilterSize, numFilterReps, numDataPoints, numTrainPoints;
  int state;
  BloomFilter<double *> *filter;
  EuclideanHashFunction *filterHashes;

  bool checkState(int goalState) {
    if (state == goalState) {
      return true;
    } else {
      throw runtime_error(
          "You must call methods on this LSBF in the following order: "
          "setupAndTrain/setup, getNumCollisions. \n For now, "
          "setupAndTrain/setup can only be called once. This call is "
          "treated as a NOOP. ");
      return false;
    }
  }
};

PYBIND11_MODULE(lsbf, m) {
  py::class_<LSBF_Euclidean>(m, "LSBF_Euclidean")
      .def(py::init<double, size_t, size_t, size_t, size_t, size_t, uint32_t>())
      .def("setupAndTrain", &LSBF_Euclidean::setupAndTrain)
      .def("setup", &LSBF_Euclidean::setup)
      .def("getNumCollisions", &LSBF_Euclidean::getNumCollisions)
      .def("getAUC", &LSBF_Euclidean::getAUC);
}
