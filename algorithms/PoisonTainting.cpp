#include "AlgorithmDecl.hpp"

#include <bits/stdc++.h>

AlgorithmMetrics  PoisonTainting(DB* db, std::string txid_n, bool writeToFile, std::ofstream& nongraphfile, DB* offchainDB)
{
    // Start measuring time
    auto startTime = std::chrono::high_resolution_clock::now();
    // Write graph to file
    std::ofstream myfile;
    if(writeToFile)
    {
        myfile.open ("./results/poison-" + txid_n + ".txt");
    }

    // check if offchainDB exists
	std::ofstream offChainFile;
	std::string offChainFileName = "./results/_offchain_poison-" + txid_n + ".txt";
	if(offchainDB)
	{
		offChainFile.open(offChainFileName);
	}

    // Metrics
    AlgorithmMetrics metrics;

    uint64_t edges = 0;
    uint64_t max_edges = 1000000;

    auto txidn = splitTxIDN(txid_n);

    // Data structure to keep information which transactions (and in which) order to check
    std::set<int> toCheck;

    // Data structure to keep information how tainted the outputs are
    std::set<std::string> taintedTxCs;

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

        // Add current transactipn to the list of tainted ones
        taintedTxCs.insert(txToCheck);

        // increase transactions tainted counter
        metrics.transactionsTainted++;

        // Get current transaction
        std::optional<DBTransaction> tx = getTx(db, txToCheck);
        assert(tx.has_value()); // we can assert here, since we checked for transaction existance before inserting in toCheck

        if(offchainDB)
		{
			std::string val;
    		Status s = db->Get(ReadOptions(), tx.value().txid, &val);
			if(s.ok())
			{
				// We reached known address
				offChainFile << tx.value().txid << "," << val << std::endl;
				std::cout << "KNOWN ADDRESS REACHED" << std::endl;
				continue;
			}
		}        

        // Interate over transaction outputs
        for(int i=0; i<tx.value().outputCount; i++)
        {
            // Check if current output is spent in our database and add it to toCheck set
            std::optional<std::string> nextTxC = getTxCWhereInputIsSpent(db, txToCheck, std::to_string(i));
            if(nextTxC.has_value())
            {
                toCheck.insert(std::stoi(nextTxC.value()));

                // Write graph.
                if(writeToFile)
                {
                    myfile << txToCheck << "," << nextTxC.value() << "," << getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value() << std::endl;
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
                metrics.unspentTaintedAmount+=getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value();
            }
        }
    }

    auto duration =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime);

    myfile.close();
	// check if nongraph file is empty
	if(offchainDB)
	{
		std::ifstream file(offChainFileName);
		if(is_empty(file))
		{
			std::filesystem::remove(offChainFileName);
		}

	}

    nongraphfile << "poison," + txid_n << "," << duration.count() << "," << metrics.unspentOutputsTainted << "," << metrics.unspentTaintedAmount  << ", " << startingValue.value();
    nongraphfile << std::endl;
    
    //std::cout << "Duration: " << duration.count() << std::endl;
    return metrics;

}
