#ifndef IMTBITCOIN_H
#define IMTBITCOIN_H

#include <map>
#include <sys/types.h>
#include <string>
#include <list>
#include <tr1/memory>

#include "btcobjects.h"

// interface
class IMTBitcoin
{
public:
    // account [optional] the account whose balance should be checked
    // returns total balance of all addresses.
    virtual int64_t GetBalance(const char *account = NULL) = 0;

    // account [optional]: the account to which the address should be added.
    // returns new address
    virtual std::string GetNewAddress(const std::string &account = "") = 0;

    // returns public key for address (works only if public key is known)
    virtual std::string GetPublicKey(const std::string &address) = 0;

    // Creates a multi-sig address from public keys
    // returns the address string
    virtual std::string GetMultiSigAddress(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet = false, const std::string &account = "") = 0;

    // Creates a multi-sig address from public keys
    // returns an object containing info required to withdraw from that address
    virtual BtcMultiSigAddressRef GetMultiSigAddressInfo(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet = true, const std::string &account = "") = 0;

    // Returns an object containing information about a raw transaction
    virtual BtcRawTransactionRef GetRawTransaction(const std::string &txId) = 0;

    // Returns an object containing information about a raw transaction
    // times out after 10 seconds
    virtual BtcRawTransactionRef WaitGetRawTransaction(const std::string &txId) = 0;

    // Returns the number of confirmations of a raw transaction
    virtual int GetConfirmations(const std::string &rawTransaction) = 0;

    virtual bool TransactionSuccessfull(int64_t amount, BtcRawTransactionRef rawTransaction, const std::string &targetAddress, int64_t confirmations = 1) = 0;

    // sends funds from your wallet to targetAddress
    // lAmount: integer containing amount in satoshis
    // returns the transaction id string or NULL
    virtual std::string SendToAddress(const std::string &to_address, int64_t lAmount) = 0;

    // Creates a multi-sig address using the public keys (not addresses, unless their public keys are known to bitcoind!)
    // and sends bitcoin to that address.
    // lAmount: integer containing amount in satoshis
    // returns the transaction id string or NULL
    virtual std::string SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> &to_publicKeys) = 0;

    // creates a partially signed raw transaction to withdraw funds
    // txToSourceId: transaction id that was used to fund the pool
    // sourceAddress: used to know which outputs of the tx we can spend (in case it came from a SendMany() tx)
    // redeemScript [optional]: is passed to the API to do some bitcoin magic. required if unknown or if passing a signingAddress
    // signingAddress [optional]: only this address's private key will be used to sign the transaction (kinda pointless in our case)
    virtual BtcSignedTransactionRef VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const std::string &redeemScript = "", const std::string &signingAddress = "") = 0;

    // combines concatenated raw transactions to one raw transaction
    // used to combine the partially signed raw tx's when releasing multi-sig
    virtual BtcSignedTransactionRef CombineTransactions(const std::string &concatenatedRawTransactions) = 0;

    virtual std::string SendRawTransaction(const std::string &rawTransaction) = 0;

    /*
        std::map<std::string, int64_t> GetAddressesAndBalances() = 0;
        int64_t GetBalanceAtAddress(const std::string & str_address) = 0;
        not implemented yet, but bitcoin now offers a nice API function to get that, so if we really need it I can add it almost instantly

        int64_t GetMultisigBalance(  ) = 0;
        not yet implemented, would be nice if it isn't needed.
        instead we could/should keep track of transaction Ids and their value to know the balance of multi-sig addresses
        and clients should tell the server what the tx id is
    */
};

typedef std::shared_ptr<IMTBitcoin> IMTBitcoinRef;

#endif // IMTBITCOIN_H
