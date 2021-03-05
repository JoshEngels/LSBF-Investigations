#ifndef MHP_C
#define MHP_C

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <cstdint>
#include <string>

class MurmurHashFunction : public HashFunction<std::string> {
public:
  MurmurHashFunction(uint32_t key) { this->key = key; }

  uint64_t getVal(std::string item) {
    return getMurmurHash64(item.c_str(), item.size(), key);
  }

private:
  uint32_t key;
};

#endif