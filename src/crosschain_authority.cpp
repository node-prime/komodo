#include "cc/eval.h"
#include "crosschain.h"
#include "notarisationdb.h"
//#include "notaries_STAKED.h"

const char *notaries_STAKEDcc[][2] =
{
    {"blackjok3r", "021914947402d936a89fbdd1b12be49eb894a1568e5e17bb18c8a6cffbd3dc106e" }, // RTVti13NP4eeeZaCCmQxc2bnPdHxCJFP9x
    {"alright", "0285657c689b903218c97f5f10fe1d10ace2ed6595112d9017f54fb42ea1c1dda8" }, //RXmXeQ8LfJK6Y1aTM97cRz9Gu5f6fmR3sg
    {"webworker01", "031d1fb39ae4dca28965c3abdbd21faa0f685f6d7b87a60561afa7c448343fef6d" }, //RGsQiArk5sTmjXZV9UzGMW5njyvtSnsTN8
    {"CrisF", "03f87f1bccb744d90fdbf7fad1515a98e9fc7feb1800e460d2e7565b88c3971bf3" }, //RMwEpnaVe3cesWbMqqKYPPkaLcDkooTDgZ
    {"smk762", "02eacef682d2f86e0103c18f4da46116e17196f3fb8f73ed931acb78e81d8e1aa5" }, // RQVvzJ8gepCDVjhqCAc5Tia1kTmt8KDPL9
    {"jorian", "02150c410a606b898bcab4f083e48e0f98a510e0d48d4db367d37f318d26ae72e3" }, // RFgzxZe2P4RWKx6E9QGPK3rx3TXeWxSqa8
    {"TonyL", "021a559101e355c907d9c553671044d619769a6e71d624f68bfec7d0afa6bd6a96" }, // RHq3JsvLxU45Z8ufYS6RsDpSG4wi6ucDev
    {"Emman", "038f642dcdacbdf510b7869d74544dbc6792548d9d1f8d73a999dd9f45f513c935" }, //RN2KsQGW36Ah4NorJDxLJp2xiYJJEzk9Y6
    {"titomane", "0222d144bbf15280c574a0ccdc4f390f87779504692fef6e567543c03aa057dfcf" }, //RUS5jcf55AupeEDjotmKLeXnLZye2rwoUg
    {"CHMEX", "03ed125d1beb118d12ff0a052bdb0cee32591386d718309b2924f2c36b4e7388e6" }, // RF4HiVeuYpaznRPs7fkRAKKYqT5tuxQQTL
    {"metaphilibert", "0344182c376f054e3755d712361672138660bda8005abb64067eb5aa98bdb40d10" }, // RG28QSnYFADBg1dAVkH1uPGYS6F8ioEUM2
    {"jusoaresf", "02dfb7ed72a23f6d07f0ea2f28192ee174733cc8412ec0f97b073007b78fab6346" }, // RBQGfE5Hxsjm1BPraTxbneRuNasPDuoLnu
    {"mylo", "03f6b7fcaf0b8b8ec432d0de839a76598b78418dadd50c8e5594c0e557d914ec09" }, // RXN4hoZkhUkkrnef9nTUDw3E3vVALAD8Kx
    {"blackjok3r2", "02f7597468703c1c5c8465dd6d43acaae697df9df30bed21494d193412a1ea193e" }, // RWHGbrLSP89fTzNVF9U9xiekDYJqcibTca
    {"blackjok3r3", "03c3e4c0206551dbf3a4b24d18e5d2737080541184211e3bfd2b1092177410b9c2" }, // RMMav2AVse5XHPvDfTzRpMbFhK3GqFmtSN
};


int GetSymbolAuthority(const char* symbol)
{
    if (strlen(symbol) >= 5 && strncmp(symbol, "TXSCL", 5) == 0)
        return CROSSCHAIN_TXSCL;
    if ( (strlen(symbol) >= 4 && strncmp(symbol, "STKD", 4) == 0) || (strlen(symbol) >= 6 && strncmp(symbol, "STAKED", 6) == 0) )
        return CROSSCHAIN_STAKED;
    return CROSSCHAIN_KOMODO;
}


bool CheckTxAuthority(const CTransaction &tx, CrosschainAuthority auth)
{
    EvalRef eval;

    if (tx.vin.size() < auth.requiredSigs) return false;

    uint8_t seen[64] = {0};

    BOOST_FOREACH(const CTxIn &txIn, tx.vin)
    {
        // Get notary pubkey
        CTransaction tx;
        uint256 hashBlock;
        if (!eval->GetTxUnconfirmed(txIn.prevout.hash, tx, hashBlock)) return false;
        if (tx.vout.size() < txIn.prevout.n) return false;
        CScript spk = tx.vout[txIn.prevout.n].scriptPubKey;
        if (spk.size() != 35) return false;
        const unsigned char *pk = spk.data();
        if (pk++[0] != 33) return false;
        if (pk[33] != OP_CHECKSIG) return false;

        // Check it's a notary
        for (int i=0; i<auth.size; i++) {
            if (!seen[i]) {
                if (memcmp(pk, auth.notaries[i], 33) == 0) {
                    seen[i] = 1;
                    goto found;
                }
            }
        }
        return false;
        found:;
    }

    return true;
}


const CrosschainAuthority auth_STAKED = [&](){
    CrosschainAuthority auth;
    auth.size = (int32_t)(sizeof(notaries_STAKEDcc)/sizeof(*notaries_STAKEDcc));
    auth.requiredSigs = (auth.size/5);
    for (int n=0; n<auth.size; n++)
        for (size_t i=0; i<33; i++)
            sscanf(notaries_STAKEDcc[n][1]+(i*2), "%2hhx", auth.notaries[n]+i);
    return auth;
}();