#pragma once

#include <vector>

#include "Misc.hpp"

class TxIn
{

public:
    uint8_t prevTx[32];
    uint32_t nTxOut;
    uint64_t scriptLen;
    std::vector<uint8_t> scriptSig;
    uint32_t sequence;

    TxIn(std::istream & input);
};


class TxOut
{
public:
    uint64_t value;
    uint64_t scriptLen;
    std::vector<uint8_t> script;
    
    TxOut(std::istream & input);
};


class Tx
{
public:
    uint32_t version;
    uint64_t inputsCounter;
    std::vector<TxIn> inputs;
    uint64_t outputsCounter;
    std::vector<TxOut> outputs;
    uint32_t lockTime;
    std::string txid;
    
    Tx(std::istream & input, bool skipDetails = false);
};  