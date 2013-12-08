#ifndef MTBITCOIN_H
#define MTBITCOIN_H

#include <map>
#include <sys/types.h>
#include <string>
#include <list>

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

    // returns public key for address (works only if public key is known)
    std::string GetPublicKey(const std::string &address);

    // Creates a multi-sig address from public keys
    // returns the address string
    std::string GetMultiSigAddress(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet = false, const char* account = NULL);

    // Creates a multi-sig address from public keys
    // returns an object containing info required to withdraw from that address
    BtcMultiSigAddressRef GetMultiSigAddressInfo(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet = true, const char* account = NULL);

    // Returns an object containing information about a raw transaction
    BtcRawTransactionRef GetRawTransaction(const std::string &txId);

    // Returns the number of confirmations of a raw transaction
    int GetConfirmations(BtcRawTransactionRef rawTransaction);

    bool TransactionSuccessfull(int64_t amount, BtcRawTransactionRef rawTransaction, const std::string &targetAddress, int64_t confirmations = 1);

    // sends funds from your wallet to targetAddress
    // returns the transaction id string or NULL
    std::string SendToAddress(int64_t lAmount, const std::string &to_address);

    // Creates a multi-sig address using the public keys (not addresses, unless their public keys are known to bitcoin-qt!)
    // and sends bitcoin to that address.
    // If the multisig address is already known, we can instead just call SendToAddress(multiSigAddress)
    // as bitcoin makes no difference between those and regular addresses.
    // returns the transaction id string or NULL
    std::string SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> &to_publicKeys);

    // creates a partially signed raw transaction to withdraw funds
    // txToSourceId: transaction id that was used to fund the pool
    // sourceAddress: used to know which outputs of the tx we can spend (in case it came from a SendMany() tx)
    // redeemScript [optional]: is passed to the API to do some bitcoin magic. required if unknown or if passing a signingAddress
    // signingAddress [optional]: only this address's private key will be used to sign the transaction (kinda pointless in our case)
    BtcSignedTransactionRef VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const std::string &redeemScript = "", const std::string &signingAddress = "");

    // combines concatenated raw transactions to one raw transaction
    // used to combine the partially signed raw tx's when releasing multi-sig
    BtcSignedTransactionRef CombineTransactions(const std::string &concatenatedRawTransactions);

    std::string SendRawTransaction(const std::string &rawTransaction);

    /*
        std::map<std::string, int64_t> GetAddressesAndBalances();
        int64_t GetBalanceAtAddress(const std::string & str_address);
        not implemented yet, but bitcoin now offers a nice API function to get that, so if we really need it I can add it almost instantly

        int64_t GetMultisigBalance(  );
        not yet implemented, would be nice if it isn't needed.
        instead we could/should keep track of transaction Ids and their value to know the balance of multi-sig addresses
        and clients should tell the server what the tx id is
    */
};

#endif // MTBITCOIN_H
