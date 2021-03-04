#ifndef MHP_H
#define MHP_H

#include "HashFunction.hpp"
#include "MurmurHash3.hpp"
#include <string>

class MurmurHashFunction: HashFunction<std::string> {



  size_t getVal(std::string item) {
    return MurmurHash3_x64_128(item.c_str(), item.size(), )
  }
};


#endif