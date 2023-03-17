algorithms: algorithms.o misc.o PoisonTainting.o HaircutTainting.o FIFOTainting.o LIFOTainting.o TIHOTainting.o COMBTainting.o TestDatabase.o InsertOffChainData.o
	g++ -std=c++17 algorithms.o misc.o PoisonTainting.o HaircutTainting.o FIFOTainting.o LIFOTainting.o TIHOTainting.o COMBTainting.o TestDatabase.o InsertOffChainData.o ../bitcoin_blk_parser/lib/rocksdb/librocksdb.a  -lsnappy -lpthread -lbz2 -lz -lrt -ldl -o algorithms
	
algorithms.o: algorithms.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ algorithms.cpp

misc.o: misc.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ misc.cpp

PoisonTainting.o: PoisonTainting.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ PoisonTainting.cpp

HaircutTainting.o: HaircutTainting.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ HaircutTainting.cpp

FIFOTainting.o: FIFOTainting.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ FIFOTainting.cpp

LIFOTainting.o: LIFOTainting.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ LIFOTainting.cpp

TIHOTainting.o: TIHOTainting.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ TIHOTainting.cpp

COMBTainting.o: COMBTainting.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ COMBTainting.cpp

TestDatabase.o: TestDatabase.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ TestDatabase.cpp

InsertOffChainData.o: InsertOffChainData.cpp
	g++ -c -std=c++17 -I ../bitcoin_blk_parser/lib/rocksdb/include/ -I ../bitcoin_blk_parser/lib/cereal/include/ InsertOffChainData.cpp