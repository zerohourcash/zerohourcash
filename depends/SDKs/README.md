macOS SDKs
==========

Put legally obtained Apple macOS SDK bundles here for Linux-to-macOS
cross-compilation.

Expected layout:

```text
depends/SDKs/MacOSX14.5.sdk
depends/SDKs/MacOSX15.0.sdk
```

Do not commit Apple SDK contents to this repository. Apple SDKs are obtained
through Xcode or Xcode Command Line Tools and are subject to Apple's license.

Recommended workflow:

1. On an Apple Silicon Mac with Xcode or Command Line Tools installed:

   ```bash
   contrib/devtools/export-macos-sdk.sh
   ```

2. Copy the generated `MacOSX*.sdk.tar.gz` archive to this machine.
3. From the repository root on this machine:

   ```bash
   contrib/devtools/import-macos-sdk.sh /path/to/MacOSX*.sdk.tar.gz
   ```

4. Build depends with:

   ```bash
   make -C depends HOST=aarch64-apple-darwin SDK_PATH="$PWD/depends/SDKs" -j4
   ```
