#include "EuclideanHashFunction.hpp"
#include "KMerStringMinHashFunction.hpp"
#include "catch.hpp"
#include <string>

using namespace std;

// TODO: Test with exact collision probabilities and variances?
SCENARIO("Euclidean hash works as expected") {

  srand(42);

  GIVEN("A Euclidean hash function with multiplicity 10 and r = 10") {
    HashFunction<vector<float>> *testHashFunction =
        new EuclideanHashFunction(10, 7, 1, 10, 3);

    WHEN("We hash the same vector twice") {
      vector<float> testVec;
      testVec.push_back(1);
      testVec.push_back(3.7777);
      testVec.push_back(-100.0);
      uint64_t hash1 = testHashFunction->getVal(testVec).at(0);
      uint64_t hash2 = testHashFunction->getVal(testVec).at(0);
      THEN("The hashes equal each other") { REQUIRE(hash1 == hash2); }
    }

    WHEN("We hash pairs of similar and dissimilar vectors") {
      size_t numHashes = 50;
      HashFunction<vector<float>> *hashFunctions =
          new EuclideanHashFunction(10, 7, numHashes, 10, 3);

      vector<float> simVector1, simVector2, difVector;
      simVector1.push_back(1);
      simVector1.push_back(1);
      simVector1.push_back(2);
      simVector2.push_back(2);
      simVector2.push_back(1);
      simVector2.push_back(1);
      difVector.push_back(-1);
      difVector.push_back(-1);
      difVector.push_back(-1);

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

        // cout << simCount << " " << difCount1 << " " << difCount2 << endl;
        REQUIRE(simCount > difCount1);
        REQUIRE(simCount > difCount2);
      }
    }
  }
}