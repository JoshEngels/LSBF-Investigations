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
  EuclideanHashFunction(double r, uint32_t key, size_t numHashes,
                        size_t concatenationNum, size_t vectorSize)
      : segments(r), concatenationNum(concatenationNum), numHashes(numHashes),
        normalDistribution(0, 1) {

    // Generated N(0, 1) Gaussian random vectors
    // https://www.cplusplus.com/reference/random/normal_distribution/
    std::default_random_engine generator;
    generator.seed(key);
    std::uniform_real_distribution<double> unif_d(0.0, segments);
    for (size_t i = 0; i < numHashes * concatenationNum; i++) {
      std::vector<double> nextVector;
      for (size_t d = 0; d < vectorSize; d++) {
        nextVector.push_back(normalDistribution(generator));
        // std::cout << nextVector[d] << " ";
      }
      // std::cout << std::endl;
      randomVectors.push_back(nextVector);
      randomOffsets.push_back(unif_d(generator));
    }
  }

  // TODO: Combine with EuclideanHashFunctionTraining code
  std::vector<uint64_t> getVal(double *item) {
    int transformedData[concatenationNum * numHashes];
    for (size_t i = 0; i < concatenationNum * numHashes; i++) {
      transformedData[i] = dot(item, i);
      // std::cout << transformedData[i] << " ";
    }
    // std::cout << std::endl;

    // Generate result
    std::vector<uint64_t> result;
    for (size_t i = 0; i < numHashes; i++) {
      uint64_t hash = getMurmurHash64(transformedData + i * concatenationNum,
                                      concatenationNum * sizeof(int), i);
      result.push_back(hash);
    }
    return result;
  }

  size_t getNumHashes() { return numHashes; }

private:
  double segments;
  size_t concatenationNum;
  size_t numHashes;
  std::vector<std::vector<double>> randomVectors;
  std::vector<double> randomOffsets;
  std::normal_distribution<double> normalDistribution;

  int dot(double *item, size_t index) {
    std::vector<double> currentVector = randomVectors.at(index);
    double total = 0;
    for (size_t i = 0; i < currentVector.size(); i++) {
      total += item[i] * currentVector[i];
    }
    // std::cout << (total + randomOffsets[index]) / segments << " ";
    return floor((total + randomOffsets[index]) / segments);
  }
};

#endif