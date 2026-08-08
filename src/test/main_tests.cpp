// Copyright (c) 2014-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <validation.h>
#include <net.h>

#include <test/test_bitcoin.h>

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

static void TestMainnetBlockSubsidySchedule(const Consensus::Params& consensusParams)
{
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1, consensusParams), 320000 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(consensusParams.nLastPOWBlock, consensusParams), 320000 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(consensusParams.nLastPOWBlock + 1, consensusParams), 800 * COIN);

    BOOST_CHECK_EQUAL(GetBlockSubsidy(1699999, consensusParams), 800 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1700000, consensusParams), 400 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(2499999, consensusParams), 400 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(2500000, consensusParams), 200 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(3499999, consensusParams), 200 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(3500000, consensusParams), 100 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(4499999, consensusParams), 100 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(4500000, consensusParams), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(5499999, consensusParams), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(5500000, consensusParams), 25 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(6499999, consensusParams), 25 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(6500000, consensusParams), 10 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(10000000, consensusParams), 10 * COIN);
}

static void TestBlockSubsidyHalvings(int nSubsidyHalvingInterval)
{
    Consensus::Params consensusParams;
    consensusParams.nSubsidyHalvingInterval = nSubsidyHalvingInterval;
    consensusParams.nLastPOWBlock = 0;

    int maxHalvings = 7;
    CAmount nInitialSubsidy = 800 * COIN;
    CAmount nPreviousSubsidy = nInitialSubsidy * 2; // for height == LastPoWBlock + 1

    for (int nHalvings = 0; nHalvings < maxHalvings; nHalvings++) {
        int nHeight = nHalvings * consensusParams.nSubsidyHalvingInterval + consensusParams.nLastPOWBlock + 1;
        CAmount nSubsidy = GetBlockSubsidy(nHeight, consensusParams);
        BOOST_CHECK(nSubsidy <= nInitialSubsidy);
        BOOST_CHECK_EQUAL(nSubsidy, nPreviousSubsidy / 2);
        nPreviousSubsidy = nSubsidy;
    }
    BOOST_CHECK_EQUAL(GetBlockSubsidy(maxHalvings * consensusParams.nSubsidyHalvingInterval + consensusParams.nLastPOWBlock + 1, consensusParams), 0);
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto mainChainParams = CreateChainParams(CBaseChainParams::MAIN);
    TestMainnetBlockSubsidySchedule(mainChainParams->GetConsensus()); // As in main
    const auto testnetChainParams = CreateChainParams(CBaseChainParams::TESTNET);
    TestMainnetBlockSubsidySchedule(testnetChainParams->GetConsensus()); // Same absolute schedule on testnet
    TestBlockSubsidyHalvings(150); // As in regtest
    TestBlockSubsidyHalvings(1000); // Just another interval
}

static void TestSubsidyLimitSchedule(const Consensus::Params& consensusParams, uint64_t expectedSupply)
{
    CAmount nSum = 0;
    for (int nHeight = 1; nHeight <= 10000000; nHeight++) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, consensusParams);

        if(nHeight <= consensusParams.nLastPOWBlock){
            BOOST_CHECK_EQUAL(nSubsidy, (320000 * COIN));
        }
        else if(nHeight < 1700000){
            BOOST_CHECK_EQUAL(nSubsidy, 800 * COIN);
        }
        else if(nHeight < 2500000){
            BOOST_CHECK_EQUAL(nSubsidy, 400 * COIN);
        }
        else if(nHeight < 3500000){
            BOOST_CHECK_EQUAL(nSubsidy, 200 * COIN);
        }
        else if(nHeight < 4500000){
            BOOST_CHECK_EQUAL(nSubsidy, 100 * COIN);
        }
        else if(nHeight < 5500000){
            BOOST_CHECK_EQUAL(nSubsidy, 50 * COIN);
        }
        else if(nHeight < 6500000){
            BOOST_CHECK_EQUAL(nSubsidy, 25 * COIN);
        }
        else{
            BOOST_CHECK_EQUAL(nSubsidy, 10 * COIN);
        }
        nSum += nSubsidy;
        BOOST_CHECK(MoneyRange(nSubsidy));
    }
    BOOST_CHECK_EQUAL(nSum, expectedSupply);
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto mainChainParams = CreateChainParams(CBaseChainParams::MAIN);
    TestSubsidyLimitSchedule(mainChainParams->GetConsensus(), 1006999921000000000ULL);
    const auto testnetChainParams = CreateChainParams(CBaseChainParams::TESTNET);
    TestSubsidyLimitSchedule(testnetChainParams->GetConsensus(), 368599921000000000ULL);
}

static bool ReturnFalse() { return false; }
static bool ReturnTrue() { return true; }

BOOST_AUTO_TEST_CASE(test_combiner_all)
{
    boost::signals2::signal<bool (), CombinerAll> Test;
    BOOST_CHECK(Test());
    Test.connect(&ReturnFalse);
    BOOST_CHECK(!Test());
    Test.connect(&ReturnTrue);
    BOOST_CHECK(!Test());
    Test.disconnect(&ReturnFalse);
    BOOST_CHECK(Test());
    Test.disconnect(&ReturnTrue);
    BOOST_CHECK(Test());
}
BOOST_AUTO_TEST_SUITE_END()
