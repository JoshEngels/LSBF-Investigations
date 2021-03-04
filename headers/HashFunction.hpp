#ifndef HF_H
#define HF_H

#include <cstdint>
#include <iostream>

template <class T>
class HashFunction {

  virtual uint64_t getVal(T item){
		std::cerr << "This should next get called"; 
		exit(1);
	};

};



#endif