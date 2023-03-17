#pragma once

// Cereal files
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/concepts/pair_associative_container.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/access.hpp>

struct DBOutputs
{
    template<class Archive>
    void serialize(Archive & ar)
     {
         ar(value, scriptLen);
     }

    DBOutputs(uint64_t avalue, uint64_t asciptLen)
    {
        value = avalue;
        scriptLen = asciptLen;
    }

    DBOutputs() = default;
    uint64_t value;
    uint64_t scriptLen;
    
};

struct DBBlocks
{
    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(height, previousBlockHash);
    }

    DBBlocks(uint64_t aheight, std::string apreviousBlockHash)
    {
        height = aheight;
        previousBlockHash = apreviousBlockHash;
    }

    DBBlocks() = default;
    uint64_t height;
    std::string previousBlockHash;
};

struct DBTransaction
{
    template<class Archive>
    void serialize(Archive& ar)
    {
        ar(cereal::make_nvp("txid", txid), cereal::make_nvp("outputCount", outputCount), cereal::make_nvp("blockHash", blockHash), 
            cereal::make_nvp("blockHeight", blockHeight), cereal::make_nvp("inputs", inputs));
    }

    DBTransaction(std::string atxid, uint64_t aoutput_count, std::string ablockHash, uint64_t ablockHeight, std::vector<std::string> ainputs)
    {
        txid = atxid;
        outputCount = aoutput_count;
        blockHash = ablockHash;
        blockHeight = ablockHeight;
        inputs = ainputs;
    }

    DBTransaction() = default;

    std::string txid;
    uint64_t outputCount;
    std::string blockHash;
    uint64_t blockHeight;
    std::vector<std::string> inputs;
};