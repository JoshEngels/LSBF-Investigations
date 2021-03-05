#include "BloomFilter.hpp"
#include "DataLocations.hpp"
#include "HashFunction.hpp"
#include "MurmurHashFunction.cpp"
#include "catch.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

vector<string> parseStringData(const char *fileName) {
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

SCENARIO("Normal bloom filters (with Murmur Hash) work") {

  srand(42);

  GIVEN("The rockyou dataset and a Bloom filter with 5 hash functions and 5 * "
        "1.44 bits/item") {

    // Read in data and build filter

    vector<string> data = parseStringData(ROCK_YOU_LOCATION);
    size_t hashRange = 1.44 * data.size();
    size_t numHashes = 5;
    vector<uint32_t> keys;
    for (uint32_t i = 0; i < numHashes; i++) {
      keys.push_back(i);
    }
    HashFunction<string> *hashFunctions = new MurmurHashFunction(keys);

    BloomFilter<string> normalFilter =
        BloomFilter<string>(hashFunctions, hashRange);

    WHEN("We add the entire dataset to the bloom filter") {
      for (string item : data) {
        normalFilter.addPoint(item);
      }

      THEN("All added items should have numCollisions = 5") {
        // These should all be positive
        for (string item : data) {
          REQUIRE(normalFilter.numCollisions(item) == numHashes);
        }
      }

      THEN("Negative elements should have false positive rate < 0.65^5") {
        size_t negativeWorkedCount = 0;
        for (size_t i = 0; i < data.size(); i++) {
          string test = getRandomString(20); // Probably won't be in dataset
          if (normalFilter.numCollisions(test) == numHashes) {
            negativeWorkedCount++;
          }
        }
        REQUIRE((float)negativeWorkedCount / data.size() < pow(0.65, 5));
      }
    }
  }
}