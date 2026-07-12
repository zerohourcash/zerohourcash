// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_TEST_MODULE Bitcoin Test Suite

#include <banman.h>
#include <net.h>

#include <atomic>
#include <memory>

#include <boost/test/unit_test.hpp>

std::unique_ptr<CConnman> g_connman;
std::unique_ptr<BanMan> g_banman;
static std::atomic<bool> g_shutdown_requested{false};

void Shutdown(void* parg)
{
    g_shutdown_requested = true;
}

void StartShutdown()
{
    g_shutdown_requested = true;
}

void AbortShutdown()
{
    g_shutdown_requested = false;
}

bool ShutdownRequested()
{
    return g_shutdown_requested;
}
