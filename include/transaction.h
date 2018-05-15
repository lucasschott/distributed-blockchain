#ifndef _TRANSACTION_H
#define _TRANSACTION_H
#include <string>
#include <RCF/RCF.hpp>
#include <SF/Archive.hpp>
#include <SF/string.hpp>

using namespace std;

class Transaction
{
    public :

        string fromAddress;
        string toAddress;
        double amount;

        Transaction(string fromAddress, string toAddress, double amount);
        Transaction();
        Transaction * copyConstructor();

        void serialize(SF::Archive& ar);
};

#endif
