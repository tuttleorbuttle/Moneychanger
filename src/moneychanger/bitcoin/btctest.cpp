#include "btctest.h"
#include "btcmodules.h"
#include <thread>


BtcModulesRef BtcTest::modules;
std::string BtcTest::multiSigAddress;
std::string BtcTest::depositTxId;

BtcTest::BtcTest()
{
    modules = NULL;

    depositTxId = "";
    multiSigAddress = "";
}

bool BtcTest::TestBitcoinFunctions()
{
    modules = BtcModulesRef(new BtcModules());

    if(modules == NULL ||
            modules->btcRpc == NULL ||
            modules->btcJson == NULL ||
            modules->btcHelper == NULL ||
            modules->mtBitcoin == NULL)
    {
        return false;
    }

    if(!TestBtcRpc())
        return false;

    if(!TestBtcJson())
        return false;

    if(!TestMultiSig())
        return false;

    return true;
}

bool BtcTest::TestMultiSig()
{
    if(!TestMultiSigDeposit())
        return false;

    if(!TestMultiSigWithdrawal())
        return false;

    return true;
}

bool BtcTest::TestMultiSigDeposit()
{
    BitcoinServerRef bitcoind1 = BitcoinServerRef(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));
    BitcoinServerRef bitcoind2 = BitcoinServerRef(new BitcoinServer("admin2", "123", "http://127.0.0.1", 19011));

    // public keys of the addresses used for multisig
    std::list<std::string> keys;

    // connect to server #1
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // GetNewAddress
    std::string address1 = modules->mtBitcoin->GetNewAddress();
    if(address1.empty())
        return false;
    keys.push_back(modules->mtBitcoin->GetPublicKey(address1));

    // connect to server #2
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind2))
        return false;

    std::string address2 = modules->mtBitcoin->GetNewAddress();
    if(address2.empty())
        return false;
    keys.push_back(modules->mtBitcoin->GetPublicKey(address2));

    // connect to server #1
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    std::string multiSigAddr = modules->mtBitcoin->GetMultiSigAddress(2, keys, true, "test");
    if(multiSigAddr.empty())
        return false;

    // send from server #1 to multisig
    int64_t amountToSend = BtcHelper::CoinsToSatoshis(1.22);
    std::string txId = modules->mtBitcoin->SendToAddress(amountToSend, multiSigAddr);
    if(txId.empty())
        return false;
    depositTxId = txId;    // need to remember this for later;

    // get an object containing the decoded raw transaction data
    BtcRawTransactionRef txToMultiSig = modules->mtBitcoin->WaitGetRawTransaction(txId);
    if(txToMultiSig == NULL)
        return false;

    // wait for confirmations and correct amount
    while(!modules->mtBitcoin->TransactionSuccessfull(amountToSend, txToMultiSig, multiSigAddr, 1))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return true;
}

bool BtcTest::TestMultiSigWithdrawal()
{
    BitcoinServerRef bitcoind1 = BitcoinServerRef(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));
    BitcoinServerRef bitcoind2 = BitcoinServerRef(new BitcoinServer("admin2", "123", "http://127.0.0.1", 19011));

    std::string txId = depositTxId;
    std::string multiSigAddr = multiSigAddress;

    // count how much btc we received in that txId
    int64_t amountToRelease = modules->btcHelper->GetTotalOutput(txId, multiSigAddr);


    // now get ready to withdraw to server #2.

    // connect to server #2
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind2))
        return false;

    // create address to receive btc
    std::string targetAddress = modules->mtBitcoin->GetNewAddress();
    if(targetAddress.empty())
        return false;

    // server #2 votes to release coins to his own address
    // the optional arguments redeemScript and signingAddress are only needed for offline signing
    // or if txId has no confirmations yet
    BtcSignedTransactionRef partialTx2 = modules->mtBitcoin->VoteMultiSigRelease(txId, multiSigAddr, targetAddress);
    if(partialTx2 == NULL)
        return false;

    // connect to server #2
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind2))
        return false;

    // server #1 votes to release coins to #2's address
    BtcSignedTransactionRef partialTx1 = modules->mtBitcoin->VoteMultiSigRelease(txId, multiSigAddr, targetAddress);
    if(partialTx1 == NULL)
        return false;

    // combine both partially signed transactions into one
    std::string rawTxString = partialTx1->signedTransaction.append(partialTx2->signedTransaction);
    BtcSignedTransactionRef completeTx = modules->mtBitcoin->CombineTransactions(rawTxString);
    if(completeTx == NULL || !completeTx->complete)
        return false;

    std::string releaseTxId = modules->mtBitcoin->SendRawTransaction(completeTx->signedTransaction);
    if(releaseTxId.empty())
        return false;

    BtcRawTransactionRef releaseTx = modules->mtBitcoin->WaitGetRawTransaction(releaseTxId);
    if(releaseTx == NULL)
        return false;

    // wait for confirmations and correct amount
    while(!modules->mtBitcoin->TransactionSuccessfull(amountToRelease, releaseTx, multiSigAddr, 1))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}



bool BtcTest::TestBtcRpc()
{
    // first testnet server:
    BitcoinServerRef bitcoind1 = BitcoinServerRef(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    // connect to server (if this function succeeds, SendRpc() works too)
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    if(modules->btcRpc->ConnectToBitcoin(NULL))
        return false;

    if(modules->btcRpc->ConnectToBitcoin("wronguser", "wrongpw","127.0.0.1",19001))
        return false;

    if(modules->btcRpc->ConnectToBitcoin("","", "127.0.0.1", 123))
        return false;

    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // try sending some garbage and see if anything crashes
    modules->btcRpc->SendRpc(NULL);

    modules->btcRpc->SendRpc("garbage");

    modules->btcRpc->SendRpc('\0');

    modules->btcRpc->SendRpc(BtcRpcPacketRef(new BtcRpcPacket('\0',1)));

    modules->btcRpc->SendRpc(BtcRpcPacketRef(new BtcRpcPacket('\0',0)));

    return true;    // not crashing is enough to pass this test
}

bool BtcTest::TestBtcJson()
{
    BitcoinServerRef bitcoind1 = BitcoinServerRef(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    // connect to server
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // see if we can get some response:
    std::string address = modules->btcJson->GetNewAddress();
    if(address.empty())
        return false;

    // disconnect
    modules->btcRpc->ConnectToBitcoin(BitcoinServerRef(new BitcoinServer("bla", "bla", "url", 123)));

    // try again
    address = modules->btcJson->GetNewAddress();
    if(!address.empty())
        return false;

    // TODO: Test all the other functions. Nah, they should be safe to use.

    return true;
}

