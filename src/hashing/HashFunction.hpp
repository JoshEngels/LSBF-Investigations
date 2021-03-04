#ifndef HF_H
#define HF_H

#include <cstdint>
#include <iostream>
#include <MurmurHash3.hpp>

template <class T>
class HashFunction {
public:
  virtual uint64_t getVal(T item){
		std::cerr << "This (generic HashFunction getVal) should not get called!\n"; 
		exit(1);
	};

	virtual ~HashFunction() {
		// No op
	}

protected:
	// This function bijectively maps the two input 64 bit integers to a 128 bit 
	// integer and then returns the result modulo 2^64 (as a 64 bit integer)
	uint64_t bijectiveMap(uint64_t a, uint64_t b) {
			// This will overflow but it's fine since wrap around should just be a modulo
			// See https://stackoverflow.com/questions/18195715/why-is-unsigned-integer-overflow-defined-behavior
			// See https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
			return a >= b ? a * a + a + b : a + b * b;
	}

	// Get a 64 bit murmur hash (optimized for 64 bit cpu) for a given pointer and key
	uint64_t getMurmurHash64(const char *pointer, size_t length, int key) {
			uint64_t hashes[2];
			MurmurHash3_x64_128(pointer, length, key, &hashes[0]);
			return bijectiveMap(hashes[0], hashes[1]);
	}

};



#endif