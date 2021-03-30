#ifndef BF_C
#define BF_C

#include "BloomFilter.hpp"
#include <cstring>

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

template <typename T> void BloomFilter<T>::addPoints(vector<T> points) {
  vector<vector<uint64_t>> stored = vector<vector<uint64_t>>(points.size());
#pragma omp parallel for
  for (size_t i = 0; i < points.size(); i++) {
    stored.at(i) = hashes->getVal(points.at(i));
  }

#pragma omp parallel for
  for (size_t i = 0; i < numHashes; i++) {
    for (size_t point = 0; point < points.size(); point++) {
      setBit(i, stored.at(point).at(i));
    }
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

template <typename T> size_t BloomFilter<T>::numOnes() {
  size_t count = 0;
  for (size_t h = 0; h < numHashes; h++) {
    for (size_t i = 0; i < hashRange; i++) {
      count += getBit(h, i);
    }
  }
  return count;
}

#endif