#ifndef _BLOCK_H
#define _BLOCK_H

#include <string>
#include <ctime>
#include <vector>
#include <cstdint>
#include <functional>
#include <iostream>
#include <RCF/RCF.hpp>
#include <SF/Archive.hpp>
#include <SF/vector.hpp>

#include "transaction.h"
#include "sha256.h"

using namespace std;

class Block
{
    public :

    string previousHash;
    time_t timestamp;
    vector<Transaction*> transactions;
    long nonce;
    string hash;

    Block(time_t timestamp, vector<Transaction*> transactions,
            string previousHash);

    Block(string previousHash, time_t timestamp,long nonce);

    Block();

    string calculateHash();

    void mineBlock(int difficulty);
    
    string toString();

    Block * copyConstructor();

    void serialize(SF::Archive& ar);

};

#endif
