#ifndef MHP_H
#define MHP_H

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <string>
#include <cstdint>

class MurmurHashFunction: public HashFunction<std::string> {
public:
  MurmurHashFunction(int key) {
    this->key = key;
  }

  uint64_t getVal(std::string item) {
    uint64_t hashes[2];
    MurmurHash3_x64_128(item.c_str(), item.size(), key, &hashes[0]);
    // This will overflow but it's fine
    // See https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
    uint64_t a = hashes[0];
    uint64_t b = hashes[1];
    return a >= b ? a * a + a + b : a + b * b;
  }

private:
  int key;
};


#endif