#include "HashFunction.hpp"
#include "MurmurHashFunction.cpp"
#include "BloomFilter.hpp"
#include <string>
#include <vector>
#include <memory>

using namespace std;

int main(int argc, char **argv) {
	size_t hashRange = 1000;
	int numHashes = 10;

  vector<HashFunction<string>*> hashFunctions;
	for (int i = 0; i < numHashes; i++) {
		hashFunctions.push_back(new MurmurHashFunction(i));
	}
	BloomFilter<string> normalFilter = BloomFilter<string>(hashFunctions, hashRange);

	normalFilter.addPoint("Jason");
	normalFilter.addPoint("Josh");
	normalFilter.addPoint("Phil");
	normalFilter.addPoint("Penny");


	cout << normalFilter.numCollisions("Jason") << endl;
	cout << normalFilter.numCollisions("Josh") << endl;
	cout << normalFilter.numCollisions("Jasonx") << endl;
	cout << normalFilter.numCollisions("Pennn") << endl;


}