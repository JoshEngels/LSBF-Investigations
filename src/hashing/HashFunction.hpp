#ifndef HF_H
#define HF_H

#include <cstdint>
#include <iostream>

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

};



#endif