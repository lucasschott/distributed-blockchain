#include "blockchain.h"

Blockchain::Blockchain(int difficulty, double miningReward)
{
    this->chain.push_back(createGenesisBlock());
    this->difficulty = difficulty;
    this->miningReward = miningReward;
}


Blockchain::Blockchain()
{
    this->chain.push_back(createGenesisBlock());
    this->difficulty = 2;
    this->miningReward = 50;
}


Block * Blockchain::createGenesisBlock()
{
    vector<Transaction*> trans;
    return new Block(time(nullptr), trans, "0");
}

Block * Blockchain::getLatestBlock()
{
    return chain.at(chain.size() - 1);
}

void Blockchain::minePendingTransactions(string miningRewardAddress)
{
    Block * block = new Block(
            time(nullptr),pendingTransactions,getLatestBlock()->hash);
    block->mineBlock(difficulty);
    chain.push_back(block);
    pendingTransactions.clear();
    pendingTransactions.push_back(
            new Transaction("", miningRewardAddress, miningReward)
            );
}

void Blockchain::createTransaction(Transaction * transaction){
    pendingTransactions.push_back(transaction);
}

double Blockchain::getBalanceOfAddress(string address)
{
    int balance = 0;
    for(Block * block : chain){
        for(Transaction * trans : block->transactions){
            if(address.compare(trans->fromAddress)){
                balance += trans->amount;
            }
            if(address.compare(trans->toAddress)){
                balance -= trans->amount;
            }
        }
    }
    return balance;
}

bool Blockchain::isChainValid()
{
    for (uint i = 1; i < chain.size(); i++){
        Block * currentBlock = chain.at(i);
        Block * previousBlock = chain.at(i - 1);
        if (currentBlock->hash.compare(currentBlock->calculateHash())!=0){
            return false;
        }
        if (currentBlock->previousHash.compare(previousBlock->hash)!=0){
            return false;
        }
    }
    return true;
}

string Blockchain::toString()
{
    int i=0;
    string str = "Blockchain\nDifficulty : " + to_string(difficulty) + 
        "\nMining reward : " + to_string(miningReward) + "\n";
    for(Block * bloc : this->chain)
    {
        str += "\nBlock " + to_string(i) + "\n" + bloc->toString();
        i++;
    }
    return str;
}

Blockchain * Blockchain::copyConstructor()
{
    Blockchain * newBlockchain = new Blockchain(this->difficulty,this->miningReward);
    newBlockchain->chain.clear();
    for(Block * block : this->chain)
    {
        newBlockchain->chain.push_back(block->copyConstructor());
    }
    for(Transaction * trans : this->pendingTransactions)
    {
        newBlockchain->pendingTransactions.push_back(trans->copyConstructor());
    }
    return newBlockchain;
}

void Blockchain::serialize(SF::Archive& ar)
{
    ar & chain;
    ar & difficulty;
    ar & miningReward;
}
