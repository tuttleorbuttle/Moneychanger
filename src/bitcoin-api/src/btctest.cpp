#include "btctest.h"
#include "btcmodules.h"
#include <thread>
//#include <OTLog.h>


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

    if(!TestRawTransactions())
        return false;

    if(!TestMultiSig())
        return false;

    return true;
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

bool BtcTest::TestRawTransactions()
{
    BitcoinServerRef bitcoind1 = BitcoinServerRef(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    // connect to server (if this function succeeds, SendRpc() works too)
    if(!modules->btcRpc->ConnectToBitcoin(bitcoind1))
        return false;

    // receive to address
    std::string myAddress1 = modules->mtBitcoin->GetNewAddress("test");
    if(myAddress1.empty())
        return false;

    // send to receiving address
    std::string txId = modules->mtBitcoin->SendToAddress(myAddress1, BtcHelper::CoinsToSatoshis(1));
    if(txId.empty())
        return false;

    // further send funds to other address:
    std::string myAddress2 = modules->mtBitcoin->GetNewAddress("test");
    if(myAddress2.empty())
        return false;

    std::list<BtcOutput> unspendOutputList;
    BtcRawTransactionRef decodedTx = modules->btcJson->GetDecodedRawTransaction(txId);
    if(decodedTx == NULL)
        return false;

    for(uint i = 0; i < decodedTx->outputs.size(); i++)
    {
        if(decodedTx->outputs[i].addresses[0] == myAddress1)
            unspendOutputList.push_back(BtcOutput(txId, i));
    }

    std::map<std::string, int64_t> targetList;
    targetList[myAddress2] = BtcHelper::CoinsToSatoshis(1);
    // spend the received funds
    std::string rawTx = modules->btcJson->CreateRawTransaction(unspendOutputList, targetList);
    if(rawTx.empty())
        return false;
    BtcSignedTransactionRef signedTx = modules->btcJson->SignRawTransaction(rawTx);
    if(signedTx == NULL)
        return false;
    if(!signedTx->complete)
        return false;

    std::string rawTxId = modules->btcJson->SendRawTransaction(signedTx->signedTransaction);
    if(rawTxId.empty())
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
    // create two modules so we don't have to change connections all the time
    BtcModules module1;
    BtcModules module2;
    module1.btcRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    module2.btcRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);

    // public keys of the addresses used for multisig
    std::list<std::string> keys;

    // GetNewAddress on bitcoind #1
    std::string address1 = module1.mtBitcoin->GetNewAddress();
    if(address1.empty())
        return false;
    keys.push_back(module1.mtBitcoin->GetPublicKey(address1));

    // GetNewAddress on bitcoind #2
    std::string address2 = module2.mtBitcoin->GetNewAddress();
    if(address2.empty())
        return false;
    keys.push_back(module2.mtBitcoin->GetPublicKey(address2));

    // add address to bitcoind #1 wallet
    multiSigAddress = module1.mtBitcoin->GetMultiSigAddress(2, keys, true, "test");
    if(multiSigAddress.empty())
        return false;

    // add address to bitcoind #2 wallet
    // if we don't add it to the wallet, we'd have to pass additional arguments to CreateRawTransaction later
    module2.mtBitcoin->GetMultiSigAddress(2, keys, true, "test");



    // send from bitcoind #1 to multisig
    int64_t amountToSend = BtcHelper::CoinsToSatoshis(1.22);
    std::string txId = module1.mtBitcoin->SendToAddress(multiSigAddress, amountToSend);
    if(txId.empty())
        return false;
    depositTxId = txId;    // need to remember this for later;

    // get an object containing the decoded raw transaction data
    BtcRawTransactionRef txToMultiSig = module1.mtBitcoin->WaitGetRawTransaction(txId);
    if(txToMultiSig == NULL)
        return false;

    // wait for confirmations and check for correct amount
    while(!module1.mtBitcoin->TransactionSuccessfull(amountToSend, txToMultiSig, multiSigAddress, 1))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return true;
}

bool BtcTest::TestMultiSigWithdrawal()
{
    BtcModules module1;
    BtcModules module2;
    module1.btcRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    module2.btcRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);

    // count how much btc we received in the deposit transaction
    int64_t amountToRelease = module1.btcHelper->GetTotalOutput(depositTxId, multiSigAddress);

    // now get ready to release to bitcoind #2.

    // create address to receive btc
    std::string targetAddress = module2.mtBitcoin->GetNewAddress();
    if(targetAddress.empty())
        return false;

    std::printf("releasing escrow from Tx #%s,\n address %s\nto address %s\n", depositTxId.c_str(), multiSigAddress.c_str(), targetAddress.c_str());
    std::cout.flush();

    // bitcoind #2 votes to release coins to his own address
    // the optional arguments redeemScript and signingAddress are only needed to
    // sign offline transactions or if depositTxId has no confirmations yet
    BtcSignedTransactionRef partialTx2 = module2.mtBitcoin->VoteMultiSigRelease(depositTxId, multiSigAddress, targetAddress);
    if(partialTx2 == NULL)
        return false;

    // bitcoind #1 votes to release coins to #2's address
    BtcSignedTransactionRef partialTx1 = module1.mtBitcoin->VoteMultiSigRelease(depositTxId, multiSigAddress, targetAddress);
    if(partialTx1 == NULL)
        return false;

    // combine both partially signed transactions into one
    std::string rawTxString = partialTx1->signedTransaction + partialTx2->signedTransaction;
    BtcSignedTransactionRef completeTx = module1.mtBitcoin->CombineTransactions(rawTxString);
    if(completeTx == NULL || !completeTx->complete)
        return false;

    std::string releaseTxId = module1.mtBitcoin->SendRawTransaction(completeTx->signedTransaction);
    if(releaseTxId.empty())
        return false;

    BtcRawTransactionRef releaseTx = module2.mtBitcoin->WaitGetRawTransaction(releaseTxId);
    if(releaseTx == NULL)
        return false;   

    // wait for confirmations and correct amount
    while(!module2.mtBitcoin->TransactionSuccessfull(amountToRelease, releaseTx, multiSigAddress, 1))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return true;
}


// TODO:

// for the lulz of it, see what would happen if
