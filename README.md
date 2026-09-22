# ZHCASH OnlyFans Edition — macOS

**Подарок от разработчиков ZHCASH сообществу.** Отдельная экспериментальная
версия полной Qt-ноды с графитовым интерфейсом, анимированной ASCII-заставкой
и синим неоновым значком с белой серединой.

- Ветка: `macos_onlyfans_version`.
- Публикуется только как **Pre-release**, без **Latest**; основную версию не заменяет.
- Сборка: Apple Silicon (ARM64), **macOS 26.0 и новее**.
- Отдельное имя приложения: **ZHCASH OnlyFans Edition**.
- Подпись ad-hoc; Developer ID и нотарификация Apple отсутствуют.
- Имя издания не означает связи с платформой OnlyFans.

[Описание издания, установка и сборка](doc/macos-onlyfans-edition.md).

---

What is ZHCASH?
-------------

ZHCASH is a decentralized blockchain project built on Bitcoin's UTXO model, with support for Ethereum Virtual Machine based smart contracts, and secured by a proof of stake consensus model. It achieves this through the revolutionary Account Abstraction Layer which allows the EVM to communicate with ZHCASH's Bitcoin-like UTXO blockchain. For more general information about ZHCASH as well as links to join our community, go to https://zh.cash.

What is ZHCASH Core?
------------------

ZHCASH Core is our primary mainnet wallet. It implements a full node and is capable of storing, validating, and distributing all history of the ZHCASH network. ZHCASH Core is considered the reference implementation for the ZHCASH network. 

ZHCASH Core currently implements the following:

* Sending/Receiving ZHC coins
* Sending/Receiving ZRC20 tokens on the ZHCASH network
* Staking and creating blocks for the ZHCASH network
* Creating and interacting with smart contracts
* Running a full node for distributing the blockchain to other users
* "Prune" mode, which minimizes disk usage
* Regtest mode, which enables developers to very quickly build their own private ZHCASH network for Dapp testing
* Testnet mode, using the public ZHCASH Testnet, with faucet available
* Compatibility with the Bitcoin Core set of RPC commands and APIs
* Full SegWit capability with p2sh-segwit (legacy) and bech32 (native) addresses

Implementation Status — 2026-09-17
----------------------------------

A local implementation review covered the `zerohourcash-modern-build` checkout
on branch `modern-build-with-depends-cache`, baseline commit `3f84eeb6`
(`Release Evolution 1.0.0`). The existing macOS node passed a basic isolated
regtest smoke check. **Release readiness has not been established.**

The detailed remediation and release plan, including task dependencies, test
commands, acceptance criteria, and compatibility boundaries, is available in
[the modern-build implementation plan](docs/superpowers/plans/2026-09-17-modern-build-release-readiness.md)
(in Russian).

### Architecture

| Area | Main source locations | Responsibility |
| --- | --- | --- |
| Validation and consensus | `src/validation.cpp`, `src/consensus/`, `src/chainparams.cpp` | Block/transaction validation, rewards, activation parameters |
| Peer networking | `src/net_processing.cpp`, `src/net.cpp`, `src/version.h` | P2P messages, peer lifecycle, protocol upgrade gate |
| EVM integration | `src/zerohour/`, `src/cpp-ethereum/` | UTXO/account bridge, contract execution, EVM state |
| Wallet and GUI | `src/wallet/`, `src/qt/` | Wallet persistence, keys, transactions, Qt interface |
| RPC | `src/rpc/`, `src/httprpc.cpp`, `src/httpserver.cpp` | Node/wallet API, authentication, HTTP transport |
| Build and verification | `configure.ac`, `depends/`, `src/test/`, `test/functional/` | Autotools build, pinned dependencies, unit and functional tests |

### Verified locally

* `zerohourd` and `zerohour-cli` started and reported version `1.0.0`.
* The inspected daemon and Qt executables are native macOS ARM64 binaries.
* In a temporary regtest datadir with no peer connections, RPC responded, the
  wallet created an address, two blocks were mined, and the node stopped with
  exit code `0`. The advertised protocol version was `70018`.
* All 191 Python files found under `test/functional/` passed syntax parsing;
  several invalid-escape warnings remain. This was not a functional test run.
* `contrib/devtools/build-macos-arm64.sh` passed `bash -n`.

