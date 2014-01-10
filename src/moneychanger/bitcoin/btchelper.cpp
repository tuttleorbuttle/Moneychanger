#include "btchelper.h"
#include "btcmodules.h"
#include <thread>
#include <vector>
#include <algorithm>


BtcHelper::BtcHelper(BtcModules *modules)
{
    MinConfirms = 1;

    this->modules = modules;
}

// converts a double bitcoin (as received through json api) to int64 satoshis
int64_t BtcHelper::CoinsToSatoshis(double value)
{
    // copied from the wiki
    return (int64_t)(value * 1e8 + (value < 0.0 ? -.5 : .5));
}

// converts int64 satoshis to double bitcoin
double BtcHelper::SatoshisToCoins(int64_t value)
{
    return (double)value / 1e8;
}


int64_t BtcHelper::GetConfirmations(BtcRawTransactionRef transaction)
{
    // firstly, see if the transaction isn't included in a block yet
    std::vector<std::string> rawMemPool = this->modules->btcJson->GetRawMemPool();
    if(std::find(rawMemPool.begin(), rawMemPool.end(), transaction->txID) != rawMemPool.end())
        return 0;    // 0 confirmations if still in mempool

    // if it is, we'll need to loop through the latest blocks until we find it
    // getblockcount --> getblockhash(count) --> getblock(hash) --> getblock(block->previous) -->...
    // get number of latest block
    int latestBlock = this->modules->btcJson->GetBlockCount();
    // get hash of latest block
    std::string blockHash = this->modules->btcJson->GetBlockHash(latestBlock);
    // get the actual block block
    BtcBlockRef currentBlock = this->modules->btcJson->GetBlock(blockHash);
    // the block might not be downloaded yet, in that case return
    if(currentBlock == NULL)
        return 0;

    int64_t confirmations = 1;  // first block = first confirmation

    while(std::find(currentBlock->transactions.begin(), currentBlock->transactions.end(), transaction->txID) == currentBlock->transactions.end())
    {
        confirmations++;      // count down minconfirms

        currentBlock = this->modules->btcJson->GetBlock(currentBlock->previousHash);
        if(currentBlock == NULL)
            return 0;   // this should never happen I think. except maybe during forks
    }

    // if we find it in an old enough block, return number of confirmations
    return confirmations;
}

BtcRawTransactionRef BtcHelper::WaitGetRawTransaction(const std::string &txID, int timerMS, int maxAttempts)
{
    BtcRawTransactionRef transaction;

    // TODO: see WaitGetTransaction()
    while((transaction = this->modules->btcJson->GetDecodedRawTransaction(txID)) == NULL && maxAttempts--)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(timerMS));
        //std::this_thread::yield();
    }

    return transaction;
}

BtcSignedTransactionRef BtcHelper::WithdrawAllFromAddress(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const std::string &redeemScript, const std::string &signingAddress)
{
    // retrieve decoded raw transaction sending funds to our sourceAddress
    BtcRawTransactionRef rawTransaction = this->modules->btcJson->GetDecodedRawTransaction(txToSourceId);
    if(rawTransaction == NULL)
        return BtcSignedTransactionRef();   // return NULL

    // count funds in source address and list outputs leading to it
    // this will only work when no btc from source tx have been spent from sourceAddress (the multi-sig address) yet
    int64_t funds = 0;
    std::list<BtcOutput> unspentOutputs;
    std::list<BtcSigningPrequisite> signingPrequisites;
    for(int i = 0; i < rawTransaction->outputs.size(); i++)
    {
        BtcRawTransaction::VOUT output = rawTransaction->outputs[i];
        // check if output leads to sourceAddess
        // idk what multiple addresses per output means so we'll skip those cases
        if(output.addresses.size() == 1 && output.addresses[0] == sourceAddress)
        {
            funds += output.value;
            unspentOutputs.push_back(BtcOutput(txToSourceId, output.n));

            // create signing prequisite (optional)
            if(!signingAddress.empty() && !redeemScript.empty())
                signingPrequisites.push_back(BtcSigningPrequisite(txToSourceId, output.n, output.scriptPubKeyHex, redeemScript));
        }
    }

    // create raw transaction to send outputs to target address
    std::map<std::string, int64_t> txTargets;
    txTargets[destinationAddress] = funds;  // TODO: calculate fee
    std::string withdrawTransaction = this->modules->btcJson->CreateRawTransaction(unspentOutputs, txTargets);

    // only sign transaction with signingAddress's privkey
    // this only works in multi-sig if redeemScript is passed aswell
    std::stringList signingKeys;
    if(!signingAddress.empty() && !redeemScript.empty())
    {
        signingKeys.push_back(this->modules->btcJson->GetPrivateKey(signingAddress));
    }

    // Note: signingPrequisites can be empty, in that case bitcoin will sign with any key that fits.
    // this should only be done with locally generated transactions or transactions whose outputs were checked first
    // because otherwise someone could give you a transaction to send funds from your wallet to his and you'd blindly sign it.
    // if a signingAddress is passed, redeemScript is required aswell.

    // sign raw transaction
    // as we just created this tx ourselves, we can assume that it is safe to sign
    BtcSignedTransactionRef signedTransact = this->modules->btcJson->SignRawTransaction(withdrawTransaction, signingPrequisites, signingKeys);

    return signedTransact;
}
