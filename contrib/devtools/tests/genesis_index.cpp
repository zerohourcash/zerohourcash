#include <chain.h>
#include <chainparams.h>
#include <validation.h>
#include <util/system.h>
#include <cstdio>

int main() {
    const auto params = CreateChainParams(CBaseChainParams::REGTEST);
    const auto &genesis = params->GenesisBlock();
    uint256 hash = genesis.GetHash();
    CBlockIndex index(genesis);
    index.phashBlock = &hash;
    index.nHeight = 0;
    if (!CheckIndexProof(index, params->GetConsensus())) return 1;
    // The same insufficient-PoW header at nonzero height gets no exception.
    index.nHeight = 1;
    if (CheckIndexProof(index, params->GetConsensus())) return 2;
    // An arbitrary hash at height zero must not receive the genesis exemption.
    index.nHeight = 0;
    hash = uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    if (CheckIndexProof(index, params->GetConsensus())) return 3;
    std::puts("PASS: exact genesis accepted; invalid non-genesis and wrong-height PoW rejected");
}
