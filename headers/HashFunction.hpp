#ifndef HF_H
#define HF_H

#include <cstdint>

template <class T>
class HashFunction {

  virtual uint64_t getVal(T item);

};



#endif