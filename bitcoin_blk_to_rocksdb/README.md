## Bitcoin blk to RocksDB

This program is used to create fast key-value database using RocksDB from Bitcoin blockchain data as they are written to the disc by (Bitcoin Node)[https://github.com/bitcoin/bitcoin] - as `.blk` files. DB can be then used to perform fast queries - in my case for tainting analysis.

## Requirements:

- Ubuntu 20.04
- build-essentials -> `sudo apt install build-essential`
- `sudo apt-get install -y libbz2-dev`
- `sudo apt-get install libz-dev`
- `sudo apt-get install -y libsnappy-dev`

## Build

1. Clone RocksDB source code

   `cd /bitcoin_blk_parser/lib/`

   `git clone https://github.com/facebook/rocksdb.git`

2. Compile RocksDB

   `cd rocksdb`

   `make static_lib` (this can take a few minutes)

3. Compile BitcoinBlkToRocksDB

   `cd bitcoin_blk_to_rocks_db`

   `make`

4. Run bitcoin block parser

   `./bitcoin_block_parser --blkloc <location_of_bitcoin_blk_files> --dbloc <location_of_rocks_db_database>`
