#ifndef BF_C
#define BF_C

#include "BloomFilter.hpp"

using namespace std;

template <typename T>
BloomFilter<T>::BloomFilter(vector<HashFunction<T>*> hashes, size_t hashRange) {
  this->hashes = hashes;
  this->hashRange = hashRange;
  this->numHashes = hashes.size();
  this->bitArray = shared_ptr<char>((char*)calloc((hashRange * numHashes + 7) / 8, 1), free); 
}

template <typename T>
void BloomFilter<T>::addPoint(T point) {
  for (size_t i = 0; i < numHashes; i++) {
    setBit(i, (*hashes.at(i)).getVal(point));
  }
}

template <typename T>
size_t BloomFilter<T>::numCollisions(T point) {
  size_t count = 0;
  for (size_t i = 0; i < numHashes; i++) {
    count += getBit(i, (*hashes.at(i)).getVal(point));
  }
  return count;
}

template <typename T>
BloomFilter<T>::~BloomFilter() {
	for (size_t i = 0; i < numHashes; i++) {
		delete hashes.at(i);
	}
}

#endif