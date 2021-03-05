#ifndef BF_H
#define BF_H

#include <HashFunction.hpp>
#include <memory>
#include <vector>

template <class T> class BloomFilter {
public:
  // This builds a Bloom filter of hashes.size() number of hash functions, each
  // of which are of size hashRange. Note that hashes should be a vector of
  // HashFunction objects, and all of them should ideally have range greater
  // than or equal to hashRange, otherwise there will be wasted space inside the
  // filter
	BloomFilter(HashFunction<T> * hashes, size_t hashRange);

  // Add a point to the bloom filter
  void addPoint(T point);

  // Get number of collisions for a test point
  size_t numCollisions(T point);

  // Destructor deletes vector of hash functions too
  ~BloomFilter();

private:
  HashFunction<T> * hashes;
  size_t hashRange;
  size_t numHashes;
  std::shared_ptr<char> bitArray;

  // Set a bit in the bloom filter
  void setBit(size_t hashIndex, size_t hashVal) {
    size_t index = hashIndex * hashRange + hashVal % hashRange;
    bitArray.get()[index / 8] |= (1 << (index % 8));
  }

  // Get a bit in the bloom filter
  bool getBit(size_t hashIndex, size_t hashVal) {
    size_t index = hashIndex * hashRange + hashVal % hashRange;
    return bitArray.get()[index / 8] & (1 << (index % 8));
  }
};

#include "BloomFilter.cpp"

#endif