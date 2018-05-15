#include "block.h"

Block::Block(time_t timestamp, vector<Transaction*> transactions,
        string previousHash)
{
    this->previousHash = previousHash;
    this->timestamp = timestamp;
    this->transactions = transactions;
    this->nonce = 0;
    this->hash = calculateHash();
}

Block::Block(string previousHash, time_t timestamp,long nonce)
{
    this->previousHash = previousHash;
    this->timestamp = timestamp;
    this->nonce = nonce;
}


Block::Block()
{
    this->previousHash = "";
    this->timestamp = 0;
    this->nonce = 0;
}


string Block::calculateHash()
{
    string str_block = toString();
    return sha256(str_block);
}

void Block::mineBlock(int difficulty)
{
    string stringZero = "";
    int i;
    for(i=0;i<difficulty;i++)
        stringZero += "0";
    while(hash.substr(0, difficulty).compare(stringZero)!=0)
    {
        nonce++;
        hash = calculateHash();
    }
}

string Block::toString()
{
    string str = "Previous Hash : " + previousHash + "\nTimestamp : " + 
        to_string(timestamp) + "\nTransaction : ";
    for(Transaction * trans : transactions)
    {
        str += "\n " + to_string(trans->amount) + " : from " + 
        trans->fromAddress + " to " + trans->toAddress;
    }
    str += "\nNonce : " + to_string(nonce) + "\n";
    return str;
}

Block * Block::copyConstructor()
{
    Block * newBlock = new Block(this->previousHash,this->timestamp,this->nonce);
    newBlock->hash = this->hash;
    for(Transaction * trans : this->transactions)
    {
        newBlock->transactions.push_back(trans->copyConstructor());
    }
    return newBlock;
}


void Block::serialize(SF::Archive& ar)
{
    ar & previousHash;
    ar & timestamp;
    ar & transactions;
    ar & nonce;
    ar & hash;
}
