#ifndef KSMHP_C
#define KSMHP_C

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

class KMerStringMinHashFunction : public HashFunction<std::string> {
public:
  KMerStringMinHashFunction(size_t kmerSize, uint32_t key, size_t numHashes,
                            size_t minHashesPerHash)
      : keys(0), kmerSize(kmerSize), minHashesPerHash(minHashesPerHash),
        numHashes(numHashes) {
    keys.push_back(key);
    for (size_t i = 1; i < numHashes * minHashesPerHash; i++) {
      uint32_t nextKey;
      MurmurHash3_x86_32(&keys.at(i - 1), 4, key, &nextKey);
      keys.push_back(nextKey);
    }
  }

  std::vector<uint64_t> getVal(std::string item) {

    std::vector<uint64_t> result(numHashes, 0);
    if (item.size() < kmerSize) {
      return result;
    }

    for (size_t hashNum = 0; hashNum < numHashes; hashNum++) {
      uint64_t combinedHash = 0;
      for (size_t hashPart = 0; hashPart < minHashesPerHash; hashPart++) {
        uint64_t smallestHash = UINT64_MAX;
        for (size_t start = 0; start < item.size() - kmerSize + 1; start++) {
          uint64_t hash =
              getMurmurHash64(item.c_str() + start, kmerSize,
                              keys.at(hashNum * minHashesPerHash + hashPart));
          smallestHash = std::min(smallestHash, hash);
        }
        combinedHash = bijectiveMap(combinedHash, smallestHash);
      }
      result.at(hashNum) = combinedHash;
    }
    return result;
  }

  size_t getNumHashes() { return numHashes; }

private:
  std::vector<uint32_t> keys;
  size_t kmerSize;
  size_t minHashesPerHash;
  size_t numHashes;
};

#endif