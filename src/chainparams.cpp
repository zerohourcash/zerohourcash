// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <consensus/consensus.h>
#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <util/convert.h>
#include <versionbitsinfo.h>

#include <assert.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

///////////////////////////////////////////// // zerohour
#include <libdevcore/SHA3.h>
#include <libdevcore/RLP.h>
#include "arith_uint256.h"
/////////////////////////////////////////////




static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 520159231 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    genesis.hashStateRoot = uint256(h256Touint(dev::h256("e965ffd002cd6ad0e2dc402b8044de833e06b23127ea8c3d80aec91410771495"))); // zerohour
    genesis.hashUTXORoot = uint256(h256Touint(dev::sha3(dev::rlp("")))); // zerohour
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "On the calendar is a great fall day to launch ZeroHour";
    const CScript genesisOutputScript = CScript() << ParseHex("0450a256079d8a5e2c6b1e2ab3146a8b4d1eec14e3095bc71fa5b05e2a57efa8ccb6acf1ea069b8364f919c585037ada3b2fb4831bd2f63badc50be15315e41e41") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}


static void MineGenesis(CBlockHeader& genesisBlock, const uint256& powLimit)
{
    genesisBlock.nNonce = 0;

    std::cout << "NOTE: Genesis nTime: " << genesisBlock.nTime << std::endl;
    std::cout << "WARN: Genesis nNonce (BLANK!): " <<  genesisBlock.nNonce << std::endl;

    arith_uint256 besthash;
    memset(&besthash,0xFF,32);
    arith_uint256 hashTarget = UintToArith256(powLimit);
    std::cout << "Target: " << hashTarget.GetHex().c_str() << std::endl;
    arith_uint256 newhash = UintToArith256(genesisBlock.GetHash());
    while (newhash > hashTarget) {
        genesisBlock.nNonce++;
        if (genesisBlock.nNonce == 0) {
            std::cout << "NONCE WRAPPED, incrementing time" << std::endl;
            ++genesisBlock.nTime;
        }
        if ((genesisBlock.nNonce & 0xfff) == 0)
            std::cout << "nonce: " << genesisBlock.nNonce << " hash: " << newhash.ToString().c_str() << " target: " << hashTarget.ToString().c_str() << std::endl;

        if(newhash < besthash) {
            besthash = newhash;
            std::cout << "New best " << newhash.GetHex().c_str() << std::endl;
        }
        newhash = UintToArith256(genesisBlock.GetHash());
    }
    std::cout << "Genesis nTime " <<  genesisBlock.nTime << std::endl;
    std::cout << "Genesis nNonce " << genesisBlock.nNonce << std::endl;
    std::cout << "Genesis nBits " << genesisBlock.nBits << std::endl;
    std::cout << "Genesis Hash " <<  newhash.ToString().c_str() << std::endl;
    std::cout << "Genesis hashStateRoot " <<  genesisBlock.hashStateRoot.ToString().c_str() << std::endl;
    std::cout << "Genesis Hash Merkle Root " << genesisBlock.hashMerkleRoot.ToString().c_str() << std::endl;
}


