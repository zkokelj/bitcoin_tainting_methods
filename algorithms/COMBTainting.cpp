#include "AlgorithmDecl.hpp"

#include <sstream>
#include <iomanip>
#include <bits/stdc++.h>
#include <numeric>


void print(const std::vector<std::vector<int>>& v) {
  std::cout << "{ ";
  for (const auto& p : v) {
    std::cout << "(";
    for (const auto& e : p) {
      std::cout << e << " ";
    }
    std::cout << ") ";
  }
  std::cout << "}" << std::endl;
}

void PartitionSub(std::vector<int> arr, int i,
				int N, int K, int nos,
				std::vector<std::vector<int> >& v, std::vector<std::vector<std::vector<int>>>& allResults)
{
	// Elements in vector...
	std::vector<std::vector<int>> elements;

	std::vector<std::vector<std::vector<int>>> elementsCombined;

	if (i >= N)
	{
		if (nos == K)
		{
			for (int x = 0; x < v.size(); x++)
			{
				std::vector<int> vec;
				// Print current subset
				for (int y = 0; y < v[x].size(); y++)
				{
					vec.push_back(v[x][y]);
				}
				elements.push_back(vec);

			}
			//print(elements);

			if(elements.size() > 0)
				allResults.push_back(elements);				
		}

		return;
	}

	for (int j = 0; j < K; j++)
	{
		if (v[j].size() > 0)
		{
			v[j].push_back(arr[i]);
			PartitionSub(arr, i + 1, N, K, nos, v, allResults);
			v[j].pop_back();
		} else {
			v[j].push_back(arr[i]);
			PartitionSub(arr, i + 1, N, K, nos + 1, v, allResults);
			v[j].pop_back();
			break;
		}
	}

	return;
}

std::vector<std::vector<std::vector<int>>> partKSubsets(std::vector<int> arr)
{
	// All results for combinaions
	std::vector<std::vector<std::vector<int>>> allResults;

	for(int i = 1; i <= arr.size();i++)
	{
		std::vector<std::vector<int> > v(i);
		PartitionSub(arr, 0, arr.size(), i, 0, v, allResults);
	}

	return allResults;
}

void aux(std::vector<uint64_t>& xs, int& total, std::vector<std::vector<uint64_t>>& found,int pos, int running, std::vector<uint64_t> ys)
{
    if(running >= total)
    {
        found.push_back(ys);
    }
    else if(pos < xs.size())
    {
        // skip an element
        aux(xs, total, found, pos+1, running, ys);

        // add an element
        ys.push_back(pos);
        aux(xs, total, found, pos+1, running+xs[pos], ys);
        ys.pop_back();
    }
}

std::vector<std::vector<uint64_t>> search(std::vector<uint64_t> inputs, int total)
{
    std::vector<std::vector<uint64_t>> found;

    aux(inputs, total, found, 0, 0, std::vector<uint64_t>());
    return found;
}

void printVector(std::vector<int> v)
{
    for (int& e : v)
    {
        std::cout << e << ", ";
    }
    std::cout << std::endl;
}

auto product(const std::vector<std::vector<int>>& lists) {
  std::vector<std::vector<int>> result;
  if (std::find_if(std::begin(lists), std::end(lists), 
    [](auto e) -> bool { return e.size() == 0; }) != std::end(lists)) {
    return result;
  }
  for (auto& e : lists[0]) {
    result.push_back({ e });
  }
  for (size_t i = 1; i < lists.size(); ++i) {
    std::vector<std::vector<int>> temp;
    for (auto& e : result) {
      for (auto f : lists[i]) {
        auto e_tmp = e;
        e_tmp.push_back(f);
        temp.push_back(e_tmp);
      }
    }
    result = temp;
  }
  return result;
}

bool checkEncodedVector(std::vector<int>& v, int numOfInputs)
{
	if(v.size() == 0)
		return false;

	int value = v[0];

	for(int i = 1; i < v.size(); i++)
	{
		int duplicate = value & v[i];
		if(duplicate > 0)
		{
			return false;
		}

		value += v[i];
	}

	if(value == ((int)std::pow(2, numOfInputs))-1)
	{
		return true;
	}
	else
	{
		return false;
	}

}

