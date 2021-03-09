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
  std::string line;
  while (getline(infile, line)) {
    result.push_back(line);
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

  if (argc < 8) {
    cout << "This program runs tests of a LSBF. One need to call this program"
            "with 'runme <data_file_name> <query_file_name> <data_type> "
            "<data_dim> <num_hashes> <hash_range> <hash_multiplicity>'"
         << endl;
    cout << "\t<data_file_name>: Name of file containing the data" << endl;
    cout << "\t<queries_file_name>: Name of file containing the queries"
         << endl;
    cout << "\t<data_type>: 1 is doubles, 10 + k is strings split into kmers of of size k" << endl;
    cout << "\t<data_dim>: 1 is 1 entry per line, 2 is a vector per line "
            "(space seperated)."
         << endl;
		cout << "<num_hashes>: Number of hashes in the bloom filter" << endl;
		cout << "<hash_range>: 2^hash_range is the number of bits in the bloom filter" << endl;
		cout << "<hash_multiplicity>: Number of hashes to concatenate together in each repetition" << endl;
    exit(0);
  }

  // Set random seed
  srand(42);

  char *dataFileName = argv[1];
  char *queryFileName = argv[2];
  int dataType = stoi(argv[3]);
  int dataDim = stoi(argv[4]);
	int numHashes = stoi(argv[5]);
	size_t hashRange = 1 << stoi(argv[6]);
	int concatenationNum = stoi(argv[7]);

	printf("Threshold results for %s, statistics %d %d %d %zu %d\n", dataFileName, dataType, dataDim, numHashes, hashRange, concatenationNum); 

  if (dataType > 10 && dataDim == 1) {

    vector<string> data = parseStringData(dataFileName);

    size_t kmer = dataType - 10;
    HashFunction<string> *hashFunctions =
        new KMerStringMinHashFunction(kmer, 7, numHashes, concatenationNum);
    BloomFilter<string> minhashFilter =
        BloomFilter<string>(hashFunctions, hashRange);
    for (string item : data) {
      minhashFilter.addPoint(item);
    }

    vector<string> queries = parseStringData(queryFileName);
    for (string query : queries) {
      cout << query << " " << minhashFilter.numCollisions(query) << endl;
    }

  } else {
    cerr << "This type of data is not supported yet" << endl;
  }
}
