#ifndef MTBITCOIN_H
#define MTBITCOIN_H

#include <map>
#include <sys/types.h>
#include <string>

#include "bitcoin/btcjsonobjects.h"

class MTBitcoin
{
public:
    MTBitcoin();


    // account [optional] the account whose balance should be checked
    // returns total balance of all addresses.
    int64_t GetBalance(const char* account = NULL);

    // account [optional]: the account to which the address should be added.
    // returns new address
    std::string GetNewAddress(const char* account = NULL);

    // sends funds from your wallet to targetAddress
    // returns the transaction id string or NULL
    std::string SendToAddress(int64_t lAmount, const std::string & to_address);

    // Creates a multi-sig address using the public keys (not addresses, unless their public keys are known to bitcoin-qt!)
    // and sends bitcoin to that address.
    // If the multisig address is already known, we can instead just call SendToAddress(multiSigAddress)
    // as bitcoin makes no difference between those and regular addresses.
    // returns the transaction id string or NULL
    std::string SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> & to_publicKeys);


    /*
        std::map<std::string, int64_t> GetAddressesAndBalances();
        int64_t GetBalanceAtAddress(const std::string & str_address);
        int64_t GetMultisigBalance(  );

        not yet implemented, would be nice if it isn't needed.
        instead we could/should keep track of transaction Ids and their value.
    */

    BtcSignedTransactionRef VoteMultiSigRelease(std::string txToSourceId, BtcMultiSigAddressRef, const std::string & destinationAddress, const std::string & signingAddress);
};

#endif // MTBITCOIN_H
