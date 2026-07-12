#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "This helper must be run on macOS." >&2
  exit 1
fi

if [[ "$(uname -m)" != "arm64" ]]; then
  echo "This helper is for native Apple Silicon arm64 builds." >&2
  echo "Current architecture: $(uname -m)" >&2
  exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required. Install it from https://brew.sh and rerun." >&2
  exit 1
fi

GUI="${GUI:-0}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu)}"

OPENSSL_PREFIX="$(brew --prefix openssl@1.1)"
if BDB_PREFIX_DETECTED="$(brew --prefix berkeley-db@4 2>/dev/null)"; then
  BDB_PREFIX="${BDB_PREFIX:-$BDB_PREFIX_DETECTED}"
else
  BDB_PREFIX="${BDB_PREFIX:-$PWD/db4}"
fi

if [[ ! -d "$BDB_PREFIX/include" || ! -d "$BDB_PREFIX/lib" ]]; then
  echo "Berkeley DB 4 headers/libs not found at: $BDB_PREFIX" >&2
  echo "Install berkeley-db@4 or run: ./contrib/install_db4.sh \"$PWD\"" >&2
  exit 1
fi

CONFIGURE_FLAGS=(
  --disable-bip70
  --with-incompatible-bdb
)

if [[ "$GUI" == "1" ]]; then
  QT_PREFIX="$(brew --prefix qt@5)"
  export PATH="${QT_PREFIX}/bin:${PATH}"
  export PKG_CONFIG_PATH="${QT_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
  CONFIGURE_FLAGS+=(--with-gui=qt5)
  EXTRA_CPPFLAGS="-I${QT_PREFIX}/include"
  EXTRA_LDFLAGS="-L${QT_PREFIX}/lib"
else
  CONFIGURE_FLAGS+=(--without-gui)
  EXTRA_CPPFLAGS=""
  EXTRA_LDFLAGS=""
fi

./autogen.sh

./configure \
  "${CONFIGURE_FLAGS[@]}" \
  CPPFLAGS="-I${OPENSSL_PREFIX}/include -I${BDB_PREFIX}/include ${EXTRA_CPPFLAGS}" \
  LDFLAGS="-L${OPENSSL_PREFIX}/lib -L${BDB_PREFIX}/lib ${EXTRA_LDFLAGS}"

make -j"${JOBS}"
make check

file src/zerohourd src/zerohour-cli src/zerohour-wallet src/zerohour-tx

if [[ "$GUI" == "1" && -f src/qt/zerohour-qt ]]; then
  file src/qt/zerohour-qt
fi
