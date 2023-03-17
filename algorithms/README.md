## Algorithms

Here is an implementation of various tainting algorithms that I used on Bitcoin transactions.

### Build

`cd algorithms`
`make`

### How to use

`./algorithms -db <absolute_path_to_rocksdb_database> -a <algoritm> -t <tainted_output_txid_n> -h <haircut_percentage>`

algorithm can be one of the following: poison, haircut, fifo, lifo, tiho, comb