std::vector<int> getElementsFromInt(int n)
{
	std::vector<int> result;
	for(int i = 0; i < 32; i++)
	{
		if(n & (1 << i))
		{
			result.push_back(i);
		}
	}
	return result;
}

std::vector<std::vector<int>> getPossibleLinks(std::vector<uint64_t>& inputs, std::vector<uint64_t>& outputs)


{
	std::vector<int> outputIndexes;
	for(int i=0; i<outputs.size(); i++)
	{
		outputIndexes.push_back(i);
	}

	// Prints all possible
	// splits into subsets
	auto allResults = partKSubsets(outputIndexes);

	std::vector<std::vector<int>> inputOutputLinks;
	for(int i =0; i < outputs.size(); i++)
	{
		inputOutputLinks.push_back(std::vector<int>{});
	} 

	for(auto&e : allResults)
	{
		std::vector<int> sums;
		for(auto& ee: e)
		{
			int sum_of_elems = 0;
			for(auto& eee: ee)
			{
				sum_of_elems += outputs[eee];
			}
			sums.push_back(sum_of_elems);
		}

		std::vector<std::vector<int>> compressedVector;
		for(auto& e: sums)
		{
			// Calculate which options for inputs are possible for current group of outputs
			auto res = search(inputs, e);
			std::vector<int> vec;
			for(auto& el : res)
			{
				int v = 0;
				for(auto&e : el)
				{
					v += 1 << e;
				}
				vec.push_back(v);
			}
			compressedVector.push_back(vec);
		}

		auto combinations = product(compressedVector);
		for(auto& combination: combinations)
		{
			if(checkEncodedVector(combination, inputs.size()))
			{

				//std::cout << "Valid: " << std::endl;
				for(int i = 0; i < e.size(); i++)
				{
					/*
					std::cout << "\t-----------" << std::endl;
					std::cout << "\tOutputs: ";
					printVector(e[i]);
					std::cout << "\tInputs: "; 
					printVector(getElementsFromInt(combination[i]));
					*/

					// loop over outputs
					for(auto& el : e[i])
					{
						for(auto& q : getElementsFromInt(combination[i]))
						{
							inputOutputLinks[el].push_back(q);
						}
						
					}
				}
			}
		}

		
	}
	//std::cout << "Final result:" << std::endl;
	//print(inputOutputLinks);
	return inputOutputLinks;
}



