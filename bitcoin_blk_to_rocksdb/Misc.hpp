#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// RocksDB files
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "picosha2.h"

using namespace ROCKSDB_NAMESPACE;

uint64_t varintDecode(std::istream &input);

void print256(uint8_t* arr, int size);

std::string getHexStringFromInt32t(uint32_t);

std::string getHexFromArray(std::vector<uint8_t>& data, int size);

void swapEndianness(std::vector<uint8_t>& data);

bool compareFunction (std::string a, std::string b);

std::string getHexStringFromRaw(uint8_t* arr, int size, bool swap);

void insertInDatabase(DB* database, Options& options, std::string dbpath);