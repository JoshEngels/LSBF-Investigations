#ifndef EHF_H
#define EHF_H

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

class EuclideanHashFunction : public HashFunction<std::vector<float>> {
public:
  EuclideanHashFunction(size_t r, uint32_t key, size_t numHashes,
                        size_t concatenationNum, size_t vectorSize)
      : segments(r), concatenationNum(concatenationNum), numHashes(numHashes) {

    // Generated N(0, 1) Gaussian random vectors
    // https://www.cplusplus.com/reference/random/normal_distribution/
    std::default_random_engine generator;
    generator.seed(key);
    std::normal_distribution<double> distribution(0, 1);
    for (size_t i = 0; i < numHashes * concatenationNum; i++) {
      std::vector<float> nextVector;
      for (size_t d = 0; d < vectorSize; d++) {
        nextVector.push_back(distribution(generator));
      }
      randomVectors.push_back(nextVector);
    }
  }

  std::vector<uint64_t> getVal(std::vector<float> item) {
    std::vector<uint64_t> result(numHashes, 0);
    for (size_t hash = 0; hash < numHashes; hash++) {
      uint64_t cumulativeHash = 0;
      for (size_t i = 0; i < concatenationNum; i++) {
        cumulativeHash = bijectiveMap(dot(item, i), cumulativeHash);
      }
      result.push_back(cumulativeHash);
    }
    return result;
  }

  size_t getNumHashes() { return numHashes; }

private:
  size_t segments;
  size_t concatenationNum;
  size_t numHashes;
  std::vector<std::vector<float>> randomVectors;
  std::vector<float> randomOffsets;

  uint64_t dot(std::vector<float> item, size_t index) {
    std::vector<float> currentVector = randomVectors.at(index);
    float currentOffset = randomOffsets.at(index);
    float total = 0;
    for (size_t i = 0; i < item.size(); i++) {
      total += item.at(i) * currentVector.at(i);
    }
    total /= segments;
    total += 1 << 63;

    // Make sure it's going to fit in 64 bits
    if (total < 0 || total >= UINT64_MAX) {
      std::cerr << "Euclidean hash cannot fit in 64 bits" << std::endl;
      exit(1);
    }

    return (uint64_t)total;
  }
};

#endif