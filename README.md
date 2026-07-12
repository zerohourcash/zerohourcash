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

Quick Build Instructions
------------------------

The recommended build path is the bundled `depends` system. Ubuntu 24.04 is a
supported build target, including the Qt GUI, when the commands below are used.
The `depends` system builds or extracts the exact dependency set used by this
source tree, including OpenSSL 1.1.1w for wallet compatibility. If
`depends/built` is present and matches the current source tree, the dependency
step reuses the cached tarballs instead of compiling every package again.

### Ubuntu 24.04 native build with Qt

Install the host tools:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential libtool autotools-dev automake pkg-config bsdmainutils \
  git cmake python3 patch curl ca-certificates gperf bison
```

Clone this build branch and build the Linux dependency prefix:

```bash
git clone --branch modern-build-with-depends-cache --recursive https://github.com/zerohourcash/zerohourcash
cd zerohourcash

make -C depends HOST=x86_64-pc-linux-gnu -j"$(nproc)"
```

After these instructions are merged into the default branch, `--branch
modern-build-with-depends-cache` can be omitted.

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

Build or extract the Windows dependency prefix:

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
    
