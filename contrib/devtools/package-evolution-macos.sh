#!/usr/bin/env bash
# Incremental native ARM64 Qt rebuild using an existing configured depends build.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
[[ "$(uname -s)/$(uname -m)" == Darwin/arm64 ]] || { echo 'Requires an Apple Silicon Mac.' >&2; exit 1; }
: "${ZHC_DEPENDS_PREFIX:?Set ZHC_DEPENDS_PREFIX to the staged ARM64 depends prefix used by configure}"
OUT="${ZHC_RELEASE_DIR:-$ROOT/release-macos}"
for item in include/db_cxx.h lib/libQt5Core.a plugins/platforms/libqcocoa.a; do
  [[ -f "$ZHC_DEPENDS_PREFIX/$item" ]] || { echo "Missing dependency: $item" >&2; exit 1; }
done
[[ -f "$ROOT/src/Makefile" ]] || { echo 'Configure the Qt build first.' >&2; exit 1; }
QT_LINK="-L$ZHC_DEPENDS_PREFIX/lib -L$ZHC_DEPENDS_PREFIX/plugins/platforms"
QT_LINK+=' -lqcocoa -lQt5AccessibilitySupport -lQt5ClipboardSupport -lQt5FontDatabaseSupport -lQt5GraphicsSupport -lQt5ThemeSupport -lQt5EventDispatcherSupport -lQt5PrintSupport -lQt5Network -lQt5Widgets -lQt5Gui -lQt5Core -lqtharfbuzz -lqtpcre2 -lqtlibpng -lz'
for framework in CoreFoundation Cocoa Carbon QuartzCore AppKit IOKit CoreServices CoreText ImageIO CoreGraphics Foundation Metal SystemConfiguration Security DiskArbitration; do
  QT_LINK+=" -framework $framework"
done
make -C "$ROOT/src" -j"${JOBS:-4}" -W qt/bitcoin.cpp qt/zerohour-qt \
  CXXFLAGS='-g -O2 -DQT_STATICPLUGIN -DQT_QPA_PLATFORM_COCOA' QT_LIBS="$QT_LINK"
mkdir -p "$OUT"
STAGE="$(mktemp -d "$OUT/.package.XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT
APP="$STAGE/ZHCASH Evolution.app"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
cp "$ROOT/src/qt/zerohour-qt" "$APP/Contents/MacOS/zerohour-qt"
cp "$ROOT/src/qt/res/icons/bitcoin.icns" "$APP/Contents/Resources/bitcoin.icns"
python3 - "$APP" <<'PY'
import pathlib, plistlib, re, subprocess, sys
app = pathlib.Path(sys.argv[1])
load_commands = subprocess.check_output(['otool', '-l', str(app/'Contents/MacOS/zerohour-qt')], text=True)
versions = re.findall(r'^\s*minos\s+(\d+(?:\.\d+)*)', load_commands, re.M)
if not versions:
    raise SystemExit('Cannot determine executable minimum macOS version')
minimum = max(['14.0', *versions], key=lambda v: tuple(map(int, v.split('.'))))
print(f'Executable requires macOS {minimum} or later')
info = dict(CFBundleName='ZHCASH Evolution', CFBundleDisplayName='ZHCASH Evolution',
    CFBundleIdentifier='org.zhcash.evolution', CFBundleExecutable='zerohour-qt',
    CFBundlePackageType='APPL', CFBundleShortVersionString='1.0.0', CFBundleVersion='1.0.1',
    CFBundleIconFile='bitcoin.icns', NSHighResolutionCapable=True,
    LSMinimumSystemVersion=minimum, LSArchitecturePriority=['arm64'],
    CFBundleURLTypes=[dict(CFBundleURLName='ZHCASH payment', CFBundleURLSchemes=['zerohour'])])
(app/'Contents/Info.plist').write_bytes(plistlib.dumps(info))
PY
codesign --force --sign "${ZHC_SIGN_IDENTITY:--}" "$APP"
codesign --verify --deep --strict "$APP"
"$APP/Contents/MacOS/zerohour-qt" -version
if otool -L "$APP/Contents/MacOS/zerohour-qt" | tail -n +2 | grep -E '/Users/|/opt/homebrew/'; then
  echo 'Executable still depends on local non-system libraries.' >&2
  exit 1
fi
[[ ! -e "$OUT/ZHCASH Evolution.app" ]] || { echo 'Output app already exists; select a new ZHC_RELEASE_DIR.' >&2; exit 1; }
mv "$APP" "$OUT/ZHCASH Evolution.app"
hdiutil create -volname 'ZHCASH Evolution 1.0.0' -srcfolder "$OUT/ZHCASH Evolution.app" \
  -format UDZO "$OUT/ZHCASH-Evolution-1.0.0-macos.1-arm64.dmg"
(cd "$OUT" && shasum -a 256 ZHCASH-Evolution-1.0.0-macos.1-arm64.dmg > SHA256SUMS)
git -C "$ROOT" rev-parse HEAD > "$OUT/SOURCE_COMMIT"
git -C "$ROOT" diff --stat > "$OUT/SOURCE_CHANGES"
echo "Created $OUT. Default signature is ad-hoc; notarization is a separate release step."
