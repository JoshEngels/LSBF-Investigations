#include "HashFunction.hpp"
#include "KMerStringMinHashFunction.hpp"
#include "catch.hpp"
#include <string>

using namespace std;

SCENARIO("Min hash works as expected") {

  srand(42);

  GIVEN("A 3-mer min hash function with multiplicity 2") {
    HashFunction<string> * testHashFunction = new KMerStringMinHashFunction(3, 7, 1, 2);


    WHEN("We hash the same string twice") {
      string testString = "BlargedyBlarg";
      uint64_t hash1 = testHashFunction->getVal(testString).at(0);
      uint64_t hash2 = testHashFunction->getVal(testString).at(0);

      THEN("The hashes equal each other") { REQUIRE(hash1 == hash2); }
    }

    WHEN("When we hash two similar (Jaccard similarity = 0.8) string 500 times "
         "with multiplicity 2") {
			size_t numHashes = 500;
      HashFunction<string> * hashFunctions = new KMerStringMinHashFunction(3, 7, numHashes, 2);

      // Both strings of length 29 (so 27 k-mers each)
      // Total of 24 shared 3-mers
      // Leads to similarity of 24 / (27 + 27 - 24) = 25 / 30 = 0.8
      string string1 = "abcdefghijklmnopqrstuvwxyz123";
      string string2 = "987abcdefghijklmnopqrstuvwxyz";

      vector<uint64_t> hashes1 = hashFunctions->getVal(string1);
	    vector<uint64_t> hashes2 = hashFunctions->getVal(string2);
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