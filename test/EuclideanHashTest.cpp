#include "EuclideanHashFunction.hpp"
#include "KMerStringMinHashFunction.hpp"
#include "catch.hpp"
#include <string>

using namespace std;

// TODO: Test with exact collision probabilities
SCENARIO("Euclidean hash works as expected") {

  srand(42);

  GIVEN("A Euclidean hash function with multiplicity 10 and r = 10") {
    HashFunction<double *> *testHashFunction =
        new EuclideanHashFunction(10, 7, 1, 10, 3);

    WHEN("We hash the same vector twice") {
      double testVec[3] = {1, 3.777, -100};
      uint64_t hash1 = testHashFunction->getVal(testVec).at(0);
      uint64_t hash2 = testHashFunction->getVal(testVec).at(0);
      THEN("The hashes equal each other") { REQUIRE(hash1 == hash2); }
    }

    WHEN("We hash pairs of similar and dissimilar vectors") {
      size_t numHashes = 10000;
      HashFunction<double *> *hashFunctions =
          new EuclideanHashFunction(5, 7, numHashes, 4, 3);

      double simVector1[3] = {1, 1, 2};
      double simVector2[3] = {2, 2, 1};
      double difVector[3] = {4, -1, -1};

      vector<uint64_t> hashes1 = hashFunctions->getVal(simVector1);
      vector<uint64_t> hashes2 = hashFunctions->getVal(simVector2);
      vector<uint64_t> hashes3 = hashFunctions->getVal(difVector);

      THEN("More similar vectors have more collisions") {
        uint32_t simCount = 0;
        for (uint32_t index = 0; index < numHashes; index++) {
          simCount += hashes1.at(index) == hashes2.at(index);
        }
        uint32_t difCount1 = 0;
        for (uint32_t index = 0; index < numHashes; index++) {
          difCount1 += hashes2.at(index) == hashes3.at(index);
        }
        uint32_t difCount2 = 0;
        for (uint32_t index = 0; index < numHashes; index++) {
          difCount2 += hashes1.at(index) == hashes3.at(index);
        }

        cout << simCount << " " << difCount1 << " " << difCount2 << endl;
        REQUIRE(simCount > difCount1);
        REQUIRE(simCount > difCount2);
        REQUIRE(difCount1 > difCount2);
      }
    }
  }
}