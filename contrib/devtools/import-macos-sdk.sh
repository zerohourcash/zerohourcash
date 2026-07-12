#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "usage: $0 /path/to/MacOSX*.sdk.tar.gz|/path/to/MacOSX*.sdk" >&2
  exit 2
fi

src="$1"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
sdk_dir="$repo_root/depends/SDKs"
mkdir -p "$sdk_dir"

case "$src" in
  *.sdk)
    if [[ ! -d "$src" ]]; then
      echo "error: SDK bundle does not exist: $src" >&2
      exit 1
    fi
    sdk_name="$(basename "$src")"
    rm -rf "$sdk_dir/$sdk_name"
    cp -a "$src" "$sdk_dir/$sdk_name"
    ;;
  *.sdk.tar.gz|*.sdk.tgz|*.tar.gz|*.tgz)
    if [[ ! -f "$src" ]]; then
      echo "error: SDK archive does not exist: $src" >&2
      exit 1
    fi
    tmp_dir="$(mktemp -d)"
    trap 'rm -rf "$tmp_dir"' EXIT
    tar -xzf "$src" -C "$tmp_dir"
    found_sdk="$(find "$tmp_dir" -maxdepth 2 -type d -name 'MacOSX*.sdk' | head -n 1)"
    if [[ -z "$found_sdk" ]]; then
      echo "error: archive does not contain a MacOSX*.sdk directory" >&2
      exit 1
    fi
    sdk_name="$(basename "$found_sdk")"
    rm -rf "$sdk_dir/$sdk_name"
    cp -a "$found_sdk" "$sdk_dir/$sdk_name"
    ;;
  *)
    echo "error: unsupported input. Expected MacOSX*.sdk or MacOSX*.sdk.tar.gz" >&2
    exit 2
    ;;
esac

if [[ ! -d "$sdk_dir/$sdk_name/usr/include" && ! -d "$sdk_dir/$sdk_name/System/Library/Frameworks" ]]; then
  echo "error: imported SDK does not look valid: $sdk_dir/$sdk_name" >&2
  exit 1
fi

echo "Imported $sdk_dir/$sdk_name"
echo "Use: make -C depends HOST=aarch64-apple-darwin SDK_PATH=\"$sdk_dir\" -j4"
