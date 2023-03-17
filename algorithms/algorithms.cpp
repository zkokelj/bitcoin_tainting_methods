#include <iostream>
#include <tuple>
#include <optional>
#include <chrono>
#include <deque>
#include <set>
#include <algorithm>

// Include things related to DB, serialization, etc.
#include "DBStructs.hpp"
#include "misc.hpp"

// Include out implementation of algorithms
#include "AlgorithmDecl.hpp"

std::string DBLocation = "/tmp/rocksdb_bitcoin";

int main(int argc, char *argv[])
{
    // help message if not enough arguments are provided
    if (argc < 3)
    {
        std::cout << "Not enough arguments provided! Please check the code" << std::endl;
    }

    // Parse command line arguments
    std::string algorithm;
    std::string tainted_txid_n;
    double haircut;
    for (int i = 1; i < argc; ++i)
    {
        // which database to use (provide absolute path)
        if (std::string(argv[i]) == "-db")
        {
            if (i + 1 < argc)
            { // Make sure we aren't at the end of argv!
                DBLocation = argv[++i];
                // std::cout << "Location of block files: " << DBLocation << std::endl;
            }
            else
            {
                std::cerr << "-db option requires one argument." << std::endl;
                return 1;
            }
        }

        // which algorithm to use ("p" / "h" / "fifo" / "tiho" / ... )
        if (std::string(argv[i]) == "-a")
        {
            if (i + 1 < argc)
            { // Make sure we aren't at the end of argv!
                algorithm = argv[++i];
                // std::cout << "Algorithm choosen: " << algorithm << std::endl;
            }
            else
            {
                std::cerr << "-a option requires one argument." << std::endl;
                return 1;
            }
        }

        // which output is tanited (txid+n)
        if (std::string(argv[i]) == "-t")
        {
            if (i + 1 < argc)
            { // Make sure we aren't at the end of argv!
                tainted_txid_n = argv[++i];
                // std::cout << "Tainted txid + n: " << tainted_txid_n << std::endl;
            }
            else
            {
                std::cerr << "-t option requires one argument." << std::endl;
                return 1;
            }
        }

        if (std::string(argv[i]) == "-h")
        {
            if (i + 1 < argc)
            { // Make sure we aren't at the end of argv!
                haircut = std::stod(argv[++i]);
                // std::cout << "Tainted txid + n: " << tainted_txid_n << std::endl;
            }
            else
            {
                std::cerr << "-h option requires one argument." << std::endl;
                return 1;
            }
        }
    }

    // Open and set options for RocksDB key-value database
    DB *db;
    Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();

    Status s = DB::Open(options, DBLocation, &db);
    assert(s.ok());

    bool includeOffChainData = true;
    std::string DBLocationOffchain = ""; // TODO: Enter off chain data location
    DB *offchaindb = nullptr;
    Options optionsOffchain;

    if (includeOffChainData)
    {
        Status sOffchain = DB::Open(optionsOffchain, DBLocationOffchain, &offchaindb);
        assert(sOffchain.ok());
        std::cout << "Offchain data ok" << offchaindb << std::endl;
        std::string value123 = "";
        offchaindb->Get(ReadOptions(), "DDCCFC89FC225C4218A8B58CA403D724DFF60013AEA79B54EDFC1342A86259235", &value123);
        std::cout << "Value is: " << value123 << std::endl;
    }

    if (offchaindb)
    {
        std::cout << "Pointer OK" << std::endl;
    }

    if (includeOffChainData && offchaindb == nullptr)
    {
        std::cout << "ERROR with offchain database" << std::endl;
        return 743;
    }

    // Create file for non-graph metrics
    std::ofstream nongraphfile;
    nongraphfile.open("./results/_metrics.csv", std::ios_base::app);

    auto start = std::chrono::high_resolution_clock::now();
    // std::cout << "Running: " << algorithm << " with tainted input: " << tainted_txid_n << "... ";
    if (algorithm == "poison")
    {
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        auto a = PoisonTainting(db, tainted_txid_n, true, nongraphfile, offchaindb);
    }
    else if (algorithm == "haircut")
    {
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        auto a = HaircutTainting(db, tainted_txid_n, haircut, true, nongraphfile, offchaindb);
    }
    else if (algorithm == "fifo")
    {
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        auto a = FIFOTainting(db, tainted_txid_n, true, nongraphfile, offchaindb);
    }
    else if (algorithm == "lifo")
    {
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        auto a = LIFOTainting(db, tainted_txid_n, true, nongraphfile, offchaindb);
    }
    else if (algorithm == "tiho")
    {
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        auto a = TIHOTainting(db, tainted_txid_n, true, nongraphfile, offchaindb);
    }
    else if (algorithm == "comb")
    {
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        auto a = COMBTainting(db, tainted_txid_n, nongraphfile, offchaindb);
    }
    else if (algorithm == "test")
    {
        std::cout << "TEST" << std::endl;
        std::transform(tainted_txid_n.begin(), tainted_txid_n.end(), tainted_txid_n.begin(), ::toupper);
        TestDatabase(db, "550000", "689999", "randomTransactionOutputs3.csv");
    }
    else if (algorithm == "insert-offchain")
    {
        std::cout << "Creating DB..." << std::endl;
        DB *db_offchain;
        Options options_offchain;
        // create the DB if it's not already present
        options_offchain.create_if_missing = true;

        Status s_offchain = rocksdb::DB::Open(options_offchain, DBLocationOffchain, &db_offchain);
        assert(s_offchain.ok());

        std::cout << "Starting inserting offchain data  " << std::endl;
        std::string offchain_filename = ""; // TODO: Add offchain data location
        CreateOffChainDB(db_offchain, offchain_filename);
        delete db_offchain;
    }
    else
    {
        std::cout << "ERROR: Unknown algorithm: " << algorithm << "." << std::endl;
        return 1;
    }
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
    // std::cout << "duration: " << (float)duration.count()/1000000 << "s" << std::endl;

    delete db;
    delete offchaindb;
}