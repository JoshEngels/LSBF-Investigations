#include "BloomFilter.hpp"
#include "EuclideanHashFunction.hpp"
#include "EuclideanHashFunctionTraining.hpp"
#include "HashFunction.hpp"
#include <iostream>
#include <memory>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>



using namespace std;

namespace py = pybind11;

double getAUC(bool *groundTruth, vector<size_t> thresholdResults) {
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
  }
  tprs.push_back((double)countTruePositive / (double)countTotalPositive);
  fprs.push_back((double)countFalsePositive / (double)countTotalNegative);
  tprs.push_back(1);
  fprs.push_back(1);

  double auc = 0;
  double lastFPR = 0;
  double lastTPR = 0;
  for (size_t i = 0; i < fprs.size(); i++) {
    auc += (fprs.at(i) - lastFPR) * (lastTPR + tprs.at(i)) / 2;
    lastFPR = fprs.at(i);
    lastTPR = tprs.at(i);
  }
  return auc;
}

// TODO: Figure out how to set these better
#define MIN_CONCATENATIONS 1
#define MAX_CONCATENATIONS 10
#define MIN_R 1
#define MAX_R 1000

shared_ptr<EuclideanHashFunction>
trainEuclidean(EuclideanHashFunctionTraining *storedHashes,
               size_t numHashesToTrainWith, size_t filterSize,
               size_t numTrainPoints, size_t numTestPoints, bool *groundTruth,
               size_t dataDim) {

  cout << "Training Euclidean" << endl;
  // double bestAUC = 0;
  // size_t bestConcatenationNum = -1;
  // double bestR = -1;

  // for (size_t i = MIN_CONCATENATIONS; i < MAX_CONCATENATIONS; i++) {

  //   // Find a place where AUC(currentR) < AUC(lastR), max is between
  //   lastlastR and currentR double lastR = MIN_R; double currentR = 2 * MIN_R;
  // }

  storedHashes->setHashParameters(200, 3, numHashesToTrainWith);
  BloomFilter<size_t> testFilter =
      BloomFilter<size_t>(storedHashes, filterSize);

  // Add all train points
  // TODO: Write a batch add method parallilized
  for (size_t i = 0; i < numTrainPoints; i++) {
    testFilter.addPoint(i);
  }

  // Do query and get AUC
  // TODO: Parallelize
  vector<size_t> queryResults;
  for (size_t i = 0; i < numTestPoints; i++) {
    queryResults.push_back(testFilter.numCollisions(numTestPoints + i));
  }
  cout << "AUC for 200-3: " << getAUC(groundTruth, queryResults) << endl;

  // Return best Euclidean hash function
  return make_shared<EuclideanHashFunction>(200, 42, numHashesToTrainWith, 3,
                                            dataDim);
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

    cout << "Setting up and training" << endl;

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
    double *trainPtr = (double *)trainingBuf.ptr;
#pragma omp parallel
    for (size_t i = 0; i < numTrainPoints; i++) {
      storedHashes.get()->storeVal(i + numDataPoints, trainPtr + i * dataDim);
    }

    EuclideanHashFunction *trainedHash =
        trainEuclidean(storedHashes.get(), numFilterReps, oneFilterSize,
                       numDataPoints, numTrainPoints, (bool *)groundBuf.ptr,
                       dataDim)
            .get();
    filter = make_shared<BloomFilter<double *>>(trainedHash, oneFilterSize);
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
  shared_ptr<BloomFilter<double *>> filter;
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