/**
 * Main network
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 5256000; // zerohour halving every 20 years
        consensus.BIP16Exception = uint256S("0x00007af309bdd818599502f8fc8af0943c4ce302df2298b14e59abd0c38e07b0");
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x00007af309bdd818599502f8fc8af0943c4ce302df2298b14e59abd0c38e07b0");
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.QIP5Height = 0;
        consensus.QIP6Height = 0;
        consensus.QIP7Height = 0;
        consensus.QIP9Height = 30000;
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.QIP9PosLimit = uint256S("0000000000001fffffffffffffffffffffffffffffffffffffffffffffffffff"); // The new POS-limit activated after QIP9
        consensus.nPowTargetTimespan = 16 * 60; // 16 minutes
        consensus.nPowTargetTimespanV2 = 4000;
        consensus.nPowTargetSpacing = 2 * 64;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = true;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00007af309bdd818599502f8fc8af0943c4ce302df2298b14e59abd0c38e07b0");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0xa6;
        pchMessageStart[3] = 0xd3;
        nDefaultPort = 38100;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 6;
        m_assumed_chain_state_size = 2;

        genesis = CreateGenesisBlock(1575408600, 36302, 0x1f00ffff, 1, 0 * COIN);

        //MineGenesis(genesis, consensus.powLimit);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00007af309bdd818599502f8fc8af0943c4ce302df2298b14e59abd0c38e07b0"));
        assert(genesis.hashMerkleRoot == uint256S("0xfe63fd973a2423dab77a08e75e640b0dba49925d7d167af6cb8a1774f8b3ea13"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.


        vSeeds.emplace_back("hub1.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub2.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub3.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub4.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub5.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub6.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub7.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub8.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub9.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub10.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub11.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub12.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub13.zh.cash"); // ZHCASH mainnet
        vSeeds.emplace_back("hub14.zh.cash"); // ZHCASH mainnet

        vSeeds.emplace_back("hub1.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub2.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub3.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub4.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub5.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub6.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub7.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub8.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub9.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub10.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub11.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub12.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub13.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub14.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub15.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub16.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub17.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub18.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub19.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub20.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub21.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub22.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub23.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub24.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub25.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub26.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub27.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub28.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub29.zhchain.sbs"); // ZHCASH mainnet
        vSeeds.emplace_back("hub30.zhchain.sbs"); // ZHCASH mainnet

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,80);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,50);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "zh";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                { 0, uint256S("0x00007af309bdd818599502f8fc8af0943c4ce302df2298b14e59abd0c38e07b0")},
		{ 500, uint256S("0x0000ccb2366b663e8692e0ca33beb4cb8cf9b8bdde644576509ea6c928dec912")},
		{ 1000, uint256S("0x0000342d191e14e62c059301bce80d08577d326fdef20b10127c5fb714bd7232")},
		{ 5000, uint256S("0x000042ce1c9aa973256990480f9fc164b22cfa56e69de2e5310f48a23b7d6c8e")},
		{ 10000, uint256S("0x00003e4646c0f61da552557c36ff3f5463888704331187c6a61d58ed6d80e136")},
		{ 50000, uint256S("0xdcbe756dbf647c64c7851ce8b97f3ee2035e021224fd39ce0f4cb552b747dea8")},
		{ 100000, uint256S("0x354bbe24bdfb8b8c7fc2ad1eb92effec87b964aa22c89678d7c606881c705263")},
		{ 150000, uint256S("0x833b342e5693bf7aca589ca83633c30e3d837677c04f94a42613608677c78e23")},
		{ 200000, uint256S("0xc3f4826a31bb6bff68d30ff970c6e3b6d26f2996cf5e57ec34d624de6ce2f300")},
		{ 250000, uint256S("0xe297aaeee42787f9a6514fcb84b27429aebfd5f1ae3a4f57631a9014897cfa8b")},
		{ 300000, uint256S("0xd20bcc9919dda2c8babcb1a62d6c1692c6be8f59ca47bfd0b0c47ed8d10e830e")},
		{ 350000, uint256S("0x1ffa5a74005e5b72541667d8ef656239d0add3d782baecd8de5d773a8762e3b5")},
		{ 400000, uint256S("0xd23b55cdbc0df3c50bd9b29117ccac66e0821ef0d401257ee67fa84e37395ad3")},
		{ 450000, uint256S("0x9f69ad1261d60354136c7d09e7be4c1d6efc284384cfbd9998e64ab57ea87f33")},
		{ 500000, uint256S("0xd92d9a9f8d4b53572e04fbae4a26af70e47afb94186db8dffabd083f8b99d099")},
		{ 550000, uint256S("0xe3b3e1da01dfb41cccfc2a21262c84808fe27c3ef97d269335e399eb8eb2be07")},
		{ 600000, uint256S("0xc4d8400a5e2d7af03f081daa5236c68848ce58311a5ef0f82cf62e5b2d29d92f")},
		{ 630000, uint256S("0xdfebee56db92db6d3384738855489c4d4794cd4df1f83c1205aeddb11fb159f0")},
		{ 650000, uint256S("0x74ea8be47ee61d1755655016ba2c39b70912ed2b56e981fbbf29316091fa9cc0")},
		{ 700000, uint256S("0xe3a193c62ac43ce013e7ff9cb9bb7a600d312ad296b5c11c9b44623aca82c250")},
		{ 750000, uint256S("0xa1cf315d524b759bfecc489d9cc66556034ed8406f1bb2323de5ddb0e571569f")},
		{ 800000, uint256S("0x1aa33e90b87ba748e86a736cfa1148ceb9ff85df571fdf99e0efb6df457d8341")},
		{ 840000, uint256S("0x8472e70af680a3b0a940b5302b80de898c30ffb2ce635dfd91c5a019757c638f")},
		{ 900000, uint256S("0x15d8aefe5fb1ab5e5f529d77aa3e1e4454cc998b8442bf8aa7b236c8d3b675d3")},
		{ 940000, uint256S("0x7262afbb11a1bc3e08b42ad8dd94ad61d2bb0ec936df4140c4a9c8c5c5ee0ac6")},
		{ 1000000, uint256S("0x4b8db66d0de8136414f547ea3a1355bd840a01ec5b1c4ef2dae0f05a128fcaf9")},
		{ 1050000, uint256S("0x22e83b481953b376f891e126b5914a105ff28dae8987bd35bb51f4d099f66c45")},
		{ 1100000, uint256S("0x54a58516606476e7ef23d3369568081bd0d56d273fd9abe5fcdeebf03023354b")},
		{ 1150000, uint256S("0xae3e7c1078055dda3d905a97203505def7561a524b042377325478b91fcf1169")},
		{ 1200000, uint256S("0x60fa9791b04bd1f25a98f0bdd9ab265c550a78c1a9e74ddb2ef928ca8ea78b4e")},
		{ 1220000, uint256S("0x389fdec55ef9af23b051e8fc63e71835327c956e24b3cda0787bbfd4dc5ec678")},
		{ 1300000, uint256S("0x2062d80d2b65805f5e1af47b417c3e4ec6a9e884c02a0ae807a528a548b3b044")},
		{ 1400000, uint256S("0x51424ffb22ef99ee2400187ecd3a1ec3f969805462dec46fffe1e6fc643e1084")}
            }
        };

        chainTxData = ChainTxData{
            1692795424, // * UNIX timestamp of last known number of transactions
            2421511, // * total number of transactions between genesis and that timestamp
            0.02 // * estimated number of transactions per second after that timestamp
        };

        /* disable fallback fee on mainnet */
        m_fallback_fee_enabled = false;

        consensus.nLastPOWBlock = 25000;
        consensus.nMPoSRewardRecipients = 10;
        consensus.nFirstMPoSBlock = consensus.nLastPOWBlock + 
                                    consensus.nMPoSRewardRecipients + 
                                    COINBASE_MATURITY;

        consensus.nFixUTXOCacheHFHeight = 100000;
        consensus.nEnableHeaderSignatureHeight = 0;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 5256000; // zerohour halving every 20 years
        consensus.BIP16Exception = uint256S("0x0000387e3beaaeb366ef82c91be85c666aac71bcb9ea933878355c1f682fd42c");
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0000387e3beaaeb366ef82c91be85c666aac71bcb9ea933878355c1f682fd42c");
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.QIP5Height = 0;
        consensus.QIP6Height = 0;
        consensus.QIP7Height = 0;
        consensus.QIP9Height = 10000;
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.QIP9PosLimit = uint256S("0000000000001fffffffffffffffffffffffffffffffffffffffffffffffffff"); // The new POS-limit activated after QIP9
        consensus.nPowTargetTimespan = 16 * 60; // 16 minutes
        consensus.nPowTargetTimespanV2 = 4000;
        consensus.nPowTargetSpacing = 2 * 64;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = true;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000387e3beaaeb366ef82c91be85c666aac71bcb9ea933878355c1f682fd42c");

        pchMessageStart[0] = 0x0d;
        pchMessageStart[1] = 0x22;
        pchMessageStart[2] = 0x15;
        pchMessageStart[3] = 0x06;
        nDefaultPort = 38300;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 6;
        m_assumed_chain_state_size = 2;

        genesis = CreateGenesisBlock(1710871500, 38154, 0x1f00ffff, 1, 0 * COIN);
        
        //MineGenesis(genesis, consensus.powLimit);
        
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000387e3beaaeb366ef82c91be85c666aac71bcb9ea933878355c1f682fd42c"));
        assert(genesis.hashMerkleRoot == uint256S("0xfe63fd973a2423dab77a08e75e640b0dba49925d7d167af6cb8a1774f8b3ea13"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("tn1.zhchain.sbs"); // ZHCASH testnet
        vSeeds.emplace_back("tn2.zhchain.sbs"); // ZHCASH testnet
        vSeeds.emplace_back("tn3.zhchain.sbs"); // ZHCASH testnet
        vSeeds.emplace_back("tn4.zhchain.sbs"); // ZHCASH testnet
        vSeeds.emplace_back("tn5.zhchain.sbs"); // ZHCASH testnet
        vSeeds.emplace_back("tn6.zhchain.sbs"); // ZHCASH testnet
        vSeeds.emplace_back("tn7.zhchain.sbs"); // ZHCASH testnet

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,110);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "zht";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        checkpointData = {
            {
                {0, uint256S("0x0000387e3beaaeb366ef82c91be85c666aac71bcb9ea933878355c1f682fd42c")}
            }
        };

        chainTxData = ChainTxData{
            1710871500,
            0,
            0
        };

        /* enable fallback fee on testnet */
        m_fallback_fee_enabled = true;

        consensus.nLastPOWBlock = 5000;
        consensus.nMPoSRewardRecipients = 10;
        consensus.nFirstMPoSBlock = consensus.nLastPOWBlock + 
                                    consensus.nMPoSRewardRecipients + 
                                    COINBASE_MATURITY;

        consensus.nFixUTXOCacheHFHeight = 84500;
        consensus.nEnableHeaderSignatureHeight = 0;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 15000;
        consensus.BIP16Exception = uint256S("0x00ec36c6dcc7a537c4f46650729c663296300f46f83968036c1b6a85f5b0a248");
        consensus.BIP34Height = 0; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests) // activate for zerohour
        consensus.BIP34Hash = uint256S("0x00ec36c6dcc7a537c4f46650729c663296300f46f83968036c1b6a85f5b0a248");
        consensus.BIP65Height = 0; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 0; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.QIP5Height = 0;
        consensus.QIP6Height = 0;
        consensus.QIP7Height = 0;
        consensus.QIP9Height = 100;
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.QIP9PosLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // The new POS-limit activated after QIP9
        consensus.nPowTargetTimespan = 16 * 60; // 16 minutes (960 = 832 + 128; multiplier is 832)
        consensus.nPowTargetTimespanV2 = 4000;
        consensus.nPowTargetSpacing = 2 * 64;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.fPoSNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        //consensus.nMinimumChainWork = uint256S("0x0");
        consensus.nMinimumChainWork = uint256S("0x00000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00ec36c6dcc7a537c4f46650729c663296300f46f83968036c1b6a85f5b0a248");

        pchMessageStart[0] = 0xfd;
        pchMessageStart[1] = 0xdd;
        pchMessageStart[2] = 0xc6;
        pchMessageStart[3] = 0xe1;
        nDefaultPort = 38400;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateVersionBitsParametersFromArgs(args);

        genesis = CreateGenesisBlock(1575408400, 255, 0x1f00ffff, 1, 0 * COIN);
        
        //MineGenesis(genesis, consensus.powLimit);
        
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00ec36c6dcc7a537c4f46650729c663296300f46f83968036c1b6a85f5b0a248"));
        assert(genesis.hashMerkleRoot == uint256S("0xfe63fd973a2423dab77a08e75e640b0dba49925d7d167af6cb8a1774f8b3ea13"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("0x00ec36c6dcc7a537c4f46650729c663296300f46f83968036c1b6a85f5b0a248")},
            }
        };

        chainTxData = ChainTxData{
            1575408400,
            0,
            0
        };
        consensus.nLastPOWBlock = 0x7fffffff;
        consensus.nMPoSRewardRecipients = 10;
        consensus.nFirstMPoSBlock = 5000;

        consensus.nFixUTXOCacheHFHeight=0;
        consensus.nEnableHeaderSignatureHeight = 0;

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,122);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,110);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "zhrt";

        /* enable fallback fee on regtest */
        m_fallback_fee_enabled = true;
    }

    /**
     * Allows modifying the Version Bits regtest parameters.
     */
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
    void UpdateVersionBitsParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateVersionBitsParametersFromArgs(const ArgsManager& args)
{
    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() != 3) {
            throw std::runtime_error("Version bits parameters malformed, expecting deployment:start:end");
        }
        int64_t nStartTime, nTimeout;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        bool found = false;
        for (int j=0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j), nStartTime, nTimeout);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to start=%ld, timeout=%ld\n", vDeploymentParams[0], nStartTime, nTimeout);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

