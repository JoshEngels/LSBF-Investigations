#ifndef EHF_H
#define EHF_H

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

class EuclideanHashFunction : public HashFunction<double *> {
public:
  EuclideanHashFunction(size_t r, uint32_t key, size_t numHashes,
                        size_t concatenationNum, size_t vectorSize)
      : segments(r), concatenationNum(concatenationNum), numHashes(numHashes) {

    // Generated N(0, 1) Gaussian random vectors
    // https://www.cplusplus.com/reference/random/normal_distribution/
    std::default_random_engine generator;
    generator.seed(key);
    std::normal_distribution<double> norm_d(0, 1);
    std::uniform_real_distribution<double> unif_d(0.0, segments);
    for (size_t i = 0; i < numHashes * concatenationNum; i++) {
      std::vector<double> nextVector;
      for (size_t d = 0; d < vectorSize; d++) {
        nextVector.push_back(norm_d(generator));
      }
      randomVectors.push_back(nextVector);
      randomOffsets.push_back(unif_d(generator));
    }
  }

  // TODO: Combine with EuclideanHashFunctionTraining code
  std::vector<uint64_t> getVal(double *item) {
    std::vector<int> transformedData;
    for (size_t i = 0; i < concatenationNum * numHashes; i++) {
      transformedData.push_back(dot(item, i));
    }

    // Generate result
    std::vector<uint64_t> result;
    for (size_t i = 0; i < numHashes; i++) {
      uint64_t hash = getMurmurHash64((char *)(&transformedData[i]),
                                      concatenationNum * sizeof(int), 42);
      result.push_back(hash);
    }
    return result;
  }

  size_t getNumHashes() { return numHashes; }

private:
  size_t segments;
  size_t concatenationNum;
  size_t numHashes;
  std::vector<std::vector<double>> randomVectors;
  std::vector<double> randomOffsets;

  int dot(double *item, size_t index) {
    std::vector<double> currentVector = randomVectors.at(index);
    double total = 0;
    for (size_t i = 0; i < currentVector.size(); i++) {
      total += item[i] * currentVector.at(i);
    }
    return (total + randomOffsets[index]) / segments;
  }
};

#endif