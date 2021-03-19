#ifndef EHFT_H
#define EHFT_H

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

class EuclideanHashFunctionTraining : public HashFunction<size_t> {
public:
  EuclideanHashFunctionTraining(uint32_t key, size_t maxHashes,
                                size_t vectorSize, size_t numVectors)
      : randomVectors(0), rawHashes(numVectors), maxHashes(maxHashes),
        numDataVectors(numVectors), notAllStored(true), stored(0) {

    // Generated N(0, 1) Gaussian random vectors
    // https://www.cplusplus.com/reference/random/normal_distribution/
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

    this->r = r;
    this->concatenationNum = concatenationNum;
    this->numHashes = numHashes;

    offsets.clear();
    std::uniform_real_distribution<double> uniform(0, r);
    for (size_t i = 0; i < concatenationNum * numHashes; i++) {
      offsets.push_back(uniform(generator));
    }
  }

  size_t getNumHashes() { return numHashes; }

  std::vector<uint64_t> getVal(size_t index) {

    // Get transformed data
    std::vector<double> rawData = rawHashes.at(index);
    std::vector<int> transformedData;
    for (size_t i = 0; i < concatenationNum * numHashes; i++) {
      transformedData.push_back((rawData[i] + offsets[i]) / r);
    }

    // Generate result
    std::vector<uint64_t> result;
    for (size_t i = 0; i < numHashes; i++) {
      uint64_t hash = getMurmurHash64((char *)(&transformedData[i]),
                                      concatenationNum * sizeof(double), 42);
      result.push_back(hash);
    }
    return result;
  }

private:
  std::default_random_engine generator;
  std::vector<std::vector<double>> randomVectors;
  std::vector<std::vector<double>> rawHashes;
  size_t maxHashes, numDataVectors;
  bool notAllStored;
  std::unordered_set<size_t> stored;

  double r, concatenationNum, numHashes;
  std::vector<double> offsets;

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