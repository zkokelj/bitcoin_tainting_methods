#include <iostream>

#include "Block.hpp"
#include "Misc.hpp"

void Block::PrintMe()
{
    std::cout << "Print block info:" << std::endl;
    std::cout << "PrevBlock: " << getHexStringFromRaw(this->prevBlock, 32, true) << std::endl;
    std::cout << "MerkleRoot: " << getHexStringFromRaw(this->merkleRoot, 32, true) << std::endl;
    std::cout << "TxCount: " << this->txCount << std::endl;
    std::cout << "Hash: " << getHexStringFromRaw(this->hash, 32, true) << std::endl;
}

Block::Block(std::istream& input, bool skipDetails)
{
    // Read magic bytes & size
    //input.read((char*)(&magicBytes), 4);
    input.ignore(4); // ignore magic bytes, because we don't need them
    input.read((char*)(&size), 4);
    
    // Read block header (80 bytes)
    input.read((char *)(&(this->version)), 4);
    input.read((char *)(this->prevBlock), 32);
    input.read((char *)(this->merkleRoot), 32); 
    input.read((char *)(&(this->timestamp)), 4);
    input.read((char *)(&(this->bits)), 4);
    input.read((char *)(&(this->nonce)), 4);

    // calculate hash
    // FIXME: Move this calculation to another function!
    std::vector<uint8_t> tmpHeader;

    uint8_t *pointer = (uint8_t *)&this->version;
    for (auto i = 0; i < 4; i++) {
        tmpHeader.push_back(*pointer);
        pointer++;
    }

    pointer = (uint8_t *)&this->prevBlock;
    for (auto i = 0; i < 32; i++) {
        tmpHeader.push_back(*pointer);
        pointer++;
    }

    pointer = (uint8_t *)&this->merkleRoot;
    for (auto i = 0; i < 32; i++) {
        tmpHeader.push_back(*pointer);
        pointer++;
    }

    pointer = (uint8_t *)&this->timestamp;
    for (auto i = 0; i < 4; i++) {
        tmpHeader.push_back(*pointer);
        pointer++;
    }

    pointer = (uint8_t *)&this->bits;
    for (auto i = 0; i < 4; i++) {
        tmpHeader.push_back(*pointer);
        pointer++;
    }

    pointer = (uint8_t *)&this->nonce;
    for (auto i = 0; i < 4; i++) {
        tmpHeader.push_back(*pointer);
        pointer++;
    }

    picosha2::hash256_one_by_one hasher;
    hasher.process(tmpHeader.begin(), tmpHeader.end());
    hasher.finish();

    std::vector<uint8_t> hash(32);
    hasher.get_hash_bytes(hash.begin(), hash.end());

    //Second SHA-256
    picosha2::hash256_one_by_one hasher2;

    hasher2.process(hash.begin(), hash.end());
    hasher2.finish();

    std::vector<uint8_t> hash2(32);
    hasher2.get_hash_bytes(hash2.begin(), hash2.end());

    // Write hash to the block.
    for(int i = 0; i < hash2.size(); i++)
    {
        this->hash[i] = static_cast<uint8_t>(hash2.at(i));
    }

    // Read varint to get txCount
    this->txCount = varintDecode(input);

    // std::cout << "After block header " << std::endl;
    // Read transactions and save them to 
    for(auto i = 0; i < this->txCount; i++)
    {
        Tx currentTx = Tx(input, skipDetails);
        if(!skipDetails)
        {
            txs.push_back(currentTx);
        }
    }

    // set block height to default value
    this->blockHeight = 0;
}
