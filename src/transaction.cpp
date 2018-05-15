#include "transaction.h"

Transaction::Transaction(string fromAddress, string toAddress, double amount)
{
    this->fromAddress = fromAddress;
    this->toAddress = toAddress;
    this->amount = amount;
}

Transaction::Transaction()
{
    this->fromAddress = "";
    this->toAddress = "";
    this->amount = 0;
}

Transaction * Transaction::copyConstructor()
{
    return new Transaction(this->fromAddress,this->toAddress, this->amount);
}

void Transaction::serialize(SF::Archive& ar)
{
    ar & fromAddress;
    ar & toAddress;
    ar & amount;
}
