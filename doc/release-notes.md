ZHCASH Core Evolution 1.0.0 Release Notes
=========================================

ZHCASH Core Evolution 1.0.0 is a consensus and maintenance release for the
ZHCASH network.

Upgrade Policy
==============

All backbone, staking, validator, explorer, exchange, and service nodes should
upgrade to Evolution 1.0.0 before the first PoS reward reduction at block
1,700,000.

Evolution 1.0.0 advertises P2P protocol version `70018`. By default, upgraded
nodes continue to accept older compatible peers before block `1,700,000`, then
disconnect peers below protocol `70018` from block `1,700,000`.

Operators can override the peer gate in `zerohour.conf`:

```
forkminpeerheight=1700000
forkminpeerversion=70018
```

This is P2P policy. The mandatory consensus change is still the reward schedule
enforced by block validation.

Important: old staking nodes can create blocks with the previous 800 ZHC reward
after the new schedule activates. Evolution 1.0.0 nodes reject those blocks as
overpaid. Users who stake must upgrade before staking across the halving height.

PoS Reward Schedule
===================

Evolution 1.0.0 activates the updated PoS reward schedule:

- before block 1,700,000: 800 ZHC
- from block 1,700,000: 400 ZHC
- from block 2,500,000: 200 ZHC
- from block 3,500,000: 100 ZHC
- from block 4,500,000: 50 ZHC
- from block 5,500,000: 25 ZHC
- from block 6,500,000: 10 ZHC

The 10 ZHC reward remains as the long-term fallback reward.

Snapshot Policy
===============

Automatic snapshot generation is not part of the node. Snapshots should be
created and distributed by a separate full-node snapshot tool/service.

Verification
============

The `getsubsidy` RPC can now calculate a fast dry-run subsidy summary for a
height range without mining or requiring those blocks to exist locally:

```
zerohour-cli getsubsidy 1 10000000
```

The single-height form is unchanged:

```
zerohour-cli getsubsidy 1700000
```
