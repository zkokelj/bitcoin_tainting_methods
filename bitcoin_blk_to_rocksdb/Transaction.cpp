#include "Transaction.hpp"

// TxIn constructor
TxIn::TxIn(std::istream& input)
{
   input.read((char *)(this->prevTx), 32);
   input.read((char *)(&this->nTxOut), 4);
   this->scriptLen = varintDecode(input);
   // read script (length is on scriptLen - varint!)
   for(auto i=0; i < scriptLen; i++)
   {
      uint8_t x;
      input.read((char*)(&x), 1);
      this->scriptSig.push_back(x);
   }
   // input.ignore(scriptLen);

   // Print scriptSig!
   //for (std::vector<uint8_t>::const_iterator i = this->scriptSig.begin(); i != this->scriptSig.end(); ++i)
   //   std::cout << *i;
   //std::cout << std::endl;

   input.read((char*)(&this->sequence), 4);
}

// TxOut constructor
TxOut::TxOut(std::istream& input)
{
   input.read((char*)(&this->value), 8);
   this->scriptLen = varintDecode(input);

   // std::cout << "TxOut value: " << this->value << std::endl;

   for(auto i=0; i<scriptLen; i++)
   {
      uint8_t x;
      input.read((char*)(&x), 1);
      this->script.push_back(x);
   }
   // input.ignore(scriptLen);
}

Tx::Tx(std::istream& input, bool skipDetails) {
    
    auto startTx = input.tellg();

    //std::cout << "Transaction constructor" << std::endl; 
    input.read((char*)(&(this->version)), 4);

    // Read inputs
    this->inputsCounter = varintDecode(input);
    //std::cout << "Number of inputs: " << this->inputsCounter << std::endl;
    for (auto i=0; i < this->inputsCounter; i++)
    {
        TxIn tmpTxIn = TxIn(input);
        if(!skipDetails)
            this->inputs.push_back(tmpTxIn);
    }

    // Read outputs
    this->outputsCounter = varintDecode(input);
    //std::cout << "Number of outputs: " << this->outputsCounter << std::endl;
    for(auto i=0; i < this->outputsCounter; i++)
    {
        TxOut tmpTxOut = TxOut(input);
        if(!skipDetails)
            this->outputs.push_back(tmpTxOut);
    }

    input.read((char*)(&(this->lockTime)), 4);

    // Calculate TxId of transaction (Double hash)
    if(!skipDetails)
    {
        auto endTx = input.tellg();
        auto lengthTx = endTx - startTx;

        std::vector<uint8_t> tmpHeader;

        char* buffer = new char [lengthTx];

        // Go to the start of Tx and read whole Tx to buffer (used to calculate TxID)
        // TODO: Do this more efficiently - do not read transaction twice!
        input.seekg(startTx);
        input.read(buffer, lengthTx);

        for(auto i = 0; i < lengthTx; i++)
        {
            tmpHeader.emplace_back(buffer[i]);
        }
        
        delete buffer;

        picosha2::hash256_one_by_one hasher;
        hasher.process(tmpHeader.begin(), tmpHeader.end());
        hasher.finish();

        std::vector<uint8_t> hash(32);
        hasher.get_hash_bytes(hash.begin(), hash.end());

        //Second SHA-256
        picosha2::hash256_one_by_one hasher2;

        hasher2.process(hash.begin(), hash.end());
        hasher2.finish();

        std::vector<uint8_t> hash2(32);
        hasher2.get_hash_bytes(hash2.begin(), hash2.end());

        swapEndianness(hash2);
        
        this->txid = getHexFromArray(hash2, 32);
    }
}
