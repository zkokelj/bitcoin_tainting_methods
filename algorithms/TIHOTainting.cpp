#include "AlgorithmDecl.hpp"

#include <numeric>

AlgorithmMetrics TIHOTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB)
{
    // Start measuring time
    auto startTime = std::chrono::high_resolution_clock::now();

    // Write graph to file
    std::ofstream myfile;
    if(writeToFile)
    {
        myfile.open ("./results/tiho-" + txid_n + ".txt");
    }

    // check if offchainDB exists
	std::ofstream offChainFile;
	std::string offChainFileName = "./results/_offchain_tiho-" + txid_n + ".txt";
	if(offchainDB)
	{
		offChainFile.open(offChainFileName);
	}

    uint64_t edges = 0;
    uint64_t max_edges = 1000000;
    
    //unspent values
    std::vector<uint64_t> unspentOutputValues;

    // Metrics
    AlgorithmMetrics metrics;

    // split to txid and N
    auto txidn = splitTxIDN(txid_n);

    // Data structure to keep information which transactions (and in which) order to check
    std::set<int> toCheck;

    // taintedInputs store how many satoshis are tainted in each output
    std::map<std::string, uint64_t> taintedOutputs;

    std::optional<std::string> txcFromTxid = getTxCFromTxID(db, std::get<0>(txidn));
    if(!txcFromTxid.has_value())
    {
        std::cout << "ERROR:  TxC not found for: " <<  std::get<0>(txidn) << std::endl;
        exit(1);
    }

    std::optional<std::string> txWhereSpent = getTxCWhereInputIsSpent(db, txcFromTxid.value(), std::get<1>(txidn));
    if(!txWhereSpent.has_value())
    {
        std::cout << "Tainted input not spent in our database!" << std::endl;
        exit(1);
    }

    // Starting value 
    auto startingValue = getTxOutValue(db, "o"+txcFromTxid.value()+"."+std::get<1>(txidn));
    if(!startingValue.has_value())
    {
        std::cout << "ERROR: value does not exist" << std::endl;
        exit(9);
    }

    // Write about first input to file if required..
    if(writeToFile)
    {
        myfile << txcFromTxid.value() << "," << txWhereSpent.value() << ","<< getTxOutValue(db, "o"+txcFromTxid.value()+"."+std::get<1>(txidn)).value() << std::endl;
        edges++;
    }

    // Insert next transaction in toCheck list and add output to taintedOutputs map
    toCheck.insert(std::stoi(txWhereSpent.value()));
    taintedOutputs["o"+txcFromTxid.value()+"."+std::get<1>(txidn)] = getTxOutValue(db, "o"+txcFromTxid.value()+"."+std::get<1>(txidn)).value();

    // Run while we still have transactions to check
    while(toCheck.size() > 0)
    {
        // Get fist element and remove it from toCheck
        std::string txToCheck = std::to_string(*toCheck.begin());
        toCheck.erase(toCheck.begin());

        // Get current transaction
        std::optional<DBTransaction> tx = getTx(db, txToCheck);
        assert(tx.has_value()); // we can assert here, since we checked for transaction existance before inserting in toCheck

        if(offchainDB != nullptr)
		{
			std::string val;
            //std::cout << "READ from new DB: " << tx.value().txid << std::endl;
            Status s = offchainDB->Get(ReadOptions(), tx.value().txid, &val);
            //std::cout << "Reading successful: " << val <<std::endl;
			if(s.ok())
			{
                if(!offChainFile.is_open())
                {
                    offChainFile.open(offChainFileName);
                }

				// We reached known address
				offChainFile << tx.value().txid << "," << val << std::endl;
				continue;
			}
		}
        // Count transactions for out metrics
        metrics.transactionsTainted++;

        // input taint amount
        uint64_t inputTaintAmount = 0;

        // Iterate over all inputs
        for(auto &input : tx.value().inputs)
        {
            // If input is tanted 
            if(taintedOutputs.find(input) != taintedOutputs.end())
            {
                // Add taint amount.
                inputTaintAmount += taintedOutputs.find(input)->second;
            }
        }

        // Create vector of outputs (tuples), to be able to sort it by highest first.
        std::vector<std::tuple<uint64_t, uint64_t>> vectorOfOutputs;
        
        for(int i = 0; i < tx.value().outputCount; i++)
        {
            uint64_t currentOutputValue = getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value();

            // insert in tuple <output_value, output_index>
            vectorOfOutputs.push_back(std::make_tuple(currentOutputValue, i));                
        }

        // sort tuple by the first element descending (we want to iterate from highest to lowest taint)
        std::sort(vectorOfOutputs.begin(), vectorOfOutputs.end(), std::greater<>());

        for(auto& element: vectorOfOutputs)
        {
            uint64_t value = std::get<0>(element);
            uint64_t index = std::get<1>(element);

            uint64_t outputTaint = 0;

            if(inputTaintAmount > value)
            {
                outputTaint = value;
                inputTaintAmount -= value;
            }
            else if(inputTaintAmount > 0)
            {
                outputTaint = inputTaintAmount;
                inputTaintAmount = 0;
            }


            if(outputTaint > 0)
            {
                std::optional<std::string> nextTxC = getTxCWhereInputIsSpent(db, txToCheck, std::to_string(index));
                if(nextTxC.has_value())
                {
                    //std::cout << "has value -> nextTxC: " << nextTxC.value() << std::endl;
                    // insert this output to be checked in the future
                    toCheck.insert(std::stoi(nextTxC.value()));
                    // add taint to this vector
                    taintedOutputs["o"+txToCheck+"."+std::to_string(index)] = outputTaint;

                    // Insert in file
                    if(writeToFile)
                    {
                        myfile << std::fixed << txToCheck << "," << nextTxC.value()  << "," << outputTaint << std::endl;
                        edges++;
                    }

                    if(edges > max_edges)
                    {
                        std::cout << "Edge limit exceeded!" << std::endl;
                        exit(1);
                    }

                }
                else
                {
                    // If value has taint, but is not spent in provided database
                    metrics.unspentOutputsTainted++;
                    metrics.unspentTaintedAmount+=outputTaint;
                    unspentOutputValues.push_back(value);
                }
            }
        }
    }

    auto duration =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime);
    std::cout << "duration: " << duration.count() << std::endl;

    myfile.close();
	// check if nongraph file is empty
	if(offchainDB)
	{
		if(offChainFile.is_open())
        {
            offChainFile.close();
        }

        std::ifstream file(offChainFileName);
		if(is_empty(file))
		{
			std::filesystem::remove(offChainFileName);
		}
	}

    nongraphfile << "tiho," + txid_n << "," << duration.count() << "," << metrics.unspentOutputsTainted << "," << metrics.unspentTaintedAmount << "," << startingValue.value();
    for(auto& e: unspentOutputValues)
    {
        nongraphfile << "," << e;
    }
    nongraphfile << std::endl;


    return metrics;

}