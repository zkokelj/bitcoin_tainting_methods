#include <iostream>
#include <tuple> 
#include <optional>
#include <chrono>
#include <deque>
#include <set>
#include <algorithm>

#include "misc.hpp"


std::optional<std::string> getTxCFromTxID(ROCKSDB_NAMESPACE::DB* db, std::string txid)
{
    std::string txcValue;
    Status s = db->Get(ReadOptions(), txid, &txcValue);
    if(s.ok())
    {
        return txcValue;
    }
    return std::nullopt;
}

std::optional<std::string> getTxCWhereInputIsSpent(ROCKSDB_NAMESPACE::DB* db, std::string txc, std::string n)
{
    std::string txcValue;
    Status s = db->Get(ReadOptions(), "i" + txc + "." + n, &txcValue);
    if(s.ok())
    {
        return txcValue;
    }
    return std::nullopt;
}

std::optional<uint64_t> getTxOutValue(ROCKSDB_NAMESPACE::DB* db, std::string key)
{
    std::string value;
    Status s = db->Get(ReadOptions(), key, &value);
    if(s.ok())
    {
        DBOutputs dbo;
        std::stringstream is(value);
        cereal::BinaryInputArchive archive_in(is);
        archive_in(dbo);
        return dbo.value;
    }
    return std::nullopt;
}

std::optional<DBTransaction> getTx(ROCKSDB_NAMESPACE::DB* db, std::string txc)
{
    std::string value;
    Status s = db->Get(ReadOptions(), txc, &value);
    if(s.ok())
    {
        DBTransaction dbt;
        std::stringstream is(value);
        cereal::BinaryInputArchive archive_in(is);
        archive_in(dbt);
        return dbt;
    }else
    {
        std::cout << s.getState() << std::endl;
    }
    return std::nullopt;

}

std::optional<DBBlocks> getBlock(ROCKSDB_NAMESPACE::DB* db, std::string blockhash)
{
    std::string value;
    Status s = db->Get(ReadOptions(), std::string("b"+blockhash), &value);
    if(s.ok())
    {
        DBBlocks dbb;
        std::stringstream is(value);
        cereal::BinaryInputArchive archive_in(is);
        archive_in(dbb);
        return dbb;
    }
    return std::nullopt;
}

AlgorithmMetrics::AlgorithmMetrics()
{
    transactionsTainted = 0;
    unspentOutputsTainted = 0;
    unspentTaintedAmount = 0;
};

TaintVector::TaintVector()
{
    taintVector = {};
}
std::deque<uint64_t>& TaintVector::getTaintVector()
{
    return taintVector;
}

void TaintVector::addTaintedNum(uint64_t n)
{
    int vecSize = taintVector.size();
    if(vecSize % 2 == 1)
    {
        // last element is clean (if it is 0, we can add taint to previous)
        // otherwise add tainted to the vector
        if(taintVector[vecSize-1] == 0 && vecSize >= 2)
        {
            taintVector[vecSize-2] += n;
        }
        else
        {
            taintVector.push_back(n);
        }
    }
    else
    {
        if(vecSize == 0)
        {
            taintVector.push_back(0);
            taintVector.push_back(n);
        }
        else
        {
            // last element is tainted (add new taint to it's value)
            taintVector[vecSize-1] += n;  
        }
    }
}

void TaintVector::addCleanNum(uint64_t n)
{
    int vecSize = taintVector.size();
    if(vecSize % 2 == 1)
    {
        // last element is clean
        taintVector[vecSize-1] += n;
    }
    else
    {
        if(vecSize == 0)
        {
            taintVector.push_back(n);
            return;
        }
        // last element is tainted
        if(taintVector[vecSize-1] == 0 && vecSize >= 2)
        {
            taintVector[vecSize-2] += n;
        }
        else
        {
            taintVector.push_back(n);
        }
    }
}