These checks used existing binaries; they do not prove a clean rebuild or an
exact source-to-binary match. Full unit/functional suites, mainnet sync, PoS,
EVM execution, old encrypted-wallet compatibility, and GUI workflows remain
unverified by this review. Temporary regtest data was removed after shutdown.

### Findings and release work

1. **Test build blocker:** `src/Makefile.test.include` lists
   `test/evm_state_cache_tests.cpp`, but that source is absent from the checkout
   and tracked files. Restore meaningful regression coverage and verify a
   tests-enabled build. The inspected local configuration used
   `--disable-tests`, and `src/test/test_zerohour` was absent.
2. **Consensus-sensitive verification:** the subsidy schedule and peer gate
   are implemented, with boundary assertions in `src/test/main_tests.cpp` and
   `src/test/net_tests.cpp`. Their execution was not verified. Test both reward
   enforcement and actual P2P disconnection before release.
3. **Wallet compatibility guard:** the macOS helper and manual instructions
   use `--with-incompatible-bdb`, although the maintenance policy requires BDB
   4.8. Remove the default bypass and verify the selected headers and library.
   This finding does not establish corruption or incompatibility of existing
   wallets.
4. **Build isolation:** the inspected local `config.status` points at libraries
   in the sibling `zerohourcash/depends` tree. Demonstrate a clean build without
   that checkout and record dependency/toolchain provenance.
5. **macOS packaging:** the inspected `bin-new/ZHCASH-Qt.app` has an ad-hoc
   signature, no TeamIdentifier, and no sealed resources. Developer ID signing,
   notarization, package validation, and release checksums remain release gates.

The local workspace also contains a separate `zerohourcash` checkout with many
tracked modifications. Do not treat the two directories as interchangeable
build inputs or infer that their binaries represent the same source revision.
The reviewed modern-build tracked tree was clean before this documentation
update; local build outputs and logs were present as untracked files.

### Implemented subsidy schedule

For chains selecting `nSubsidyHalvingInterval == 5256000`,
`GetBlockSubsidy()` currently applies the following schedule after the PoW
phase. Heights at or below `nLastPOWBlock` retain the earlier function branch
with a subsidy of `320000 ZHC`.

| PoS height | Subsidy per block |
| --- | ---: |
| After the PoW phase through 1,699,999 | 800 ZHC |
| 1,700,000–2,499,999 | 400 ZHC |
| 2,500,000–3,499,999 | 200 ZHC |
| 3,500,000–4,499,999 | 100 ZHC |
| 4,500,000–5,499,999 | 50 ZHC |
| 5,500,000–6,499,999 | 25 ZHC |
| From 6,500,000 | 10 ZHC |

This describes the inspected implementation, not current network activation
or a newly authorized consensus change. Other halving intervals use the
separate interval-based branch in `GetBlockSubsidy()`.

Consensus Change Policy
-----------------------

Project owner rule: consensus changes are forbidden in this maintenance line
everywhere except the separately approved halving/subsidy schedule patch and
its P2P peer-protocol upgrade gate.

Do not change block validity, transaction validity, EVM execution semantics,
P2P protocol behavior, serialization, wallet database compatibility, historical
validation behavior, chain parameters, checkpoints, DGP/QIP rules, staking
rules, or state transition logic as part of build, dependency, UI, RPC,
performance, explorer, or contract usability work.

Contract-related UX improvements must be implemented by creating already-valid
transactions, for example using explicit `OP_CALL`/`sendtocontract` for a
payment to a contract `receive()` function, not by changing validator behavior
for ordinary payment outputs.

The only currently authorized consensus work is the halving/subsidy patch, and
it must remain isolated, height-gated, tested, documented, and released as a
mandatory network upgrade.

Mandatory Upgrade Peer Gate
---------------------------

Evolution 1.0.0 advertises P2P protocol version `70018`. By default, upgraded
nodes stop interacting with peers below protocol `70018` starting at block
`1,700,000`.

Operators can override the peer gate in `zerohour.conf` if needed:

```ini
forkminpeerheight=1700000
forkminpeerversion=70018
```

This gate is P2P policy, not a replacement for consensus validation. The
halving/subsidy schedule is still enforced by block validation.

Live Network Status
-------------------

