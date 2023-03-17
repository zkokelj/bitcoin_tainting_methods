bitcoin_block_parser: bitcoin_block_parser.o Block.o Misc.o Transaction.o DBInsert.o
	g++ -std=c++17 bitcoin_block_parser.o ./lib/rocksdb/librocksdb.a  -lsnappy -lpthread -lbz2 -lz -lrt -ldl Block.o Misc.o Transaction.o DBInsert.o -o bitcoin_block_parser

bitcoin_block_parser.o: bitcoin_block_parser.cpp
	g++ -c -std=c++17 -I ./lib/rocksdb/include/ -I ./lib/cereal/include/ bitcoin_block_parser.cpp

DBInsert.o: DBInsert.cpp
	g++ -c -std=c++17 -I ./lib/rocksdb/include/ DBInsert.cpp

Block.o: Block.cpp
	g++ -c -std=c++17 -I ./lib/rocksdb/include/ Block.cpp

Misc.o: Misc.cpp
	g++ -c -std=c++17 -I ./lib/rocksdb/include/ Misc.cpp

Transaction.o: Transaction.cpp
	g++ -c -std=c++17 -I ./lib/rocksdb/include/ Transaction.cpp

.PHONY: clean

clean:
	rm *.o