void TaintVector::appendNum(uint64_t n)
{
    taintVector.push_back(n);
}

void TaintVector::addTaintVector(TaintVector& tv)
{
    auto& tvv = tv.getTaintVector();
    int vecSize = taintVector.size();
    if(vecSize%2 == 1)
    {
        // last element is clean and first element of tv is clean
        taintVector[vecSize-1] += tvv[0];
        taintVector.insert(taintVector.end(), tvv.begin()+1, tvv.end());
    }
    else
    {
        taintVector.insert(taintVector.end(), tvv.begin(), tvv.end());
    }
}

uint64_t TaintVector::getTaintValue()
{
    uint64_t taint = 0;
    for(auto i=1; i < taintVector.size(); i+=2)
    {
        taint+=taintVector[i];
    }
    return taint;
}

uint64_t TaintVector::sumVector()
{
    // std::accumulate(taintVector.begin(), taintVector.end(), 0)
    uint64_t sum = 0;
    for(uint64_t& e : taintVector)
        sum += e;
    return sum;
}
bool TaintVector::containsTaint()
{
    for(auto i=1; i < taintVector.size(); i+=2)
    {
        if(taintVector[i] > 0)
        {
            return true;
        }
    }
    return false;
}

void TaintVector::printVector()
{
    for (int i = taintVector.size() - 1; i >= 0; i--) {
        std::cout << taintVector[i] <<  ", ";
    }
    std::cout << std::endl;
}

TaintVector TaintVector::assingTaintVectorToOutput(uint64_t txOutValue)
{
    uint64_t vectorSum = this->sumVector();
    if(txOutValue > vectorSum)
    {
        std::cout << "ERROR: Output value is larger than vector sum! " << std::endl;
        std::cout << "taint vector sum: " << vectorSum << std::endl;
        std::cout << "value for output: " << txOutValue << std::endl;
        exit(1);
    }
    TaintVector outputTaintVector;

    // counters for number of elements added and amount of bitcoins added
    int elementsAdded = 0;
    uint64_t sumOfAdded = 0;

    // look how many elements we have to move to the output vector
    for(auto const& val: taintVector)
    {
        if(sumOfAdded + val <= txOutValue)
        {
            sumOfAdded += val;
            outputTaintVector.appendNum(val);
            elementsAdded++;
        }
        else
        {
            break;
        }
    }

    // remove elements from the taintVector
    taintVector.erase(taintVector.begin(), taintVector.begin()+elementsAdded);

    // calculate if we have to divide some of the elements
    uint64_t diff = txOutValue - sumOfAdded;
    if(diff > 0)
    {
        // append num to the outputVector and subtract value from the first element
        outputTaintVector.appendNum(diff);
        taintVector[0] -= diff;
    }

    // check if we had to add additional 0 at the beginning of taintVector to avoid shifting taint/clean
    if(elementsAdded%2 == 1)
    {
        taintVector.push_front(0);
    }

    // REMOVE: (Only for debugging purposes)
    if(outputTaintVector.sumVector() + this->sumVector() != vectorSum)
    {
        std::cout << "ERROR in assingTaintVectorToOutput function" << std::endl;
        std::cout << "Output taint vector : " << outputTaintVector.sumVector() << std::endl;
        std::cout << "Taint vector        : " << this->sumVector() << std::endl;
        std::cout << "Value               : " << txOutValue << std::endl;
        std::cout << "---------------" << std::endl;
        outputTaintVector.printVector();
        this->printVector();
        exit(1);

    }

    return outputTaintVector;
}


std::tuple<std::string, std::string> splitTxIDN(std::string txidn)
{
    if(txidn.length() < 65)
    {
        std::cout << "ERROR: " << txidn << " length is too small!" << std::endl;
        std::cout << txidn << std::endl;
        std::cout << txidn.length() << std::endl;
        exit(1);
    }
    return {txidn.substr(0, 64), txidn.substr(64)};
}

bool is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}