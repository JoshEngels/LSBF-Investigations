#include "BloomFilter.hpp"
#include "HashFunction.hpp"
#include "MurmurHashFunction.cpp"
#include "KMerStringMinHashFunction.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

vector<string> parseStringData(char *fileName) {
  // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
  vector<string> result;
  ifstream infile(fileName);
  string nextString;
  while (infile >> nextString) {
    result.push_back(nextString);
  }
  return result;
}

string getRandomString(size_t length) {
  // https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
  string result;
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";

  result.reserve(length);
  for (size_t i = 0; i < length; ++i)
    result += alphanum[rand() % (sizeof(alphanum) - 1)];

  return result;
}

int main(int argc, char **argv) {

  if (argc < 5) {
    cout << "Need to call this program with 'runme <file_name> <data_type> "
            "<data_dim> <hash_type>'"
         << endl;
    cout << "\t<file_name>: Name of file containing the data." << endl;
    cout << "\t<data_type>: 1 is strings, 2 is doubles." << endl;
    cout << "\t<data_dim>: 1 is 1 entry per line, 2 is a vector per line "
            "(space seperated)."
         << endl;
    cout << "\t<hash_type>: 1 is random hashes (creates a bloom filter), 2 is "
            "lsh"
         << endl;
    exit(0);
  }

  // Set random seed
  srand(42);

  char *fileName = argv[1];
  int dataType = stoi(argv[2]);
  int dataDim = stoi(argv[3]);
  int hashType = stoi(argv[4]);

  if (dataType == 1 && dataDim == 1) {
    // Read in data
    vector<string> data = parseStringData(fileName);
    printf("Data (%zu strings) read in...\n", data.size());

		if (hashType == 1) {
    	cout << "Starting test of normal Bloom filter of string data" << endl;
			size_t hashRange = 1.44 * data.size();
			size_t numHashes = 5;
			vector<HashFunction<string> *> hashFunctions;
			for (size_t i = 0; i < numHashes; i++) {
				hashFunctions.push_back(new MurmurHashFunction(i));
			}
			BloomFilter<string> normalFilter =
					BloomFilter<string>(hashFunctions, hashRange);
			for (string item : data) {
				normalFilter.addPoint(item);
			}
		
			// TODO: Write test method
			printf("Bloom filter built (size %zu MB), testing positive elements...\n",
						hashRange * numHashes / 8000000);

			// These should all be positive
			for (string item : data) {
				if (normalFilter.numCollisions(item) != numHashes) {
					cerr << "Bloom filter doesn't work for string " << item << endl;
					exit(1);
				}
			}
			cout << "All positive elements work, testing negative elements..." << endl;

			// Test some negative items (number of tests = size of data)
			size_t negativeWorkedCount = 0;
			for (size_t i = 0; i < data.size(); i++) {
				string test = getRandomString(20); // Probably won't be in dataset
				if (normalFilter.numCollisions(test) == numHashes) {
					negativeWorkedCount++;
				}
			}
			printf("Negative tests done, false positive rate is %f%%\n",
						(float)negativeWorkedCount / data.size());
		} else {
    	cout << "Starting test of LSH Bloom filter of string data (using bimers and minhash)" << endl;
			size_t hashRange = 1 << 15;
			size_t numHashes = 500;
			size_t concatenationNum = 5;
			vector<HashFunction<string> *> hashFunctions;
			for (size_t i = 0; i < numHashes; i++) {
				hashFunctions.push_back(new KMerStringMinHashFunction(2, i, concatenationNum));
			}
			BloomFilter<string> minhashFilter =
					BloomFilter<string>(hashFunctions, hashRange);
			for (string item : data) {
				minhashFilter.addPoint(item);
			}
			printf("Bloom filter built (size %zu MB), testing positive elements...\n",
						hashRange * numHashes / 8000000);

			
		}
  } else {
    cerr << "This type of data is not supported yet" << endl;
  }
}