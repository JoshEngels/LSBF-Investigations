#include "BloomFilter.hpp"
#include "HashFunction.hpp"
#include "KMerStringMinHashFunction.hpp"
#include "MurmurHashFunction.cpp"
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

  if (argc < 4) {
    cout << "This program runs tests of a LSBF. One need to call this program"
            "with 'runme <file_name> <data_type> <data_dim> <hash_type>'"
         << endl;
    cout << "\t<file_name>: Name of file containing the data." << endl;
    cout << "\t<data_type>: 1 is strings, 2 is doubles." << endl;
    cout << "\t<data_dim>: 1 is 1 entry per line, 2 is a vector per line "
            "(space seperated)."
         << endl;
    exit(0);
  }

  // Set random seed
  srand(42);

  char *fileName = argv[1];
  int dataType = stoi(argv[2]);
  int dataDim = stoi(argv[3]);

  if (dataType == 1 && dataDim == 1) {
    // Read in data
    vector<string> data = parseStringData(fileName);
    printf("Data (%zu strings) read in...\n", data.size());
    cout << "Starting test of LSH Bloom filter of string data (using trimers "
            "and minhash)"
         << endl;
    size_t hashRange = 1 << 15;
    size_t numHashes = 500;
    size_t concatenationNum = 5;
    vector<HashFunction<string> *> hashFunctions;
    for (size_t i = 0; i < numHashes; i++) {
      hashFunctions.push_back(
          new KMerStringMinHashFunction(3, i, concatenationNum));
    }
    BloomFilter<string> minhashFilter =
        BloomFilter<string>(hashFunctions, hashRange);
    for (string item : data) {
      minhashFilter.addPoint(item);
    }
    printf("Bloom filter built (size %zu MB), testing positive elements...\n",
           hashRange * numHashes / 8000000);

  } else {
    cerr << "This type of data is not supported yet" << endl;
  }
}