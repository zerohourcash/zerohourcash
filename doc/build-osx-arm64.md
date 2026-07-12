macOS Apple Silicon arm64 Build Instructions
============================================

This document describes the native Apple Silicon build path for ZHCASH Core.
Use this for Macs with M1, M2, M3, M4, or newer Apple Silicon CPUs.

The canonical GNU host triplet is:

```bash
aarch64-apple-darwin
```

Do not use `arm64-apple-darwin` with the bundled autotools config scripts:
the current `config.sub` rejects that alias.

Scope
-----

The recommended order is:

1. Build and test the CLI/node binaries first.
2. Build the Qt GUI after the CLI build is healthy.
3. Package, sign, and notarize the application as a separate release step.

This order keeps the node build separate from Qt deployment issues.

Important Dependency Notes
--------------------------

Wallet builds require Berkeley DB 4.8 to preserve `wallet.dat` compatibility.

Use OpenSSL 1.1.1 for this maintenance line. Do not migrate this build to
OpenSSL 3 without a separate wallet compatibility test plan.

The depends OpenSSL package is pinned to OpenSSL 1.1.1w for this maintenance
line. A native arm64 macOS build can use either Homebrew `openssl@1.1` or the
depends package once the local macOS SDK/toolchain is available.

Install Native Build Tools
--------------------------

Install Xcode command line tools:

```bash
xcode-select --install
```

Install Homebrew from https://brew.sh if it is not already installed.

Install dependencies:

```bash
brew install autoconf automake libtool pkg-config cmake boost libevent gmp miniupnpc qrencode zeromq openssl@1.1 berkeley-db@4 qt@5
```

If `berkeley-db@4` is not available in the active Homebrew tap, build BDB 4.8
from the repository helper instead:

```bash
./contrib/install_db4.sh "$PWD"
```

Then pass the generated BDB path to `configure` with `BDB_PREFIX`.

Build CLI / Headless Node
-------------------------

From the repository root:

```bash
contrib/devtools/build-macos-arm64.sh
```

The helper builds CLI/headless binaries and runs `make check`.

To build the GUI through the helper:

```bash
GUI=1 contrib/devtools/build-macos-arm64.sh
```

Manual commands are listed below for troubleshooting.

From the repository root:

```bash
./autogen.sh
```

If using Homebrew BDB and OpenSSL:

```bash
export OPENSSL_PREFIX="$(brew --prefix openssl@1.1)"
export BDB_PREFIX="$(brew --prefix berkeley-db@4 2>/dev/null || true)"
```

If BDB was built through `contrib/install_db4.sh`, use the generated path:

```bash
export BDB_PREFIX="$PWD/db4"
```

Configure CLI/headless first:

```bash
./configure \
  --without-gui \
  --disable-bip70 \
  --with-incompatible-bdb \
  CPPFLAGS="-I${OPENSSL_PREFIX}/include -I${BDB_PREFIX}/include" \
  LDFLAGS="-L${OPENSSL_PREFIX}/lib -L${BDB_PREFIX}/lib"
```

Build and test:

```bash
make -j"$(sysctl -n hw.ncpu)"
make check
```

Expected binaries:

```text
src/zerohourd
src/zerohour-cli
src/zerohour-wallet
src/zerohour-tx
```

Verify that the binaries are native arm64:

```bash
file src/zerohourd src/zerohour-cli src/zerohour-wallet src/zerohour-tx
```

Expected output includes:

```text
Mach-O 64-bit executable arm64
```

Build Qt GUI
------------

After the CLI build passes, configure with Qt:

```bash
export QT_PREFIX="$(brew --prefix qt@5)"
export PATH="${QT_PREFIX}/bin:${PATH}"
export PKG_CONFIG_PATH="${QT_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"

./configure \
  --with-gui=qt5 \
  --disable-bip70 \
  --with-incompatible-bdb \
  CPPFLAGS="-I${OPENSSL_PREFIX}/include -I${BDB_PREFIX}/include -I${QT_PREFIX}/include" \
  LDFLAGS="-L${OPENSSL_PREFIX}/lib -L${BDB_PREFIX}/lib -L${QT_PREFIX}/lib"
```

Build:

```bash
make -j"$(sysctl -n hw.ncpu)"
```

Optional app bundle / DMG step:

```bash
make deploy
```

Verify the GUI binary:

```bash
file src/qt/zerohour-qt
```

Expected output includes:

```text
Mach-O 64-bit executable arm64
```

Linux Cross-Compilation Notes
-----------------------------

Linux cross-compilation to macOS arm64 requires an Apple macOS SDK. The SDK is
not redistributable and must be supplied locally.

Apple's supported source for the SDK is Xcode or Xcode Command Line Tools.
Do not use random SDK mirrors for release builds.

To export the SDK from a Mac that already has Xcode or Command Line Tools
installed:

```bash
contrib/devtools/export-macos-sdk.sh /tmp
```

Copy the generated `MacOSX*.sdk.tar.gz` to this Linux build machine, then
import it into the repository:

```bash
contrib/devtools/import-macos-sdk.sh /path/to/MacOSX*.sdk.tar.gz
```

The intended depends target is:

```bash
cd depends
make HOST=aarch64-apple-darwin SDK_PATH=/path/to/SDKs -j4
```

Cross-compilation should not be treated as release-ready until the macOS SDK,
OpenSSL 1.1.1w depends package, Qt, and final app packaging are tested together
without regressing Linux and Windows builds.

Release Notes
-------------

For public macOS releases:

1. Build native arm64 binaries.
2. Run `make check`.
3. Start `zerohourd` with a temporary datadir.
4. Verify `getnetworkinfo`, `getblockchaininfo`, and peer connections.
5. Verify old encrypted `wallet.dat` unlock/decrypt behavior on a copy.
6. Sign and notarize the app bundle.
7. Create and verify checksums for the release archive or DMG.
