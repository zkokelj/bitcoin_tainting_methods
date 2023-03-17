#include "AlgorithmDecl.hpp"

#include <deque>
#include <numeric>

AlgorithmMetrics LIFOTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB)
{
    // Start measuring time
    auto startTime = std::chrono::high_resolution_clock::now();
   
    // unspent values
    std::vector<uint64_t> unspentOutputValues;

    // Write graph to file
    std::ofstream myfile;

    if(writeToFile)
    {
        myfile.open ("./results/lifo-" + txid_n + ".txt");
    }

    // check if offchainDB existsž
	std::ofstream offChainFile;
	std::string offChainFileName = "./results/_offchain_lifo-" + txid_n + ".txt";

    uint64_t edges = 0;
    uint64_t max_edges = 1000000;

    // Metrics
    AlgorithmMetrics metrics;

    // split to txid and N
    auto txidn = splitTxIDN(txid_n);

    // Data structure to keep information which transactions (and in which) order to check
    std::set<int> toCheck;

    // taintedInputs store TaintVector for each txidn
    std::map<std::string, TaintVector> taintedOutputs;

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

    // Insert next transaction in toCheck list
    toCheck.insert(std::stoi(txWhereSpent.value()));

    // Write about first input to file if required..
    if(writeToFile)
    {
        myfile << txcFromTxid.value() << "," << txWhereSpent.value() << ","<< getTxOutValue(db, "o"+txcFromTxid.value()+"."+std::get<1>(txidn)).value() << std::endl;
        edges++;
    }

    // Create Taint vector to store taint in FIFO/LIFO
    TaintVector startTv;
    startTv.addTaintedNum(getTxOutValue(db, "o"+txcFromTxid.value()+"."+std::get<1>(txidn)).value());

    // Add taint vector to taintedOutputs
    taintedOutputs["o"+txcFromTxid.value()+"."+std::get<1>(txidn)] = startTv;

    // Run while we still have transactions to check
    while(toCheck.size() > 0)
    {
        // Get fist element and remove it from toCheck
        std::string txToCheck = std::to_string(*toCheck.begin());
        toCheck.erase(toCheck.begin());

        // Get current transaction
        std::optional<DBTransaction> tx = getTx(db, txToCheck);
        assert(tx.has_value()); // we can assert here, since we checked for transaction existance before inserting in toCheck

        if(offchainDB)
		{
			std::string val;
    		Status s = offchainDB->Get(ReadOptions(), tx.value().txid, &val);
			if(s.ok())
			{
                if(!offChainFile.is_open())
                {
                    offChainFile.open(offChainFileName);
                }

				// We reached known address
				offChainFile << tx.value().txid << "," << val << std::endl;
				//std::cout << "KNOWN ADDRESS REACHED" << std::endl;
				continue;
			}
		}

        // create taint vector and construct it by iterating over all inputs
        TaintVector transactionTaintVector;
        // Iterate over all inputs
        for(auto &input : tx.value().inputs)
        {
            // If input is tanted (we have it's taint vector and we can add it to TxTaintVector)
            if(taintedOutputs.find(input) != taintedOutputs.end())
            {
                transactionTaintVector.addTaintVector(taintedOutputs.find(input)->second);
            }
            else
            {
                std::optional<uint64_t> inputValue = getTxOutValue(db, input);
                assert(inputValue.has_value()); // we should have all inputs in our database

                transactionTaintVector.addCleanNum(inputValue.value());
            }
        }

        // Iterate over all outputs
        for(int i = tx.value().outputCount-1; i >=0 ; i--)
        {
            uint64_t currentOutputValue = getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value();

            // Create taint vector for this output and remove elements in front of transactionTaintVector
            TaintVector outputTaintVector = transactionTaintVector.assingTaintVectorToOutput(currentOutputValue);

            // Check if current output is spent in our database and add it to toCheck set
            std::optional<std::string> nextTxC = getTxCWhereInputIsSpent(db, txToCheck, std::to_string(i));
            
            if(nextTxC.has_value())
            {
                if(outputTaintVector.containsTaint())
                {
                    // insert this output to be checked in the future
                    toCheck.insert(std::stoi(nextTxC.value()));
                    // add taint to this vector
                    taintedOutputs["o"+txToCheck+"."+std::to_string(i)] = outputTaintVector;

                    // Insert in file
                    if(writeToFile)
                    {
                        myfile << std::fixed << txToCheck << "," << nextTxC.value()  << "," << outputTaintVector.getTaintValue() << std::endl;
                        edges++;
                    }
                    if(edges > max_edges)
                    {
                        std::cout << "Edge limit exceeded!" << std::endl;
                        exit(1);
                    }
                }
            }
            else
            {
                if(outputTaintVector.containsTaint())
                {
                    // If it is tanted -> update metrics
                    metrics.unspentOutputsTainted++;
                    metrics.unspentTaintedAmount+=outputTaintVector.getTaintValue();
                    unspentOutputValues.push_back(currentOutputValue);
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

    nongraphfile << "lifo," + txid_n << "," << duration.count() << "," << metrics.unspentOutputsTainted << "," << metrics.unspentTaintedAmount  << "," << startingValue.value();
    for(auto& e: unspentOutputValues)
    {
        nongraphfile << "," << e;
    }
    nongraphfile << std::endl;

    return metrics;
}