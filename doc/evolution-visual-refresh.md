# Evolution: visual refresh of the complete Qt wallet

The delivery uses the existing Qt5 application and wallet models. The full wallet remains the delivered application. Existing actions,
forms, RPC console, wallet encryption, backups, address book, coin control,
staking, contracts and ZRC token operations remain in the original implementation.

## Appearance

The palette follows the supplied macOS desktop reference: neutral `#181818`
canvas, `#292c29` sidebar/panels, `#353535` inputs, `#3b3e3b` selection
and soft `#e3e3e3` text. Legacy light menus, tables and tooltip surfaces also
use the dark palette; transaction status/error colors retain their meaning. The overview shows balances and
transactions directly; the planet banner and its Evolution heading have been
removed. Available-balance figures are enlarged by 50%; the total uses 1.9×
the base font size, with unchanged caption sizing.

Overview balances, tokens and recent transactions share the dark canvas without
contrasting card backgrounds or outlines.

The navigation dock fills the available window height with a flat dark surface.
Buttons use quiet flat hover/selection states, without glass highlights or gradients.
Overview sections are separated by thin #2c2c2c horizontal rules with inset edges.
UI icons, including status indicators and QSS arrows, use light glyphs; disabled
controls remain visible in gray and transaction statuses retain light semantic colors.

The Cocoa layer-backed renderer is selected before QApplication initialization to avoid
partial-repaint artifacts observed on macOS 26 with this legacy Qt build.
A deferred full-window update after resizing refreshes the title/navigation docks.

## Incoming transfer sound (macOS)

The bundled `src/qt/res/sounds/cash-register.mp3` is the identical asset from
`ZHC-Wallet/public/sounds/cash-register.mp3`, played at volume 0.72.

- Enabled by default; toggle **Настройки → Звук пополнения**.
- New positive ZHC receive/contract-receive records and incoming token records
  trigger asynchronous playback through the system `/usr/bin/afplay`.
- Initial synchronization and queued historical notifications are suppressed.
- Outgoing/self transfers and mined rewards do not trigger the ZHC sound.
- A bounded transaction-key cache prevents duplicates during the session.
- Simultaneous playback is suppressed; playback failures cannot block the node.
- The resource is extracted into a temporary MP3 owned by the playback process
  and removed when that process is destroyed.
- No environment variables, external player installation or QtMultimedia package
  are required. The current sound implementation targets macOS.

## Build and package

Use a configured native ARM64 depends build, then:

```sh
ZHC_DEPENDS_PREFIX=/absolute/path/to/depends/aarch64-apple-darwin \
ZHC_RELEASE_DIR="$PWD/release-evolution-1.0.0-macos-visual" \
bash contrib/devtools/package-evolution-macos.sh
```

Choose a fresh output directory. The package script creates an application and
DMG, checks the signature and rejects non-system runtime library dependencies.
It reads the binary's minimum macOS version rather than advertising a lower
unsupported version. The current local build requires **macOS 26+ on Apple
Silicon**. Compatibility with macOS 14–15 is not validated and requires a rebuilt
dependency toolchain. Default signing is ad-hoc, without Apple notarization.

## Verification and limits

The isolated regtest integration check creates two temporary wallets and 501
blocks, sends a real test transaction, observes one MP3 playback process, checks
that confirmation/self-send/mining do not replay it, and exercises address
generation, message signing/verification and wallet backup:

```sh
python3 contrib/devtools/tests/qt_incoming_sound_smoke.py \
  '/absolute/path/to/ZHCASH Evolution.app/Contents/MacOS/zerohour-qt'
python3 contrib/devtools/tests/run_genesis_index.py
```

The sound test needs a graphical macOS session and enabled incoming sound; it
plays the MP3 audibly. `--keep-open` retains the isolated GUI for manual inspection.
Temporary test data is retained for diagnosis. The actual macOS audio device and
user volume determine audible output. Manual checks also cover the mute menu and
navigation through overview, send, receive, history, contracts and tokens.
This is not an exhaustive end-to-end validation of every staking/contract/token
scenario or production-wallet migration.

## Separate startup correction

During testing, reopening an existing regtest database rejected the configured
genesis because its hash does not satisfy its encoded PoW target. Initial startup
already trusts that exact configured genesis. `CheckIndexProof` now applies the
same exception only at height zero and only for the exact configured genesis
hash. Other height-zero hashes and invalid PoW at nonzero height are still
rejected. The focused linked-core regression checks those cases. No network
parameters or wallet storage formats are changed.

## Application icon

Finder and the Qt runtime use the same current red ZHC Wallet brand asset,
`red-app-dark-v1-1024.png` as the reference. The icon is tightly framed around the
red circle with transparent outer corners. The macOS bundle includes normal and Retina sizes.
On macOS the app icon is not hue-shifted for test networks; the window title
continues to identify testnet/regtest.

## Minimal theme verification (2026-09-22)

Inspected overview, send, receive, history, all three contract pages, token pages,
all four settings tabs and diagnostic tabs in an isolated offline regtest wallet.
Diagnostic resize now schedules a full background/child repaint on macOS, where
old tab positions could remain in the backing store. Peer headings receive widths
based on translated text; overflowing columns remain horizontally scrollable.
Empty peer details hide their scroll container. Table scrollbars use the same
compact dark treatment as the rest of the application. No live transactions were
performed during these appearance checks.

Typography uses the native macOS general UI font at 13 pt, including console,
addresses and signatures. Buttons retain normal capitalization; large balances
use semibold weight. No monospaced override is applied.
