#!/usr/bin/env bash
# Independent opt-in macOS gift edition. Never publish as GitHub Latest.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
export ZHC_APP_NAME='ZHCASH OnlyFans Edition'
export ZHC_BUNDLE_ID='org.zhcash.onlyfans'
export ZHC_BUNDLE_VERSION='1.0.2'
export ZHC_DMG_NAME='ZHCASH-OnlyFans-Edition-1.0.0-macos-arm64.dmg'
export ZHC_RELEASE_DIR="${ZHC_RELEASE_DIR:-$ROOT/release-onlyfans-macos}"
exec bash "$ROOT/contrib/devtools/package-evolution-macos.sh"
