// C++ program for the above approach

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

	for(int i = 2; i <= arr.size();i++)
	{
		std::vector<std::vector<int> > v(i);
		PartitionSub(arr, 0, arr.size(), i, 0, v, allResults);
	}

	return allResults;
}

void aux(std::vector<int>& xs, int& total, std::vector<std::vector<int>>& found,int pos, int running, std::vector<int> ys)
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

std::vector<std::vector<int>> search(std::vector<int> inputs, int total)
{
    std::vector<std::vector<int>> found;

    aux(inputs, total, found, 0, 0, std::vector<int>());
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

std::vector<std::vector<int>> getPossibleLinks(std::vector<int>& inputs, std::vector<int>& outputs)
{
	std::vector<int> outputIndexes;
	for(int i=0; i<outputs.size(); i++)
	{
		outputIndexes.push_back(i);
	}

	// Prints all possible
	// splits into subsets
	auto allResults = partKSubsets(outputIndexes);
	//std::cout << "allResults size: " << allResults.size() << std::endl;

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
			//int sum_of_elems = std::accumulate(ee.begin(), ee.end(), 0);
			sums.push_back(sum_of_elems);
		}

		//std::cout << "SUMS: ";
		//print(e);
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
	std::cout << "Final result:" << std::endl;
	print(inputOutputLinks);
	return inputOutputLinks;
}

int main()
{

	// Given array
	std::vector<int> inputs = {1000,138};
	std::vector<int> outputs = {11,985,10,127};

	getPossibleLinks(inputs, outputs);

	
	std::vector<int> arr(20);
	arr[5] = 1;
	std::cout << arr[19] << ", " << arr[5] << std::endl;
	
}
