#include "HashFunction.hpp"
#include "MurmurHashFunction.cpp"
#include <string>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
  vector<HashFunction<string>> hashFunctions;
	for (int i = 0; i < 10; i++) {
		hashFunctions.push_back(MurmurHashFunction(i));
	}

}