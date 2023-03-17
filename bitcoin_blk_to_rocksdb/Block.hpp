#pragma once

#include <fstream>
#include <vector>

#include "Misc.hpp"
#include "Transaction.hpp"

class Block
{
    public:
        uint32_t size;
        
        // Block Header
        uint32_t magicBytes;
        uint32_t version; 

        uint8_t prevBlock[32];
        uint8_t merkleRoot[32];
        uint32_t timestamp, bits, nonce;
        uint8_t hash[32];

        // Height (obtained by prevBlock)
        uint64_t blockHeight;

        // Transactions
        uint64_t txCount;
        std::vector<Tx> txs;

        Block(std::istream& input, bool skipDetails);

        void PrintMe();
};