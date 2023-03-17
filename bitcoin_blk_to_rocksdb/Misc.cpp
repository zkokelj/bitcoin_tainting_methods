#include "Misc.hpp"

// all possible values for hex
char hex[] = {'0', '1', '2', '3', '4', '5',
                '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// Varint description:  https://learnmeabitcoin.com/technical/varint
uint64_t varintDecode(std::istream& input)
{

    //TODO: Modify varint to really know what each step does!
    uint8_t lengthId = 0;
    uint64_t res = 0;
    
    input.read((char *)(&lengthId), 1);

    if (lengthId < 0xFD) {
        return lengthId;
    }
    else if (lengthId == 0xFD) {
        input.read((char *)(&res), 2);
    }
    else if (lengthId == 0xFE) {
        input.read((char *)(&res), 4);
    }
    else if (lengthId == 0xFF) {
        input.read((char *)(&res), 8);
    }
    else {
        std::cout << "VARINT ERROR!" << std::endl;
        exit(1);
    }
    
    return res;

}

void print256(uint8_t* arr, int size)
{
    for(auto i=0; i<size; i++)
    {
        std::cout << arr[i];
    }
    std::cout << std::endl;
}


std::string getHexFromArray(std::vector<uint8_t>& data, int size)
{
    std::string x;
    
    for(auto i=0; i<size; i++)
    {
        x += hex[(data[i] & 0xF0) >> 4]; // Top bytes
        x += hex[data[i] & 0x0F]; // Bottom bytes 
    }
    return x;
}

std::string getHexStringFromRaw(uint8_t* arr, int size, bool swap)
{
    std::string x;

    if(!swap)
    {
        for(auto i=0; i<size; i++)
        {
            x += hex[(arr[i] & 0xF0) >> 4]; // Top bytes
            x += hex[arr[i] & 0x0F]; // Bottom bytes 
        }
    }else{
      for(auto i=31; i>=0; i--)
        {
            x += hex[(arr[i] & 0xF0) >> 4]; // Top bytes
            x += hex[arr[i] & 0x0F]; // Bottom bytes 
        }  
    }

    return x;
}



std::string getHexStringFromInt32t(uint32_t n)
{
    std::string x;
    uint8_t bytes [4] = {static_cast<uint8_t>(n & 0x000000FF), static_cast<uint8_t>((n & 0x0000FF00) >> 8),
                 static_cast<uint8_t>((n & 0x00FF0000) >> 16), static_cast<uint8_t>((n & 0xFF000000) >> 24)};
    for(auto i=0; i < 4; i++)
    {
        x += hex[(bytes[i] & 0xF0) >> 4]; // Top bytes
        x += hex[bytes[i] & 0x0F]; // Bottom bytes
    }
    return x;
}

void swapEndianness(std::vector<uint8_t>& data)
{
    std::reverse(data.begin(), data.end());
}

bool compareFunction (std::string a, std::string b) {return a<b;} 


void insertInDatabase(DB* database, Options& options, std::string dbpath)
{
    std::string insertString = "TEST_INSERT_STRING";
    Status s = DB::Open(options, dbpath, &database);
    s = database->Put(WriteOptions(), "insert1", insertString);
    assert(s.ok());

}
