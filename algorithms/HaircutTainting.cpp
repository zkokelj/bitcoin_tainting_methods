#include "AlgorithmDecl.hpp"

#include <sstream>
#include <iomanip>

AlgorithmMetrics HaircutTainting(DB* db, std::string txid_n, double threshold, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB)
{
    // Start measuring time
    auto startTime = std::chrono::high_resolution_clock::now();

    // Metrics with values of unspent values
    std::vector<uint64_t> unspentOutputValues;

    // Write graph to file
    std::ofstream myfile;
    std::ostringstream threshold_precision;
    threshold_precision << std::fixed;
    threshold_precision << std::setprecision(4);
    threshold_precision << threshold;
    if(writeToFile)
    {
        myfile.open ("./results/haircut_" + threshold_precision.str() + "-" + txid_n + ".txt");
    }

    // check if offchainDB exists
	std::ofstream offChainFile;
	std::string offChainFileName = "./results/_offchain_haircut_" + threshold_precision.str() + "-" + txid_n + ".txt";

    uint64_t edges = 0;
    uint64_t max_edges = 1000000;

    // Metrics.
    AlgorithmMetrics metrics;
    
    // split to txid and N
    auto txidn = splitTxIDN(txid_n);

    // Data structure to keep information which transactions (and in which) order to check
    std::set<int> toCheck;

    // Data structure to keep information about percentage of taint in output.
    std::map<std::string, double> taintedOutputs;

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

    toCheck.insert(std::stoi(txWhereSpent.value()));

    // Add initial output to the list of taitned outputs
    taintedOutputs["o"+txcFromTxid.value()+"."+std::get<1>(txidn)] = 1.0;
    if(writeToFile)
    {
        myfile << txcFromTxid.value() << "," << txWhereSpent.value() << ","<< getTxOutValue(db, "o"+txcFromTxid.value()+"."+std::get<1>(txidn)).value() << std::endl;
        edges++;
    }

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

        uint64_t totalValue = 0;
        uint64_t taintedValue = 0;

        for(auto &input : tx.value().inputs)
        {
            std::optional<uint64_t> inputValue = getTxOutValue(db, input);
            assert(inputValue.has_value()); // we should have all inputs in our database
            
            totalValue += inputValue.value();

            // Check if input is tanited
            if(taintedOutputs.find(input) != taintedOutputs.end())
            {
                taintedValue += inputValue.value() * taintedOutputs[input];
            }
        }

        double transactionTaint = (double)taintedValue/totalValue;
        
        // If taint is above certain threshold => Mark inputs as tainted
        if(transactionTaint > threshold)
        {
            metrics.transactionsTainted++;
            // Iterate over all outputs
            for(int i = 0; i < tx.value().outputCount; i++)
            {
                // Check if current output is spent in our database and add it to toCheck set
                std::optional<std::string> nextTxC = getTxCWhereInputIsSpent(db, txToCheck, std::to_string(i));
                uint64_t currentOutputValue = getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value();
                double taintInOutput = currentOutputValue*transactionTaint;
                if(nextTxC.has_value())
                {
                    toCheck.insert(std::stoi(nextTxC.value()));
                    taintedOutputs["o"+txToCheck+"."+std::to_string(i)] = transactionTaint;

                    if(writeToFile)
                    {
                        myfile << std::fixed << txToCheck << "," << nextTxC.value()  << "," << taintInOutput << std::endl;
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
                    metrics.unspentOutputsTainted+=1;
                    metrics.unspentTaintedAmount+=taintInOutput;
                    unspentOutputValues.push_back(currentOutputValue);
                }

            }

        }
    }
    auto duration =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime);

    myfile.close();
	// check if nongraph file is empty
	if(offchainDB)
	{
		if(offChainFile.is_open())
        {
            offChainFile.close();
        }
	}

    nongraphfile << "haircut_" + threshold_precision.str() + "," + txid_n  << "," << duration.count() << "," << metrics.unspentOutputsTainted << "," << metrics.unspentTaintedAmount  << "," << startingValue.value();
    for(auto& e: unspentOutputValues)
    {
        nongraphfile << "," << e;
    }
    nongraphfile << std::endl;

    return metrics;
}