/**
 * Regression network parameters overwrites for unit testing
 */
class CUnitTestParams : public CRegTestParams
{
public:
    explicit CUnitTestParams(const ArgsManager& args)
    : CRegTestParams(args)
    {
        // Activate the the BIPs for regtest as in Bitcoin
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.QIP6Height = 1000;
        consensus.QIP7Height = 0; // QIP7 activated on regtest

        // ZHCASH have 500 blocks of maturity, increased values for regtest in unit tests in order to correspond with it
        consensus.nSubsidyHalvingInterval = 750;
        consensus.nRuleChangeActivationThreshold = 558; // 75% for testchains
        consensus.nMinerConfirmationWindow = 744; // Faster than normal for regtest (744 instead of 2016)
    }
};

static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams(gArgs));
    else if (chain == CBaseChainParams::UNITTEST)
        return std::unique_ptr<CChainParams>(new CUnitTestParams(gArgs));
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

std::string CChainParams::EVMGenesisInfo(dev::eth::Network network) const
{
    std::string genesisInfo = dev::eth::genesisInfo(network);
    ReplaceInt(consensus.QIP7Height, "QIP7_STARTING_BLOCK", genesisInfo);
    ReplaceInt(consensus.QIP6Height, "QIP6_STARTING_BLOCK", genesisInfo);
    return genesisInfo;
}

