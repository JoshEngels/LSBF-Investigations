#ifndef BF_H
#define BF_H

#include <vector>
#include <HashFunction.hpp>
#include <memory>

template <class T>
class BloomFilter {
public:

  // This builds a Bloom filter of hashes.size() number of hash functions, each
	// of which are of size hashRange. Note that hashes should be a vector of 
	// HashFunction objects, and all of them should ideally have range greater 
	// than or equal to hashRange, otherwise there will be wasted space inside the filter
  BloomFilter(std::vector<HashFunction<T>> hashes, size_t hashRange);

	// Add a point to the bloom filter
	// I know a char* pointer is kind of ugly but it let's you use any type you want
	// Ok for now
	void addPoint(std::vector<T> point);

	// Get number of collisions for a test point
	size_t numCollisions(std::vector<T> point);

private:
  std::vector<HashFunction<T>> hashes;
  size_t hashRange;
  size_t numHashes;
  std::shared_ptr<void> bitArray;

};

#include "BloomFilter.cpp"

#endif