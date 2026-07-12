# macOS arm64 Native Build Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a safe, documented path for native Apple Silicon macOS builds without regressing existing Linux and Windows builds.

**Architecture:** Keep Linux and Windows build paths unchanged. Add macOS arm64 as an additional Darwin host target using the GNU canonical triplet `aarch64-apple-darwin`, with docs that explain native Mac builds and Linux cross-build prerequisites. Start with CLI/headless verification, then Qt packaging.

**Tech Stack:** Autotools, depends, Darwin clang, Qt 5, Berkeley DB 4.8, OpenSSL from depends, macOS SDK.

## Global Constraints

- Do not remove or rename existing Linux and Windows targets.
- Use `aarch64-apple-darwin` as the canonical Apple Silicon target because current `config.sub` rejects `arm64-apple-darwin`.
- Keep wallet compatibility assumptions intact: BDB 4.8 remains required for wallet builds.
- Use OpenSSL 1.1.1w for depends; do not migrate this work to OpenSSL 3.
- Prefer CLI build verification before Qt packaging.

---

### Task 1: Add macOS arm64 target metadata

**Files:**
- Modify: `depends/README.md`
- Modify: `depends/Makefile`
- Modify: `depends/hosts/darwin.mk`

**Interfaces:**
- Consumes: existing depends variables `host_arch`, `host_os`, `OSX_MIN_VERSION`, `OSX_SDK_VERSION`.
- Produces: documented `aarch64-apple-darwin` target and `download-osx-arm64` helper.

- [ ] **Step 1: Document canonical host triplet**

Add `aarch64-apple-darwin` to the common host triplets in `depends/README.md`.

- [ ] **Step 2: Add a download helper**

Add `download-osx-arm64` to `depends/Makefile`:

```make
download-osx-arm64:
	@$(MAKE) -s HOST=aarch64-apple-darwin download-one
```

Keep `download-osx` pointing at `x86_64-apple-darwin14` for legacy compatibility.

- [ ] **Step 3: Set Darwin arm64 minimum macOS version**

In `depends/hosts/darwin.mk`, keep x86_64 defaults unchanged and set Apple Silicon minimum to macOS 11:

```make
ifeq ($(host_arch),aarch64)
OSX_MIN_VERSION=11.0
endif
```

- [ ] **Step 4: Verify target parsing**

Run:

```bash
./depends/config.sub aarch64-apple-darwin
```

Expected output:

```text
aarch64-apple-darwin
```

### Task 2: Add macOS arm64 build documentation

**Files:**
- Create: `doc/build-osx-arm64.md`
- Modify: `doc/README.md`
- Modify: `README.md`

**Interfaces:**
- Consumes: target from Task 1.
- Produces: operator-facing native Apple Silicon build instructions.

- [ ] **Step 1: Write the build doc**

Create `doc/build-osx-arm64.md` with commands for:

```bash
./autogen.sh
./configure --without-gui --disable-bip70
make -j"$(sysctl -n hw.ncpu)"
make check
```

Then Qt:

```bash
./configure --with-gui=qt5 --disable-bip70
make -j"$(sysctl -n hw.ncpu)"
make deploy
```

- [ ] **Step 2: Document cross-build limitation**

State that Linux cross-build requires an Apple SDK and should use:

```bash
cd depends
make HOST=aarch64-apple-darwin SDK_PATH=/path/to/SDKs -j4
```

Also state that depends OpenSSL is pinned to 1.1.1w and that native macOS may
use Homebrew `openssl@1.1` while the SDK/toolchain path is being verified.

- [ ] **Step 3: Link the doc**

Add `doc/build-osx-arm64.md` to `doc/README.md`. Add a short note to `README.md` that Apple Silicon uses `aarch64-apple-darwin`.

### Task 3: Validate no Linux/Windows regressions

**Files:**
- Test only.

**Interfaces:**
- Consumes: existing Linux and Windows depends targets.
- Produces: verification logs.

- [ ] **Step 1: Check configure triplets**

Run:

```bash
./depends/config.sub x86_64-pc-linux-gnu
./depends/config.sub x86_64-w64-mingw32
./depends/config.sub aarch64-apple-darwin
```

Expected: each command prints a canonical triplet and exits `0`.

- [ ] **Step 2: Check helper syntax**

Run:

```bash
bash -n contrib/devtools/build-macos-arm64.sh
```

Expected: exit code `0`.

- [ ] **Step 3: Run available local build smoke check**

Run the current host build smoke command that is already known to work in this workspace. If the tree is configured for another target, reconfigure before compiling.
