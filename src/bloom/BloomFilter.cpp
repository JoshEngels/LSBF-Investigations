#ifndef BF_C
#define BF_C

#include "BloomFilter.hpp"

using namespace std;

template <typename T>
BloomFilter<T>::BloomFilter(HashFunction<T> *hashes, size_t hashRange) {
  this->hashes = hashes;
  this->hashRange = hashRange;
  this->numHashes = hashes->getNumHashes();
  this->bitArray = shared_ptr<char>(
      (char *)calloc((hashRange * numHashes + 7) / 8, 1), free);
}

template <typename T> void BloomFilter<T>::addPoint(T point) {
  vector<uint64_t> hashList = hashes->getVal(point);
  for (size_t i = 0; i < numHashes; i++) {
    setBit(i, hashList.at(i));
  }
}

template <typename T> size_t BloomFilter<T>::numCollisions(T point) {
  vector<uint64_t> hashList = hashes->getVal(point);
  size_t count = 0;
  for (size_t i = 0; i < numHashes; i++) {
    count += getBit(i, hashList.at(i));
  }
  return count;
}

template <typename T> BloomFilter<T>::~BloomFilter() { delete hashes; }

#endif