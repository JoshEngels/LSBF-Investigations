#ifndef EHF_H
#define EHF_H

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

class EuclideanHashFunctionTraining {
public:
  EuclideanHashFunctionTraining(uint32_t key, size_t maxHashes,
                                size_t vectorSize, size_t numVectors)
      : randomVectors(0), rawHashes(numVectors), maxHashes(maxHashes),
        numDataVectors(numVectors), notAllStored(true), stored(0) {

    // Generated N(0, 1) Gaussian random vectors
    // https://www.cplusplus.com/reference/random/normal_distribution/
    std::default_random_engine generator;
    generator.seed(key);
    std::normal_distribution<double> norm_d(0, 1);
    for (size_t i = 0; i < maxHashes; i++) {
      std::vector<double> nextVector;
      for (size_t d = 0; d < vectorSize; d++) {
        nextVector.push_back(norm_d(generator));
      }
      randomVectors.push_back(nextVector);
    }
  }

  void storeVal(size_t i, double *item) {
    std::vector<double> hashes;
    for (size_t hashNum = 0; hashNum < maxHashes; hashNum++) {
      hashes.push_back(dot(item, hashNum));
    }
    rawHashes.at(i) = hashes;
    stored.insert(i);
  }

  void setHashParameters(double r, double concatenationNum, size_t numHashes) {
    if (numHashes * concatenationNum > maxHashes) {
      std::cerr << "Not enough hashes were generated" << std::endl;
      exit(1);
    }
    if (stored.size() != numDataVectors) {
      std::cerr << "Some hashes were not stored" << std::endl;
      exit(1);
    }
  }

private:
  std::vector<std::vector<double>> randomVectors;
  std::vector<std::vector<double>> rawHashes;
  size_t maxHashes, numDataVectors;
  bool notAllStored;
  std::unordered_set<size_t> stored;

  double dot(double *item, size_t index) {
    std::vector<double> currentVector = randomVectors.at(index);
    double total = 0;
    for (size_t i = 0; i < currentVector.size(); i++) {
      total += item[i] * currentVector.at(i);
    }
    return total;
  }
};

#endif