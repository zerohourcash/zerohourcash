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

This is a quick start script for compiling ZHCASH on Ubuntu 20.04. On UBUNTU newer than 20.04, compilation is only possible without qt

    sudo apt-get install build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils git cmake libboost-all-dev libgmp3-dev
    sudo apt-get update

    # If you want to build the Qt GUI:
    sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler qrencode

    git clone https://github.com/zerohourcash/zerohourcash --recursive
    cd zerohourcash

    # Note autogen will prompt to install some more dependencies if needed
    1. Go to the "depends" folder, run: 
        make -j$(nproc)                              // For linux. Wait until it finishes (about 10–15 minutes). The last line will show a path — copy it to the clipboard.
        make HOST=x86_64-w64-mingw32 -j$(nproc)      // to compile for windows
    2. Run:
        autogen.sh.
    3. Run:
        ./configure --prefix=/root/zerohourcash/depends/x86_64-pc-linux-gnu     // paste the path from step 1 right after the equals sign in prefix
        ./configure --prefix=/root/zerohourcash/depends/x86_64-w64-mingw32 --with-gui=qt5 --host=x86_64-w64-mingw32 --enable-static --disable-shared  CXX=x86_64-w64-mingw32-g++  CXXFLAGS="-static-libgcc -static-libstdc++"    // to configure for windows
    4. Go to "zerohourcash" folder. Run: 
        make -j$(nproc)                              // and wait (about 20–25 minutes).
    
