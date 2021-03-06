#ifndef EHFT_H
#define EHFT_H

#include "HashFunction.hpp"
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
        numDataVectors(numVectors), normalDistribution(0, 1) {

    // Generated N(0, 1) Gaussian random vectors
    // https://www.cplusplus.com/reference/random/normal_distribution/
    generator.seed(key);
    for (size_t i = 0; i < maxHashes; i++) {
      std::vector<double> nextVector;
      for (size_t d = 0; d < vectorSize; d++) {
        nextVector.push_back(normalDistribution(generator));
      }
      randomVectors.push_back(nextVector);
    }
  }

  void storeVal(size_t i, double *item) {
    std::vector<double> hashes;
    for (size_t hashNum = 0; hashNum < maxHashes; hashNum++) {
      hashes.push_back(dot(item, hashNum));
    }
    rawHashes[i] = hashes;
  }

  void setHashParameters(double r, double concatenationNum, size_t numHashes) {
    if (numHashes * concatenationNum > maxHashes) {
      std::cerr << "Not enough hashes were generated" << std::endl;
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

    std::vector<double> rawData = rawHashes.at(index);
    int transformedData[concatenationNum * numHashes];
    for (size_t i = 0; i < concatenationNum * numHashes; i++) {
      transformedData[i] = floor((rawData[i] + offsets[i]) / r);
    }

    // Generate result
    std::vector<uint64_t> result;
    for (size_t i = 0; i < numHashes; i++) {
      uint64_t hash = getMurmurHash64(transformedData + i * concatenationNum,
                                      concatenationNum * sizeof(int), i);
      result.push_back(hash);
    }
    return result;
  }

private:
  std::default_random_engine generator;
  std::vector<std::vector<double>> randomVectors;
  std::vector<std::vector<double>> rawHashes;
  size_t maxHashes, numDataVectors;
  size_t concatenationNum, numHashes;
  double r;
  std::vector<double> offsets;
  std::normal_distribution<double> normalDistribution;

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