#ifndef _BLOCKCHAIN_H
#define _BLOCKCHAIN_H

#include <vector>
#include <cstdlib>
#include <string>
#include <RCF/RCF.hpp>
#include <SF/Archive.hpp>

#include "block.h"

using namespace std;

class Blockchain
{

    public:

        vector<Block*> chain;
        int difficulty;
        vector<Transaction*> pendingTransactions;
        double miningReward;


        Blockchain(int difficulty, double miningReward);

        Blockchain();

        Block *  createGenesisBlock();

        Block * getLatestBlock();

        void minePendingTransactions(string miningRewardAddress);

        void createTransaction(Transaction * transaction);

        double getBalanceOfAddress(string address);

        bool isChainValid();

        string toString();

        Blockchain * copyConstructor();

        void serialize(SF::Archive& ar);
};

#endif
