#include "BloomFilter.hpp"
#include "EuclideanHashFunction.hpp"
#include "EuclideanHashFunctionTraining.hpp"
#include "HashFunction.hpp"
#include <iostream>
#include <memory>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <utility>

using namespace std;

namespace py = pybind11;

// TODO: Figure out how to set these better
#define TEST_NUM_FILTERS 20
#define IMPROVEMENT_PERCENT_LIMIT 1
#define MAX_CONCATENATIONS 32

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

pair<double, double> runTest(EuclideanHashFunctionTraining *storedHashes,
                             size_t filterSize, size_t numDataPoints,
                             size_t numTrainPoints, bool *groundTruth) {

  // Create test filter
  BloomFilter<size_t> testFilter =
      BloomFilter<size_t>(storedHashes, filterSize);

  // Add all train points
  vector<size_t> toAdd;
  for (size_t i = 0; i < numDataPoints; i++) {
    toAdd.push_back(i);
  }
  testFilter.addPoints(toAdd);

  // Do query and get AUC and total number of collisions
  double total = 0;
  vector<size_t> queryResults;
  for (size_t i = 0; i < numTrainPoints; i++) {
    size_t numCol = testFilter.numCollisions(numDataPoints + i);
    total += numCol;
    queryResults.push_back(numCol);
  }
  double thisAUC = getAUC(groundTruth, queryResults);

  // Return a pair of the auc and the total number of collisions
  return make_pair(thisAUC, total / numTrainPoints);
}

pair<double, double> trainEuclidean(EuclideanHashFunctionTraining *storedHashes,
                                    size_t filterSize, size_t numDataPoints,
                                    size_t numTrainPoints, bool *groundTruth) {

  // Keep track of best so far
  double bestAUC = 0;
  double bestR = -1;
  double bestConcatenations = -1;
  for (size_t numConcatenations = 2; numConcatenations <= MAX_CONCATENATIONS;
       numConcatenations *= 2) {
    double startR = 100;
    storedHashes->setHashParameters(startR, numConcatenations,
                                    TEST_NUM_FILTERS);
    pair<double, double> firstResult = runTest(
        storedHashes, filterSize, numDataPoints, numTrainPoints, groundTruth);

    // Find left and right such that left < half repetitons and right > half
    // repetitions
    double leftR, rightR;
    pair<double, double> leftScore, rightScore;
    if (firstResult.second > TEST_NUM_FILTERS / 2) {
      rightR = startR;
      rightScore = firstResult;
      leftR = rightR;
      while (true) {
        leftR /= 2;
        storedHashes->setHashParameters(leftR, numConcatenations,
                                        TEST_NUM_FILTERS);
        leftScore = runTest(storedHashes, filterSize, numDataPoints,
                            numTrainPoints, groundTruth);
        if (leftScore.second < TEST_NUM_FILTERS / 2) {
          break;
        }
      }
    } else {
      leftR = startR;
      leftScore = firstResult;
      rightR = leftR;
      while (true) {
        rightR *= 2;
        storedHashes->setHashParameters(rightR, numConcatenations,
                                        TEST_NUM_FILTERS);
        rightScore = runTest(storedHashes, filterSize, numDataPoints,
                             numTrainPoints, groundTruth);
        if (rightScore.second > TEST_NUM_FILTERS / 2) {
          break;
        }
      }
    }

    // Do the binary search
    while (
        abs(leftScore.second - TEST_NUM_FILTERS / 2) > TEST_NUM_FILTERS / 10 ||
        abs(rightScore.second - TEST_NUM_FILTERS / 2) > TEST_NUM_FILTERS / 10) {
      double newR = (leftR + rightR) / 2;
      storedHashes->setHashParameters(newR, numConcatenations,
                                      TEST_NUM_FILTERS);
      pair<double, double> newScore = runTest(
          storedHashes, filterSize, numDataPoints, numTrainPoints, groundTruth);
      if (newScore.second > TEST_NUM_FILTERS / 2) {
        rightScore = newScore;
        rightR = newR;
      } else {
        leftScore = newScore;
        leftR = newR;
      }
    }

    // Update AUC
    double oldBestAUC = bestAUC;
    if (rightScore.first > bestAUC) {
      bestAUC = rightScore.first;
      bestConcatenations = numConcatenations;
      bestR = rightR;
    } 
    if (leftScore.first > bestAUC) {
      bestAUC = leftScore.first;
      bestConcatenations = numConcatenations;
      leftR = rightR;
    } 

    // If AUC didn't improve enough, break
    if (oldBestAUC * (1 + IMPROVEMENT_PERCENT_LIMIT / 100.0) > bestAUC) {
      break;
    }
  }

  // Return best Euclidean hash function
  return make_pair(bestR, bestConcatenations);
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
    storedHashes = new EuclideanHashFunctionTraining(
        key + 1, TEST_NUM_FILTERS * MAX_CONCATENATIONS, dataDim,
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
#pragma omp parallel for
    for (size_t i = 0; i < numDataPoints; i++) {
      storedHashes->storeVal(i, dataPtr + i * dataDim);
    }
    double *trainPtr = (double *)trainingBuf.ptr;

#pragma omp parallel for
    for (size_t i = 0; i < numTrainPoints; i++) {
      storedHashes->storeVal(i + numDataPoints, trainPtr + i * dataDim);
    }

    pair<double, double> bestParams =
        trainEuclidean(storedHashes, oneFilterSize, numDataPoints,
                       numTrainPoints, (bool *)groundBuf.ptr);

    EuclideanHashFunction *trainedHash =
        new EuclideanHashFunction(bestParams.first, 42, numFilterReps, bestParams.second, dataDim);
    filter = new BloomFilter<double *>(trainedHash, oneFilterSize);
    state = 1;
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
  // py::array_t<size_t> getNumCollisionsBatch(py::array_t<double> queries) {}

private:
  double cutoff;
  size_t dataDim, oneFilterSize, numFilterReps, numDataPoints, numTrainPoints;
  int state;
  BloomFilter<double *> *filter;
  EuclideanHashFunction *filterHashes;
  EuclideanHashFunctionTraining *storedHashes;

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
