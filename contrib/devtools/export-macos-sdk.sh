#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "error: this script must run on macOS with Xcode or Command Line Tools installed" >&2
  exit 1
fi

sdk_path="$(xcrun --sdk macosx --show-sdk-path)"
if [[ ! -d "$sdk_path" ]]; then
  echo "error: xcrun did not return a valid macOS SDK path" >&2
  exit 1
fi

# xcrun commonly returns .../SDKs/MacOSX.sdk, which is a symlink. Archiving the
# symlink creates a tiny unusable tarball, so resolve it to the physical SDK dir.
sdk_path="$(cd "$sdk_path" && pwd -P)"
sdk_name="$(basename "$sdk_path")"
if [[ "$sdk_name" != MacOSX*.sdk ]]; then
  echo "error: unexpected SDK bundle name: $sdk_name" >&2
  exit 1
fi

out_dir="${1:-$PWD}"
mkdir -p "$out_dir"
out_file="$out_dir/$sdk_name.tar.gz"

echo "Exporting $sdk_path"
tar -C "$(dirname "$sdk_path")" -czhf "$out_file" "$sdk_name"
echo "$out_file"
