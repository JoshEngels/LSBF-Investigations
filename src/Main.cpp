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
            "with 'runme <data_file_name> <query_file_name> <data_type> <data_dim>'"
         << endl;
    cout << "\t<data_file_name>: Name of file containing the data." << endl;
    cout << "\t<queries_file_name>: Name of file containing the queries." << endl;
    cout << "\t<data_type>: 1 is strings, 2 is doubles." << endl;
    cout << "\t<data_dim>: 1 is 1 entry per line, 2 is a vector per line "
            "(space seperated)."
         << endl;
    exit(0);
  }

  // Set random seed
  srand(42);

  char *dataFileName = argv[1];
  char *queryFileName = argv[2];
  int dataType = stoi(argv[3]);
  int dataDim = stoi(argv[4]);

  if (dataType == 1 && dataDim == 1) {
    // cout << "Starting test of LSH Bloom filter of string data (using trimers "
    //         "and minhash)"
    //      << endl;
    vector<string> data = parseStringData(dataFileName);

    size_t hashRange = 1 << 25;
    size_t numHashes = 20;
    size_t concatenationNum = 3;
		size_t kmer = 3;
    HashFunction<string> * hashFunctions = new KMerStringMinHashFunction(kmer, 7, numHashes, concatenationNum);
    BloomFilter<string> minhashFilter =
        BloomFilter<string>(hashFunctions, hashRange);
		size_t i = 0;
    for (string item : data) {
			i++;
			if (i % 10000 == 0) {
				cout << i * 100 / data.size() << endl;
			}
      minhashFilter.addPoint(item);
    }
		cout << "Done adding items" << endl;

		vector<string> queries = parseStringData(queryFileName);
		for (string query : queries) {
			cout << minhashFilter.numCollisions(query) << endl;
		}

  } else {
    cerr << "This type of data is not supported yet" << endl;
  }
}