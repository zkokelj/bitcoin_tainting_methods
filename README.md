# Comparison of tainting analysis methods in Bitcoin network (Master's thesis project)

Bitcoin offers many new opportunities and challenges with its pseudonymity
and open source nature. One of the challenges is performing taint analysis
in order to follow coins that originated from criminal activities. Due to a
large number of nodes and the complexity of the Bitcoin transaction graph,
methods for the performance of taint analysis have been developed. In this
master’s thesis, existing methods were implemented and furthermore a new
method called COMB was proposed. A database that supports running these
methods was put together. For the testing purpose, two data sets of starting
transaction outputs were prepared. After executing all methods on the data
sets and analysis of the results, it was concluded that all methods have pros
and cons. The intersections of graphs produced by different algorithms from
the same starting inputs were analyzed, because they contain transactions
with a higher probability of being connected to the starting transaction out-
put. Another database with off-chain data that can be used in implemented
methods was developed. Even with a relatively small database, we were able
to reach some known transactions with implemented methods, showing the
big potential of this technique.

#### This repository consists of:

- `bitcoin_blk_to_rocksdb` - Source code (C++) for transforming bitcoin `.blk` files to `RocksDB` database
- `algorithms` - Source code (C++) of algorithms for different tainting methods
- `Masters_thesis.pdf` - Master's thesis (mostly in Slovene language, abstract is available in English)

I analysed data with `Python` programming language with following libraries:

- `networkx`
- `statistics`
- `community`
- `powerlaw`
- `matplotlib`
- `scipy`
