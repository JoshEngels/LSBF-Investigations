#ifndef KSMHP_C
#define KSMHP_C

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <string>
#include <cstdint>
#include <algorithm> 
#include <vector>

class KMerStringMinHashFunction: public HashFunction<std::string> {
public:
  KMerStringMinHashFunction(size_t kmerSize, uint32_t key, size_t multiplicity):
		keys(0),
		kmerSize(kmerSize)
	 {
		keys.push_back(key);
		for (size_t i = 1; i < multiplicity; i++) {
			uint32_t nextKey;
			MurmurHash3_x86_32(&keys.at(i - 1), 4, key, &nextKey);
			keys.push_back(nextKey);
		}
  }

  uint64_t getVal(std::string item) {
		uint64_t combinedHash = 0;
		for (int key : keys) {
			uint64_t smallestHash = UINT64_MAX;
			for (size_t start = 0; start < item.size() - kmerSize + 1; start++) {
				uint64_t hash = getMurmurHash64(item.c_str() + start, kmerSize, key);
				smallestHash = std::min(smallestHash, hash);
			}
			combinedHash = bijectiveMap(combinedHash, smallestHash);
		}
		return combinedHash;
  }

private:
  std::vector<uint32_t> keys;
	size_t kmerSize;
};


#endif