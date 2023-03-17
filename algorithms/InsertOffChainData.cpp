#include "AlgorithmDecl.hpp"

void CreateOffChainDB(DB* db, std::string fileName)
{
    std::ifstream myFile(fileName);

    // to be sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    std::string line;
    while(std::getline(myFile, line))
    {
        //std::stringstream ss(line);
        if(line.size() > 65)
        {
            std::string txid = line.substr(0,64);
            std::transform(txid.begin(), txid.end(),txid.begin(), ::toupper);
            std::cout << txid << std::endl;
            Status s = db->Put(WriteOptions(), txid, line.substr(65));
        }
    }

    std::string exchange;
    Status s = db->Get(ReadOptions(), "8C75F64EC1AB3017B611E91B7AD86837724CEE9ADA3F94883978F2DA1C977FDD", &exchange);
    if(s.ok())
    {
        std::cout << "Exchange: " << exchange << std::endl;
    }
   

    std::cout << "TEST... inserting the db.." << std::endl;
}