std::string CChainParams::EVMGenesisInfo(dev::eth::Network network, int nHeight) const
{
    std::string genesisInfo = dev::eth::genesisInfo(network);
    ReplaceInt(nHeight, "QIP7_STARTING_BLOCK", genesisInfo);
    ReplaceInt(nHeight, "QIP6_STARTING_BLOCK", genesisInfo);
    return genesisInfo;
}

void CChainParams::UpdateOpSenderBlockHeight(int nHeight)
{
    consensus.QIP5Height = nHeight;
}

void UpdateOpSenderBlockHeight(int nHeight)
{
    const_cast<CChainParams*>(globalChainParams.get())->UpdateOpSenderBlockHeight(nHeight);
}

void CChainParams::UpdateBtcEcrecoverBlockHeight(int nHeight)
{
    consensus.QIP6Height = nHeight;
}

void UpdateBtcEcrecoverBlockHeight(int nHeight)
{
    const_cast<CChainParams*>(globalChainParams.get())->UpdateBtcEcrecoverBlockHeight(nHeight);
}

void CChainParams::UpdateConstantinopleBlockHeight(int nHeight)
{
    consensus.QIP7Height = nHeight;
}

void UpdateConstantinopleBlockHeight(int nHeight)
{
    const_cast<CChainParams*>(globalChainParams.get())->UpdateConstantinopleBlockHeight(nHeight);
}

void CChainParams::UpdateDifficultyChangeBlockHeight(int nHeight)
{
    consensus.nSubsidyHalvingInterval = 5256000; // zerohour halving every 20 years
    consensus.posLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensus.QIP9Height = nHeight;
    consensus.fPowAllowMinDifficultyBlocks = false;
    consensus.fPowNoRetargeting = true;
    consensus.fPoSNoRetargeting = false;
    consensus.nLastPOWBlock = 25000;
    consensus.nMPoSRewardRecipients = 10;
    consensus.nFirstMPoSBlock = consensus.nLastPOWBlock + 
                                consensus.nMPoSRewardRecipients + 
                                COINBASE_MATURITY;
}

void UpdateDifficultyChangeBlockHeight(int nHeight)
{
    const_cast<CChainParams*>(globalChainParams.get())->UpdateDifficultyChangeBlockHeight(nHeight);
}