AlgorithmMetrics COMBTainting(DB* db, std::string txid_n, std::ofstream& nongraphfile, DB* offchainDB)
{
    // Start measuring time
    auto startTime = std::chrono::high_resolution_clock::now();

    // Metrics with values of unspent values
    std::vector<uint64_t> unspentOutputValues;

    // Declare variables (fixed in this algorithm)
    double threshold = 0.1;
    bool writeToFile = true;

    // Write graph to file
    std::ofstream myfile;
    std::ostringstream threshold_precision;
    threshold_precision << std::fixed;
    threshold_precision << std::setprecision(4);
    threshold_precision << threshold;
    if(writeToFile)
    {
        myfile.open ("./results/comb-" + txid_n + ".txt");
    }

	// check if offchainDB exists
	std::ofstream offChainFile;
	std::string offChainFileName = "./results/_offchain_comb-" + txid_n + ".txt";

    uint64_t edges = 0;
    uint64_t max_edges = 1000000;

    // Metrics
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
				// std::cout << "Reached exchange: " << val << ":" << tx.value().txid << std::endl;
				// std::cout << toCheck.size() << std::endl;
				continue;
			}
		}
        
        // Vectors of inputs and outputs
        std::vector<uint64_t> inputs;
        std::vector<uint64_t> outputs;
        std::vector<double> taintedInputs;

        // Iterate over all inputs
        for(auto &input : tx.value().inputs)
        {
            std::optional<uint64_t> inputValue = getTxOutValue(db, input);
            assert(inputValue.has_value()); // we should have all inputs in our database
            inputs.push_back(inputValue.value());

            // Check if input is tanited
            if(taintedOutputs.find(input) != taintedOutputs.end())
            {
              taintedInputs.push_back(taintedOutputs[input]);
            }else
            {
              taintedInputs.push_back(0);
            }
        }

        // Iterate over all outputs
        for(int i = 0; i < tx.value().outputCount; i++)
        {
          uint64_t currentOutputValue = getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value();
          outputs.push_back(currentOutputValue);
        }

		if(inputs.size() > 5 || outputs.size() > 5)
		{
			//std::cout << "Haircut" << std::endl;
			double taintPercentage = 0;
			int totalValue = 0;
			int taintedValue = 0;
			
			for(int i = 0; i < inputs.size(); i++)
			{ 
				totalValue += inputs[i];
				taintedValue += inputs[i]*taintedInputs[i];
			}
			
			double transactionTaint = (double)taintedValue/(double)totalValue;

			// Iterate over all outputs and set outputs as tainted if their value is at least 10000 satoshis(around 5 euros at current ATH price)
			for(int i = 0; i < outputs.size(); i++)
			{
				std::optional<std::string> nextTxC = getTxCWhereInputIsSpent(db, txToCheck, std::to_string(i));
				uint64_t currentOutputValue = getTxOutValue(db, "o"+txToCheck+"."+std::to_string(i)).value();

				if(nextTxC.has_value() && currentOutputValue > 10000 && transactionTaint > 0.05)
                {
					toCheck.insert(std::stoi(nextTxC.value()));
                    taintedOutputs["o"+txToCheck+"."+std::to_string(i)] = transactionTaint;

					if(writeToFile)
                    {
                        myfile << std::fixed << txToCheck << "," << nextTxC.value()  << "," << currentOutputValue << std::endl;
                        edges++;

						if(edges > max_edges)
						{
							std::cout << "Edges limit exceeded" << std::endl;
							exit(71);
						}
                    }
				}
				else
				{
					metrics.unspentOutputsTainted+=1;
                    metrics.unspentTaintedAmount+=currentOutputValue;
                    unspentOutputValues.push_back(currentOutputValue);
				}

			}

		}
		else
		{
			// Actually calculate combinations.
			// std::cout << "Inputs: " << inputs.size() << ", Outputs: " << outputs.size() << std::endl;
			// Get all possible options for all outputs
			std::vector<std::vector<int>> links = getPossibleLinks(inputs, outputs);
			
			// Calculate how you 'll determine links from inputs to outputs
			for(int j = 0; j < links.size(); j++)
			{
				std::vector<double> inputPercentage(inputs.size());
				for(auto& el: links[j])
				{
					inputPercentage[el] += 1.0;
				}
				for(int i = 0; i < inputs.size(); i++)
				{
					inputPercentage[i] /= links[j].size();
				}

				// Calculate for current output the amount of taint!
				double taint = 0;
				for(int i = 0; i < inputPercentage.size(); i++)
				{
					taint += inputPercentage[i] * taintedInputs[j];
				}

				// Check if taint is significant
				if(taint > 0.05)
				{
					std::optional<std::string> nextTxC = getTxCWhereInputIsSpent(db, txToCheck, std::to_string(j));
					uint64_t currentOutputValue = outputs[j];


					if(nextTxC.has_value() && currentOutputValue > 10000)
					{
						toCheck.insert(std::stoi(nextTxC.value()));
						taintedOutputs["o"+txToCheck+"."+std::to_string(j)] = taint;

						if(writeToFile)
						{
							myfile << std::fixed << txToCheck << "," << nextTxC.value()  << "," << taint << std::endl;
							edges++;

							if(edges > max_edges)
							{
								std::cout << "Edges limit exceeded" << std::endl;
								exit(7);
							}
						}
					}
					else
					{
						metrics.unspentOutputsTainted+=1;
						metrics.unspentTaintedAmount+=taint;
						unspentOutputValues.push_back(currentOutputValue);
					}
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

		std::ifstream file(offChainFileName);
		if(is_empty(file))
		{
			std::filesystem::remove(offChainFileName);
		}
	}

    nongraphfile << "comb," + txid_n  << "," << duration.count() << "," << metrics.unspentOutputsTainted << "," << metrics.unspentTaintedAmount  << "," << startingValue.value();
    for(auto& e: unspentOutputValues)
    {
        nongraphfile << "," << e;
    }
    nongraphfile << std::endl;

    return metrics;
}