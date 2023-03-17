#pragma once

#include <iostream>
#include <tuple> 
#include <optional>
#include <chrono>
#include <deque>
#include <set>
#include <algorithm>
#include <fstream>
#include <tuple>
#include <filesystem>

// RocksDB files
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "DBStructs.hpp"

using namespace ROCKSDB_NAMESPACE;

std::optional<std::string> getTxCFromTxID(ROCKSDB_NAMESPACE::DB* db, std::string txid);

std::optional<std::string> getTxCWhereInputIsSpent(ROCKSDB_NAMESPACE::DB* db, std::string txc, std::string n);

std::optional<uint64_t> getTxOutValue(ROCKSDB_NAMESPACE::DB* db, std::string key);

std::optional<DBTransaction> getTx(ROCKSDB_NAMESPACE::DB* db, std::string txc);

std::optional<DBBlocks> getBlock(ROCKSDB_NAMESPACE::DB* db, std::string blockhash);

std::tuple<std::string, std::string> splitTxIDN(std::string txidn);

struct AlgorithmMetrics
{
    uint64_t transactionsTainted;
    uint64_t unspentOutputsTainted;
    uint64_t unspentTaintedAmount;

    AlgorithmMetrics();
};

class TaintVector
{
private:
    std::deque<uint64_t> taintVector;
public:
    TaintVector();
    std::deque<uint64_t>& getTaintVector();

    void addTaintedNum(uint64_t n);

    void addCleanNum(uint64_t n);

    void appendNum(uint64_t n);

    void addTaintVector(TaintVector& tv);

    uint64_t getTaintValue();

    uint64_t sumVector();
    bool containsTaint();

    void printVector();

    TaintVector assingTaintVectorToOutput(uint64_t txOutValue);
};

bool is_empty(std::ifstream& pFile);