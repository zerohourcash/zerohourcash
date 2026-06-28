# ZHCash Compatible Modernization Release Design

Date: 2026-06-28

## Goal

Build the next maintenance release of the ZHCash node on modern toolchains and libraries while preserving compatibility with existing nodes, wallets, blocks, chainstate, and historical consensus behavior.

This release is not a soft fork. It must modernize build reliability, platform support, dependency versions, warning cleanliness, and Qt usability without changing consensus rules or network semantics.

## Release Scope

The release targets all primary user-facing platforms:

- Linux daemon, CLI, wallet tool, and Qt GUI, starting with Ubuntu 24.04 and keeping the build ready for newer Ubuntu releases.
- Windows daemon, CLI, wallet tool, and Qt GUI through the project's reproducible or cross-build flow where available.
- macOS daemon, CLI, wallet tool, and Qt GUI through the project's reproducible or native build flow where available.
- Headless server builds remain a first-class target.

The release may update the displayed software version and user agent subversion. It must not require peer nodes to upgrade.

## Compatibility Boundaries

The following behavior must remain unchanged unless a later, separately approved soft-fork spec changes it:

- Block and transaction validity rules.
- Proof-of-work, proof-of-stake, subsidy, maturity, DGP, QIP, EVM, and fork activation logic.
- Genesis block, chain parameters, checkpoints, network magic, address prefixes, Bech32 HRPs, and historical activation heights.
- P2P message serialization, disk block format, undo data format, and chainstate interpretation.
- Wallet database compatibility for existing Berkeley DB wallets.
- RPC behavior that existing operational scripts reasonably depend on.

Berkeley DB 4.8 remains the legacy wallet compatibility anchor for this release. Any later migration away from BDB 4.8 must be an optional migration project with its own design, backups, downgrade behavior, and recovery tests.

## Modernization Strategy

Use a compatibility-first staged modernization branch. Dependencies are updated in risk groups, with verification after each group instead of one large dependency jump.

Recommended order:

1. Stabilize the existing Ubuntu 24 build with and without Qt.
2. Update lower-risk infrastructure dependencies such as zlib, libevent, ZeroMQ, miniupnpc, and GMP.
3. Update build and packaging support for Linux, Windows, and macOS.
4. Update Boost, Crypto++, protobuf, and Qt-related integration code.
5. Treat OpenSSL as a separate migration because OpenSSL 3 removes or deprecates APIs used by older Bitcoin-derived code.
6. Keep BDB 4.8 unchanged until all other modernization work is stable.

Each dependency update must identify whether the dependency is consensus-sensitive, wallet-sensitive, P2P-sensitive, GUI-only, or build-only.

## OpenSSL Position

The target is eventual compatibility with modern OpenSSL, including OpenSSL 3 where practical. The migration must be isolated behind compatibility wrappers or localized code paths before switching the dependency version.

The release must not accept a crypto-library update solely because it compiles. It must verify signature encoding, key handling, wallet encryption, certificate/payment-request code used by Qt, and any low-level ECDSA behavior that can affect transaction validity or wallet interoperability.

If OpenSSL 3 cannot be completed safely in the first maintenance release, the fallback is to modernize the rest of the dependency stack and keep a documented OpenSSL compatibility step for the next maintenance milestone. That fallback requires explicit review before release.

## Qt Design

Qt is in scope for this release. GUI modernization should stay in the Qt layer and must not change consensus, wallet database semantics, or node validation behavior.

Expected Qt work includes:

- Build fixes for the selected modern Qt version.
- Replacement of removed or deprecated Qt APIs.
- macOS and Windows packaging fixes when required by the Qt version.
- Startup smoke tests for the GUI against an isolated datadir.
- Wallet open/create smoke tests where GUI wallet support is enabled.

The GUI remains a consumer of node and wallet interfaces. Any deeper wallet or node interface redesign belongs in a separate project.

## Warning Policy

The project should build cleanly under the supported toolchains. The target is warning-free project code and a reliable `--enable-werror` path.

Third-party imported code and generated files may need scoped warning suppressions if upstream warnings cannot be fixed locally without forking large libraries. Suppressions must be narrow and documented in the build files.

No warning may be hidden if it indicates possible consensus divergence, undefined behavior, data truncation, lifetime bugs, serialization changes, cryptographic misuse, or wallet corruption.

## Verification Requirements

Every dependency stage must run the relevant subset of:

- Configure and build for headless Linux.
- Configure and build for Linux with Qt.
- Unit tests including existing blockchain, wallet, miner, script, validation, key, and ZHCash-specific tests.
- Smoke start of `zerohourd` on a temporary datadir.
- Smoke use of `zerohour-cli`.
- Qt startup smoke test on a temporary datadir where display infrastructure is available.
- Wallet create, open, encrypt where supported by existing tests or smoke harnesses.
- Historical block and chainstate compatibility checks using preserved test fixtures or a known datadir snapshot.
- P2P handshake compatibility with the previous released node.

Before release, the full verification matrix must cover Linux, Windows, and macOS builds. If a platform cannot run the complete test suite locally, the gap must be documented and covered by CI, reproducible builders, or manual release checks.

## Out Of Scope

This release does not include:

- New consensus rules.
- New soft-fork deployments.
- Mandatory hard-fork behavior.
- P2P protocol redesign.
- Wallet database replacement.
- Major node architecture rewrite.
- Replacing the build system with a new system such as CMake.

Those items may be designed later after this compatibility release is stable.

## Success Criteria

The release is successful when:

- Supported platforms build from clean checkout instructions.
- Headless and Qt builds work.
- Project code is warning-clean or has documented, narrow third-party suppressions.
- Existing unit tests pass.
- Existing wallets remain readable.
- Existing block data and chainstate remain usable.
- The node can handshake and operate with previous compatible ZHCash nodes.
- No consensus, serialization, chain parameter, or historical validation behavior changes are introduced.

