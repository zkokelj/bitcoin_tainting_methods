#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <regex>
#include <thread>

// RocksDB files
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

// My header files
#include "Block.hpp"
#include "DBInsert.hpp"
#include "DBStructs.hpp"

using namespace ROCKSDB_NAMESPACE;

// Counters for all inserted key-value types
uint64_t DBBlocksCount = 0;
uint64_t DBTransactionsCount = 0;
uint64_t DBTxOutputsCount = 0;
uint64_t DBWhereSpentCount = 0;

// Transaction counter
uint64_t txCounter = 0;


std::string blklocation = "";
std::string kDBPath = "";
uint64_t toBlock = 9999999999;
uint64_t fromBlock = 0;
std::string txcDBName = "";
bool createTxcDb = false;

// Min and Max txc that we want in our database
int minTxC = 0;
int maxTxC = 0;

uint64_t highestBlockSeen = 0;

void InsertData(Block& b, DB* db, Status& s, DB* db2, Status& s2, bool createtxcdb)
{
    // Avoid inserting data above selected, also we don't have to do anything about the furure...
    if(b.blockHeight > toBlock)
    {
        return;
    }

    // Type0 means that we want to fill second database and just know how many tx
    if(createTxcDb)
    {
        s2 = db2->Put(WriteOptions(), std::to_string(b.blockHeight), std::to_string(txCounter));
        assert(s2.ok());
        txCounter += b.txCount;
        highestBlockSeen = b.blockHeight;
        return;
    }

    // If we are in this part of code it means that type of inserting == 1
    
    // Batch write data for whole block to reduce number of calls to the database
    WriteBatch batch;
    // Insert block only if it is in our range!
    if(b.blockHeight >=  fromBlock)
    {
        // Insert block data (DBBLock)
        std::string blockHash = getHexStringFromRaw(b.hash, 32, true);
        DBBlocks blocktmp(b.blockHeight, getHexStringFromRaw(b.prevBlock, 32, true));
        std::string blockKey = "b" + blockHash;
        std::stringstream ostmp;
        {
            cereal::BinaryOutputArchive archive_out(ostmp);
            archive_out(CEREAL_NVP(blocktmp));
        }
        std::string insert_str_blk = ostmp.str();
        batch.Put(blockKey, insert_str_blk);
        DBBlocksCount++;
    }
    
    // Loop over transactions
    bool isCoinbase = true;
    for(auto tx: b.txs)
    {
        std::string currentTxC = std::to_string(txCounter++);

        // Insert Outputs (DBOutputs)
        // It is not possible to know at the time of inserting, if we would need this output sometimes in the future.
        // We have to insert Outputs and later delete them as they are used in inputs (each output can be used only once) -> delete only if txC is below minTxC.
        for(int i = 0; i < tx.outputsCounter; i++)
        {
            DBOutputs tmp(tx.outputs[i].value, tx.outputs[i].scriptLen);
            std::string txOutKey = "o"+currentTxC+"."+std::to_string(i); // KEY: "o"+TxC+.+N ; VALUE: DBOutputs
            std::stringstream os;
            {
                cereal::BinaryOutputArchive archive_out(os);
                archive_out(CEREAL_NVP(tmp));
            }
            std::string insert_str = os.str();
            batch.Put(txOutKey, insert_str);
            DBTxOutputsCount++;
        }

        std::vector<std::string> transactionInputs;
        int start = 0;
        if(isCoinbase)
        {
            start=1;
            isCoinbase=false;
        }

        // We have to TxID -> TxC to database immediately, because we need it
        // later in transactions, when we reference transaction from the same block.
        s = db->Put(WriteOptions(), tx.txid, currentTxC);
        assert(s.ok());
        

        for(int i = start; i < tx.inputsCounter; i++)
        {
            // everything is normal if currentTxC is between minTxC and maxTxC
            if(std::stoi(currentTxC) >= minTxC)
            {
                // Get TxC where transaction input was created
                std::string txCInput;
                std::string prevTxID = getHexStringFromRaw(tx.inputs[i].prevTx, 32, true);
                if(prevTxID == tx.txid)
                {
                    txCInput = currentTxC;
                }
                else
                {
                    Status s = db->Get(ReadOptions(), prevTxID, &txCInput);
                    if(!s.ok())
                    {
                        std::cout << "Transaction referenced in input, but not available in the database: " << getHexStringFromRaw(tx.inputs[i].prevTx, 32, true) << std::endl;
                        std::cout << "Block height: " << b.blockHeight << std::endl;
                        std::cout << "Current tx: " << tx.txid << "; Referenced tx: " << getHexStringFromRaw(tx.inputs[i].prevTx, 32, true) << std::endl;
                        exit(1);
                    }
                }

                std::string inputKey = "i" + txCInput + "." + std::to_string(tx.inputs[i].nTxOut);
                batch.Put(inputKey, currentTxC);
                DBWhereSpentCount++;

                transactionInputs.emplace_back("o" + txCInput + "." + std::to_string(tx.inputs[i].nTxOut));
            }
            else
            {
                // delete all outputs that are consumed in the database
                std::string prevTxID = getHexStringFromRaw(tx.inputs[i].prevTx, 32, true);
                batch.Delete("o"+prevTxID+"."+std::to_string(tx.inputs[i].nTxOut));
            }
        }

        // everything is normal if currentTxC is between minTxC and maxTxC
        if(std::stoi(currentTxC) >= minTxC)
        {
            // Insert transaction
            DBTransaction txval(tx.txid ,tx.outputsCounter, getHexStringFromRaw(b.hash, 32, true), b.blockHeight, transactionInputs);
            std::stringstream os_tx;
            {
                cereal::BinaryOutputArchive archive_out(os_tx);
                archive_out(cereal::make_nvp("tx", txval));
            }
            std::string insert_str_tx = os_tx.str();
            batch.Put(currentTxC, insert_str_tx);
            DBTransactionsCount++;
        }
    }
    s = db->Write(WriteOptions(), &batch); 
    assert(s.ok());
}

