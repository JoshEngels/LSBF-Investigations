#include "BloomFilter.hpp"
#include "EuclideanHashFunction.hpp"
#include "HashFunction.hpp"
#include "KMerStringMinHashFunction.hpp"
#include "MurmurHashFunction.cpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
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

vector<vector<float>> parseFloatVectorData(char *fileName) {
  vector<vector<float>> result;
  ifstream infile(fileName);
  std::string line;
  while (getline(infile, line)) {
    std::stringstream stringStream;
    stringStream.str(line);
    vector<float> lineFloats;
    float f;
    while (stringStream >> f) {
      lineFloats.push_back(f);
    }
    result.push_back(lineFloats);
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

  // TODO: Change input to list of values for each data type
  // TODO: Center vector data?
  if (argc < 9) {
    cout << "This program runs tests of a LSBF. One need to call this program"
            "with 'runme <data_file_name> <query_file_name> <data_type> "
            "<data_dim> <num_hashes> <hash_range> <hash_multiplicity> "
            "<extra_hash_info>'"
         << endl;
    cout << "\t<data_file_name>: Name of file containing the data" << endl;
    cout << "\t<queries_file_name>: Name of file containing the queries"
         << endl;
    cout << "\t<data_type>: 1 is doubles and angular distance, 2 is doubles "
            "and euclidean distance, 3 is strings"
         << endl;
    cout << "\t<data_dim>: 1 is 1 entry per line, 2 is a vector per line "
            "(space seperated)."
         << endl;
    cout << "\t<num_hashes>: Number of hashes in the bloom filter" << endl;
    cout << "\t<hash_range>: 2^hash_range is the number of bits in the bloom "
            "filter"
         << endl;
    cout << "\t<hash_multiplicity>: Number of hashes to concatenate together "
            "in each repetition"
         << endl;
    cout << "\t<extra_hash_info>: Extra hyperparameter for hash function, if "
            "string data this is kmer size, if euclidean distance this is r"
         << endl;
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
  int extra_hash_info = stoi(argv[8]);

  printf("Threshold results for %s, statistics %d %d %d %zu %d %d\n",
         dataFileName, dataType, dataDim, numHashes, hashRange,
         concatenationNum, extra_hash_info);

  if (dataType == 3 && dataDim == 1) {

    vector<string> data = parseStringData(dataFileName);

    HashFunction<string> *hashFunctions = new KMerStringMinHashFunction(
        extra_hash_info, 7, numHashes, concatenationNum);
    BloomFilter<string> minhashFilter =
        BloomFilter<string>(hashFunctions, hashRange);
    for (string item : data) {
      minhashFilter.addPoint(item);
    }

    vector<string> queries = parseStringData(queryFileName);
    for (string query : queries) {
      cout << query << " " << minhashFilter.numCollisions(query) << endl;
    }

  } else if (dataType == 2 && dataDim == 2) {
    vector<vector<float>> data = parseFloatVectorData(dataFileName);
    HashFunction<vector<float>> *hashFunctions = new EuclideanHashFunction(
        extra_hash_info, 7, numHashes, concatenationNum, data.at(0).size());
    (void)hashFunctions;

    BloomFilter<vector<float>> minhashFilter =
        BloomFilter<vector<float>>(hashFunctions, hashRange);
    size_t i = 0;
    for (vector<float> item : data) {
      minhashFilter.addPoint(item);
      i++;
    }

    vector<vector<float>> queries = parseFloatVectorData(queryFileName);
    for (vector<float> query : queries) {
      cout << minhashFilter.numCollisions(query) << endl;
    }

  } else {
    cerr << "This type of data is not supported yet" << endl;
  }
}
