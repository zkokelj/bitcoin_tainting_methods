#pragma once

#include "misc.hpp"

AlgorithmMetrics PoisonTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB);

AlgorithmMetrics HaircutTainting(DB* db, std::string txid_n, double threshold, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB);

AlgorithmMetrics FIFOTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB);

AlgorithmMetrics LIFOTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB);

AlgorithmMetrics TIHOTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB);

AlgorithmMetrics COMBTainting(DB* db, std::string txid_n, std::ofstream& nongraphfile, DB* offchainDB);

void TestDatabase(DB* db, std::string startBlock, std::string endBlock, std::string fileName);

void CreateOffChainDB(DB* db, std::string fileName);