Use the Zeroscan websocket API endpoint below as the primary live source for
mainnet height, supply, circulating supply, network stake weight, fee rate, and
DGP parameters:

```bash
curl https://ws.zeroscan.st/info
```

Quick Build Instructions
------------------------

The recommended build path is the bundled `depends` system. Ubuntu 24.04 is a
supported build target, including the Qt GUI, when the commands below are used.
The `depends` system builds or extracts the exact dependency set used by this
source tree, including OpenSSL 1.1.1w for wallet compatibility. If
`depends/built` is present and matches the current source tree, the dependency
step reuses the cached tarballs instead of compiling every package again.

Qt is built by `depends`, not by the system Qt packages. The Qt package is
configured in `depends/packages/qt.mk` with `-no-openssl` because Qt 5.9.7 does
not configure cleanly against the OpenSSL 1.1 API. This disables QtNetwork TLS
inside Qt only; ZHCASH Core still builds and links OpenSSL 1.1.1w for wallet
encryption and node cryptographic code.

### Ubuntu 24.04 native build with Qt

Install the host tools:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential libtool autotools-dev automake pkg-config bsdmainutils \
  git cmake python3 patch curl ca-certificates gperf bison
```

Clone this build branch and build the Linux dependency prefix. This step builds
or extracts Qt 5.9.7 with `-no-openssl` automatically:

```bash
git clone --branch modern-build-with-depends-cache --recursive https://github.com/zerohourcash/zerohourcash
cd zerohourcash

make -C depends HOST=x86_64-pc-linux-gnu -j"$(nproc)"
```

After these instructions are merged into the default branch, `--branch
modern-build-with-depends-cache` can be omitted.

Do not pass `-no-openssl` to `./configure`; it is a Qt configure option already
handled inside the `depends` Qt package. The node configure step only needs to
point at the generated dependency prefix through `CONFIG_SITE`.

Configure and build ZHCASH Core:

```bash
./autogen.sh
CONFIG_SITE="$PWD/depends/x86_64-pc-linux-gnu/share/config.site" \
  ./configure --with-gui=qt5
make -j"$(nproc)"
```

The main binaries are created under `src/`, including `zerohourd`,
`zerohour-cli`, `zerohour-tx`, `zerohour-wallet`, and the Qt GUI binary when
GUI support is enabled.

For a CLI-only build:

```bash
CONFIG_SITE="$PWD/depends/x86_64-pc-linux-gnu/share/config.site" \
  ./configure --without-gui
make -j"$(nproc)"
```

### Windows cross-build from Ubuntu 24.04

Install the Windows cross compiler:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential libtool autotools-dev automake pkg-config bsdmainutils \
  git cmake python3 patch curl ca-certificates gperf bison \
  g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64
```

Build or extract the Windows dependency prefix. This also builds/extracts Qt
with `-no-openssl` from `depends/packages/qt.mk`:

```bash
make -C depends HOST=x86_64-w64-mingw32 -j"$(nproc)"
```

Configure and build the Windows CLI and Qt binaries:

```bash
./autogen.sh
CONFIG_SITE="$PWD/depends/x86_64-w64-mingw32/share/config.site" \
  ./configure --host=x86_64-w64-mingw32 --with-gui=qt5
make -j"$(nproc)"
```

The Windows `.exe` binaries are created under `src/`.

### macOS Apple Silicon

For native Apple Silicon macOS builds, see `doc/build-osx-arm64.md`.

For Linux-to-macOS cross-build preparation, the target triplet is:

```bash
make -C depends HOST=aarch64-apple-darwin -j"$(nproc)"
```

This requires a local Apple `MacOSX*.sdk` under `depends/SDKs/`. The SDK is not
included in this repository because it is distributed under Apple's license.
The canonical target triplet is `aarch64-apple-darwin`; `arm64-apple-darwin` is
not accepted by the bundled `config.sub`.
    

### Evolution visual refresh (macOS)

The refreshed desktop build preserves the complete existing Qt wallet and adds
a graphite palette and the ZHC Wallet Desktop incoming-transfer MP3.
The overview shows balances and transactions directly, without a decorative banner. See [the implementation, build and verification notes](doc/evolution-visual-refresh.md).
The macOS release retains the original complete Qt wallet.
