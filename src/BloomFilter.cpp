#ifndef BF_C
#define BF_C

#include "BloomFilter.hpp"

using namespace std;
template <typename T>

BloomFilter<T>::BloomFilter(vector<HashFunction<T>> hashes, size_t hashRange) {
  this->hashes = hashes;
  this->hashRange = hashRange;
  this->numHashes = hashes.size();
  this->bitArray = shared_ptr<void>(calloc((hashRange * numHashes + 7) / 8, 1), free); 
}

#endif