int main(int argc, char* argv[])
{
    auto startMain = std::chrono::high_resolution_clock::now();
    // Parse parameters to get .blk source files location and location of RocksDB database
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--datadir") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                blklocation = argv[++i];
                std::cout << "Location of block files: " << blklocation << std::endl;
            } else {
                  std::cerr << "--datadir option requires one argument." << std::endl;
                return 1;
            }  
        }

        if (std::string(argv[i]) == "--dbdir") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                kDBPath = argv[++i];
            } else {
                  std::cerr << "--dbdir option requires one argument." << std::endl;
                return 1;
            }  
        }

        if (std::string(argv[i]) == "--fromblock") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                fromBlock = std::stoi(argv[++i]);
            } else {
                  std::cerr << "--fromblock option requires one argument." << std::endl;
                return 1;
            }
        }

        if (std::string(argv[i]) == "--toblock") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                toBlock = std::stoi(argv[++i]);
            } else {
                  std::cerr << "--toblock option requires one argument." << std::endl;
                return 1;
            }  
        }

        if (std::string(argv[i]) == "--txcdbname") {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                txcDBName = argv[++i];
            } else {
                  std::cerr << "--txcdbname option requires one argument." << std::endl;
                return 1;
            }  
        }

        if (std::string(argv[i]) == "--ctxdb") {
            createTxcDb = true;
        }
    }

    if (blklocation == "")
    {
        std::cerr << "Missing or misussed argument --blkloc. It should point to directory containing .dat file of the Bitcoin blokchain." << std::endl;
        exit(1);
    }

    if (kDBPath == "")
    {
        std::cout << "Missing or misussed argument --dbloc. Default used: `/tmp/rockdsb_bitcoin`" << std::endl;
        kDBPath = "/tmp/rocksdb_bitcoin";
    }

    if (txcDBName == "")
    {
        std::cerr << "Missing or misussed argument --txcDBName. It should contain name of already created db or db to be created." << std::endl;
        exit(1);
    }

    std::cout << "START: " << std::endl;
    std::cout << "blk files source directory: " << blklocation << std::endl;
    std::cout << "DB location:                " << kDBPath << std::endl;
    std::cout << "From block:                 " << fromBlock << std::endl;
    std::cout << "To block:                   " << toBlock << std::endl;
    std::cout << "TxCDBName:                  " << txcDBName << std::endl;
    std::cout << "createtxcdb:                " << std::to_string(createTxcDb) << std::endl;


    // Open and set options for RocksDB key-value database
    DB* db;
    Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;

    Status s = DB::Open(options, kDBPath, &db);
    assert(s.ok());

    // Open second database that holds numbers of transactions per block and helps with partial histroy insert
    DB* db2;
    Options options2;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options2.IncreaseParallelism();
    options2.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options2.create_if_missing = true;
    Status s2 = DB::Open(options2, txcDBName, &db2);
    std::cout << s2.ToString() << std::endl;
    assert(s2.ok());

    std::vector<std::string> blk_files;

    // Get all block files in specified folder (format: blkXXXXX.dat)
    for (const auto & entry : std::filesystem::directory_iterator(blklocation))
    {
        std::string s = entry.path();
        if (std::regex_search (s, std::regex("(blk[0-9]{5}.dat)")))
        {
            blk_files.push_back(s);
        }
    }

    // Sort files -> we want to process them in alphabetical order
    std::sort(blk_files.begin(), blk_files.end(),compareFunction);
        
    // reset global variables
    txCounter = 0;
    DBBlocksCount = 0;
    DBTransactionsCount = 0;
    DBTxOutputsCount = 0;
    DBWhereSpentCount = 0;

    if(!createTxcDb)
    {
        std::string minTxCStr = "";
        std::string maxTxCStr = "";

        s2 = db2->Get(ReadOptions(), std::to_string(fromBlock), &minTxCStr);
        assert(s2.ok());

        s2 = db2->Get(ReadOptions(), std::to_string(toBlock), &maxTxCStr);
        assert(s2.ok());

        // Get min and max tx that we want in our DB.
        minTxC = std::stoi(minTxCStr);
        maxTxC = std::stoi(maxTxCStr);

        // Print min and max block and TxC
        std::cout << "from block:   " << fromBlock << std::endl;
        std::cout << "to block:     " << toBlock << std::endl;
        std::cout << "minTxC:       " << minTxCStr << std::endl;
        std::cout << "maxTxC:       " << maxTxCStr << std::endl;
    }

    // Block in queue if they are not in the correct order...
    std::vector<Block> outOfOrderBlocks;

    // Height and prevHashGlobal at the beginning of the Bitcoin blockchain
    uint8_t prevBlockGlobal[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int32_t heightGlobal = 0;

    // Process blk files in correct order
    for (std::string file_name : blk_files)
    {
        // We also want to measure how much time it took us to process blocks
        auto start = std::chrono::high_resolution_clock::now();

        std::ifstream input(file_name, std::ios::binary);
        
        // Start processing blk file block by block ...
        while(!input.eof() && (toBlock > heightGlobal))
        {
            Block block(input, createTxcDb);

            if(!input.fail())
            {
                if(std::equal(std::begin(block.prevBlock),std::end(block.prevBlock), std::begin(prevBlockGlobal)))
                {
                    block.blockHeight = heightGlobal;
                    memcpy(prevBlockGlobal, block.hash, 32);
                    heightGlobal++;

                    // std::cout << "Inserting block: " << getHexStringFromRaw(block.hash, 32, true) << std::endl;
                    // std::cout << "Txs: " << block.txs.size() << std::endl;
                    InsertData(block, db, s, db2, s2, createTxcDb);

                }else{
                    outOfOrderBlocks.push_back(block);

                    // Try to match all out of order blocks that we can. We don't expect out of order to be very high..
                    // if this assumption is wrong I need to change this algorithm for matching...
                    int startSize = 0;
                    int endSize = 0;
                    do {
                        startSize = outOfOrderBlocks.size();
                        for(auto i=0; i < outOfOrderBlocks.size(); i++)
                        {
                            if(std::equal(std::begin(outOfOrderBlocks[i].prevBlock),std::end(outOfOrderBlocks[i].prevBlock), std::begin(prevBlockGlobal)))
                            {
                                outOfOrderBlocks[i].blockHeight = heightGlobal;
                                memcpy(prevBlockGlobal, outOfOrderBlocks[i].hash, 32);
                                heightGlobal++;
                                InsertData(outOfOrderBlocks[i], db, s, db2, s2, createTxcDb);
                                //std::cout << "Txs: " << outOfOrderBlocks[i].txs.size() << std::endl;
                                outOfOrderBlocks.erase(outOfOrderBlocks.begin()+i);

                                break;
                            }
                        }
                        endSize = outOfOrderBlocks.size();
                        
                    }while(startSize > endSize);
                }    
            }
        }

        input.close();

        auto stop = std::chrono::high_resolution_clock::now(); 
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
    
        std::cout << "Reading file: " << file_name << " took: " 
            << duration.count()/1000000 << " seconds" << std::endl; 
        std::cout << "Blocks inserted until height: " << heightGlobal << std::endl;

    }

    std::cout << "Finished parsing and inserting in dababase! \n Statistics: " << std::endl;
    std::cout << "DBBlocks:         " << DBBlocksCount << std::endl;
    std::cout << "DBTransactions:   " << DBTransactionsCount << std::endl;
    std::cout << "DBTxOutputs:      " << DBTxOutputsCount << std::endl;
    std::cout << "DBWhereSpentCount:" << DBWhereSpentCount << std::endl;
    
    auto stopMain = std::chrono::high_resolution_clock::now(); 
    auto durationMain = std::chrono::duration_cast<std::chrono::microseconds>(stopMain - startMain); 
    std::cout << "Duration for parsing and inserting in DB is: " << durationMain.count()/1000000 << std::endl;
    

    return 0;
}
