#include "HashFunction.hpp"
#include "KMerStringMinHashFunction.hpp"
#include "catch.hpp"
#include <string>

using namespace std;

SCENARIO("Min hash works as expected") {

  srand(42);

  GIVEN("A 3-mer min hash function with multiplicity 2") {
    HashFunction<string> *testHashFunction =
        new KMerStringMinHashFunction(3, 42, 2);

    WHEN("We hash the same string twice") {
      string testString = "BlargedyBlarg";
      uint64_t hash1 = testHashFunction->getVal(testString);
      uint64_t hash2 = testHashFunction->getVal(testString);

      THEN("The hashes equal each other") { REQUIRE(hash1 == hash2); }
    }

    WHEN("When we hash two similar (Jaccard similarity = 0.8) string 100 times "
         "with multiplicity 2") {
      vector<HashFunction<string> *> hashFunctions;
      uint32_t numHashes = 100;
      for (uint32_t i = 0; i < numHashes; i++) {
        hashFunctions.push_back(new KMerStringMinHashFunction(3, i, 2));
      }
      // Both strings of length 29 (so 27 k-mers each)
      // Total of 24 shared 3-mers
      // Leads to similarity of 24 / (27 + 27 - 24) = 25 / 30 = 0.8
      string string1 = "abcdefghijklmnopqrstuvwxyz123";
      string string2 = "987abcdefghijklmnopqrstuvwxyz";

      vector<uint64_t> hashes1;
      vector<uint64_t> hashes2;
      for (HashFunction<string> *func : hashFunctions) {
        hashes1.push_back(func->getVal(string1));
        hashes2.push_back(func->getVal(string2));
      }
      THEN("More than 75%*75% and less than 85%*85% of the hashes equal each "
           "other") {
        double bottomBound = 0.75 * 0.75;
        double topBound = 0.85 * 0.85;
        uint32_t count = 0;
        for (uint32_t index = 0; index < numHashes; index++) {
          count += hashes1.at(index) == hashes2.at(index);
        }
        REQUIRE(count > bottomBound * numHashes);
        REQUIRE(count < topBound * numHashes);
      }
    }
  }
}