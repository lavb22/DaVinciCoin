// Copyright (c) 2009-2012 Bitcoin Developers
// Copyright (c) 2012-2017 The Peercoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "main.h"
#include "sync.h"
#include "wallet.h"

#include <fstream>
#include <stdint.h>

#include <boost/lexical_cast.hpp>

#define printf OutputDebugStringF

using namespace json_spirit;
using namespace std;

class CTxDump
{
public:
    CBlockIndex *pindex;
    int64 nValue;
    bool fSpent;
    CWalletTx* ptx;
    int nOut;
    CTxDump(CWalletTx* ptx = NULL, int nOut = -1)
    {
        pindex = NULL;
        nValue = 0;
        fSpent = false;
        this->ptx = ptx;
        this->nOut = nOut;
    }
};

Value importprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "importprivkey <davincicoinprivkey> [label] [rescan=true]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet.");

    string strSecret = params[0].get_str();
    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();

    // Whether to perform rescan after import
    bool fRescan = true;
    if (params.size() > 2)
        fRescan = params[2].get_bool();

    CDavincicoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    if (fWalletUnlockMintOnly) // davincicoin: no importprivkey in mint-only mode
        throw JSONRPCError(-102, "Wallet is unlocked for minting only.");

    CKey key;
    bool fCompressed;
    CSecret secret = vchSecret.GetSecret(fCompressed);
    key.SetSecret(secret, fCompressed);
    CKeyID vchAddress = key.GetPubKey().GetID();
    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();
        pwalletMain->SetAddressBookName(vchAddress, strLabel);

        if (!pwalletMain->AddKey(key))
            throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
	
        if (fRescan) {
            pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
            pwalletMain->ReacceptWalletTransactions();
        }
    }

    return Value::null;
}

void ImportScript(const CScript& script, const std::string& strLabel, const CKeyID& AddID)
{
	if (IsMine(*pwalletMain,script)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "The wallet already contains this address or script");
	}
	pwalletMain->MarkDirty();
		if (!pwalletMain->AddWatchOnly(script, AddID)) {
				throw JSONRPCError(RPC_WALLET_ERROR, "Error adding address to wallet");
			}
	pwalletMain->SetAddressBookName(AddID, strLabel);
}

Value importaddress(const Array& params, bool fHelp)
{
	if (fHelp || params.size() < 1 || params.size() > 4)

		throw std::runtime_error(
				"importaddress <address> [label] [rescan=true] \n"
				"\nAdds a script (in hex) or address that can be watched as if it were in your wallet but cannot be used to spend.\n"
				"\nArguments:\n"
				"1. \"script\" (string, required) The hex-encoded script (or address)\n"
				"2. \"label\" (string, optional, default=\"\") An optional label\n"
				"3. rescan (boolean, optional, default=true) Rescan the wallet for transactions\n"
				"\nNote: This call can take minutes to complete if rescan is true.\n"
);

	std::string strLabel = "";

	if (params.size() > 1)
		strLabel = params[1].get_str();

	// Whether to perform rescan after import

	bool fRescan = true;

	if (params.size() > 2)
		fRescan = params[2].get_bool();

	LOCK2(cs_main, pwalletMain->cs_wallet);

	CDavincicoinAddress coinAdd;

	coinAdd.SetString(params[0].get_str());

	CKeyID dest;

	if (coinAdd.IsValid() && coinAdd.GetKeyID(dest)) {

		CScript scriptAdd;

		scriptAdd.SetDestination(dest);

		ImportScript(scriptAdd, strLabel, dest);
	} else {

		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Davincicoin address");
	}
	if (fRescan)
	{
		pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
		pwalletMain->ReacceptWalletTransactions();
	}
	return Value::null;
}

Value dumpprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "dumpprivkey <davincicoinaddress>\n"
            "Reveals the private key corresponding to <davincicoinaddress>.");

    string strAddress = params[0].get_str();
    CDavincicoinAddress address;
    if (!address.SetString(strAddress))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid davincicoin address");
    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
    if (fWalletUnlockMintOnly) // davincicoin: no dumpprivkey in mint-only mode
        throw JSONRPCError(-102, "Wallet is unlocked for minting only.");

    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    CSecret vchSecret;
    bool fCompressed;
    if (!pwalletMain->GetSecret(keyID, vchSecret, fCompressed))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");
    return CDavincicoinSecret(vchSecret, fCompressed).ToString();
}
