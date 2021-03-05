#ifndef MHP_C
#define MHP_C

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <cstdint>
#include <string>
#include <vector>

class MurmurHashFunction : public HashFunction<std::string> {
public:
  MurmurHashFunction(std::vector<uint32_t> keys):
		keys(keys) {}

  std::vector<uint64_t> getVal(std::string item) {
		std::vector<uint64_t> result;
		for (uint32_t key : keys) {
			result.push_back(getMurmurHash64(item.c_str(), item.size(), key));
		}
		return result;
  }

	size_t getNumHashes() {
		return keys.size();
	}

private:
  std::vector<uint32_t> keys;
};

#endif