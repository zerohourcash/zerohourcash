# Modern-build Release Readiness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Получить воспроизводимую, проверенную сборку Evolution 1.0.0 для Linux, Windows и macOS, закрыв найденные дефекты тестов, проверки совместимости кошельков и подготовки релиза.

**Architecture:** Сохраняем существующие C++-компоненты Bitcoin/Qtum, UTXO–EVM, PoS, Qt и Autotools. Работы выполняются небольшими независимыми изменениями тестов, сборочных скриптов и документации; существующие правила консенсуса фиксируются регрессионными проверками.

**Tech Stack:** C++, Boost.Test, Python functional framework, Autotools, depends, Berkeley DB 4.8, OpenSSL 1.1.1w, Qt 5, macOS codesign/notarytool.

## Global Constraints

- Рабочий репозиторий: `zerohourcash-modern-build`; текущая ветка: `modern-build-with-depends-cache`; исходный снимок аудита: `3f84eeb6`.
- План создан 2026-09-17. Пункты ниже являются будущей работой, а не выполненными исправлениями.
- Запрет изменения консенсуса из README остаётся обязательным. Исключение — уже отдельно разрешённая схема subsidy/halving и её P2P gate; этот план не расширяет разрешение.
- Не менять сериализацию, историческую валидацию, EVM-семантику, genesis, checkpoints, DGP/QIP, staking или формат `wallet.dat` ради прохождения тестов.
- Сохранить `PROTOCOL_VERSION=70018`, default peer gate height `1700000`, minimum peer version `70018` и существующие настройки оператора.
- BDB 4.8 и OpenSSL 1.1.1w сохраняются в этой линии. Переход на другую BDB или OpenSSL 3 требует отдельного проекта совместимости.
- Не переносить содержимое и не исправлять соседний `zerohourcash` в рамках этого плана; не использовать его `depends` как скрытую зависимость.
- Все runtime-тесты используют временные datadir и копии fixtures. Реальные кошельки не открывать на запись; пароли, ключи и содержимое кошельков не помещать в отчёты.
- Не считать успешную компиляцию проверкой консенсуса, smoke — полным functional suite, cross-build — проверкой на целевой ОС.
- Исходный дизайн `2026-06-28-compatible-modernization-design.md` исторически запрещал mandatory upgrade. Для уже принятого subsidy/peer gate приоритет имеет текущая Consensus Change Policy в README; остальные границы совместимости сохраняются.
- Публикация пакетов и активация обновления сети не входят в разрешение на создание этого плана.

## 1. Исходное состояние и доказательства

| Проверка на 2026-09-17 | Результат | Граница вывода |
| --- | --- | --- |
| Запуск существующих daemon/CLI | Версия 1.0.0 | Не доказывает соответствие текущим исходникам |
| Архитектура daemon/Qt | Mach-O arm64 | Не проверяет GUI-сценарии |
| Изолированный regtest | RPC, новый адрес, два блока, protocol 70018, ноль peers, exit 0 | Не проверяет PoS, EVM или mainnet |
| Python syntax parse | 191 файл без SyntaxError, есть escape warnings | Не выполнение functional tests |
| `bash -n` macOS helper | Успех | Не выполнение сборки |
| Unit tests | Не запускались: бинарник отсутствует, конфигурация содержит `--disable-tests` | Прохождение не подтверждено |
| Test source inventory | Отсутствует `src/test/evm_state_cache_tests.cpp`, указанный в Makefile | Конкретный дефект тестовой сборки |
| macOS app в `bin-new` | Ad-hoc, без TeamIdentifier и sealed resources | Не подготовленный Developer ID релиз |

Ключевые исходники: `src/validation.cpp`, `src/net_processing.cpp`, `src/zerohour/zerohourstate.{h,cpp}`, `src/wallet/`, `src/qt/`. Большие существующие файлы не дробить в этом цикле: рефакторинг усложнит проверку отсутствия изменений поведения.

## 2. Очередность и зависимости

| Этап | Приоритет | Зависит от | Результат |
| --- | --- | --- | --- |
| A. Снимок и отчёт | P0 | — | Воспроизводимое описание входов |
| B. Восстановление unit-сборки | P0 | A | Собранный и выполняемый test_zerohour |
| C. BDB и build isolation | P0 | A | Совместимые зависимости из собственного prefix |
| D. Subsidy и P2P gate | P0 | B, C | Проверки границ и реального поведения узлов |
| E. EVM, PoS, кошельки | P0 | B, C | Совместимость критических сценариев |
| F. Матрица платформ и CI | P1 | D, E | Повторяемая проверка одного SHA |
| G. Qt и macOS-пакет | P1 | E, F | Проверенный пакет на реальном Mac |
| H. Документация и гигиена сборки | P1 | B–G | Инструкции совпадают с проверенным процессом |
| I. Release gate | P0 | Все этапы | Доказательная оценка готовности |

Каждый этап заканчивается самостоятельным diff, результатами проверок и отдельным небольшим коммитом при исполнении плана. Не включать в него посторонние untracked логи и бинарники.

## 3. Пошаговый план

### A. Зафиксировать исходный снимок

**Файлы:** создать `docs/release/modern-build-verification.md`; читать `README.md`, `config.status`, `depends/packages/*.mk` и существующие build logs.

**Вход:** текущий checkout и имеющиеся артефакты. **Выход:** таблица «SHA → toolchain/dependencies → команды → результаты → артефакты» для остальных этапов.

- [ ] Записать SHA, ветку и tracked/untracked status отдельно. Не путать наличие untracked артефактов с чистым release checkout.
- [ ] Записать ОС, архитектуру, compiler/SDK, configure flags и реальные prefix библиотек; абсолютные пользовательские пути в публичном отчёте заменить описанием роли каталога.
- [ ] Записать версии зависимостей из `depends/packages`: BDB 4.8.30, OpenSSL 1.1.1w, Boost 1.64.0, Qt 5.9.7, libevent 2.1.8-stable, ZeroMQ 4.3.1 как baseline, без заявления об их актуальности или отсутствии уязвимостей.
- [ ] Снять хеши существующих бинарников, явно обозначив их как старые артефакты с неподтверждённой связью с SHA.

```bash
git rev-parse HEAD
git branch --show-current
git status --short
git diff --check
file src/zerohourd src/zerohour-cli src/qt/zerohour-qt
shasum -a 256 src/zerohourd src/zerohour-cli src/qt/zerohour-qt
```

**Приёмка:** другой разработчик может определить, какие исходники и бинарники проверялись; каждое утверждение имеет статус «код просмотрен», «тест прошёл», «runtime проверен» или «не проверено».

### B. Восстановить тестовую сборку и EVM cache regression

**Файлы:** `src/Makefile.test.include`; восстановить/создать `src/test/evm_state_cache_tests.cpp`; при необходимости `src/test/test_bitcoin.{h,cpp}` для fixture. Читать `src/zerohour/zerohourstate.{h,cpp}`, `src/test/main_tests.cpp`, `src/test/net_tests.cpp`.

**Вход:** baseline A. **Выход:** полный исходный состав unit target и `src/test/test_zerohour`, построенный из проверяемого SHA.

- [ ] Воспроизвести missing-source проблему в изолированной сборке с включёнными tests; сохранить точную ошибку. Не запускать `make check` в tests-disabled конфигурации как доказательство покрытия.
- [ ] Проверить историю отсутствующего теста. Если исходник найден, восстановить содержательный тест и проверить его применимость; если нет — написать fixture на основе `TestingSetup` и текущего `ZHCASHState`. Не удалять строку Makefile только ради зелёной сборки.
- [ ] Добавить тест сброса UTXO cache: установить cache entry через `setCacheUTXO`, сменить root через `setRootUTXO`, проверить отсутствие старой записи через публичный интерфейс чтения.
- [ ] Добавить проверки `execute(..., Permanence::Reverted)`: roots и наблюдаемое состояние до/после одинаковы; повторный вызов не получает остаточное состояние предыдущего исполнения.
- [ ] Проверить исключение исполнения по обе стороны существующего `nFixUTXOCacheHFHeight`, сохраняя историческое различие поведения. Отдельно проверить committed execution и последующее чтение/reload.
- [ ] До проверки семантики зафиксировать ожидаемые roots, balances, refunds и receipts по контролируемой fixture/совместимому baseline. Если ожидания расходятся с кодом, зарегистрировать дефект отдельно; не менять консенсус для подгонки теста.
- [ ] Собрать tests-enabled конфигурацию и выполнить целевые suites, затем полный unit gate.

```bash
./autogen.sh
CONFIG_SITE="$PWD/depends/x86_64-pc-linux-gnu/share/config.site" \
  ./configure --without-gui --enable-tests
make -j2
src/test/test_zerohour --run_test=evm_state_cache_tests --log_level=test_suite
src/test/test_zerohour --run_test=main_tests,net_tests --log_level=test_suite
make check
```

Команды выше предназначены для Linux после создания собственного prefix на этапе C. В остальных ОС использовать конфигурацию соответствующей платформы с включёнными tests.

**Приёмка:** все перечисленные test sources существуют; целевые suites реально исполняются; полный `make check` завершается успешно, число выполненных тестов и SHA записаны в отчёт.

### C. Закрепить BDB 4.8 и изолировать сборку

**Файлы:** `contrib/devtools/build-macos-arm64.sh`, `doc/build-osx-arm64.md`, `build-aux/m4/bitcoin_find_bdb48.m4`, при необходимости `depends/hosts/darwin.mk` и `depends/packages/bdb.mk`; создать `test/build/test_macos_helper.py`.

**Вход:** ограничения совместимости и собственный dependency prefix. **Выход:** build helper, который не разрешает неподходящую BDB по умолчанию и не зависит от соседнего checkout.

- [ ] Сначала добавить subprocess-тест helper с подставными `uname`, `brew`, `make` и `configure` в временном каталоге. Проверять переданные argv, выбор prefix и ненулевой выход при отсутствующих headers/libs; реальные пакеты не устанавливать из этого теста.
- [ ] Удалить `--with-incompatible-bdb` из default helper flags и поддерживаемых ручных команд. Не добавлять автоматический fallback на BDB 5/6.
- [ ] Проверить версию headers и реально связанной библиотеки, включая путь с явно заданными `BDB_CFLAGS`/`BDB_LIBS`, который сейчас обходит autodetection.
- [ ] Подтвердить положительный сценарий BDB 4.8 и отрицательный сценарий с иной версией. Неправильная версия должна останавливать поддерживаемый release build до открытия кошелька.
- [ ] Дать helper явный поддерживаемый путь к собственным BDB/OpenSSL/Qt dependencies; исключить принудительное затирание корректного caller-provided prefix. Сохранить проверку macOS/arm64.
- [ ] Выполнить clean build на отдельном builder без каталога соседнего проекта. Сначала daemon/CLI, затем GUI. Не исправлять сгенерированный `config.status` вручную.

Целевая конфигурация для ручного native macOS CLI после установки совместимых библиотек:

```bash
./configure --without-gui --disable-bip70 --enable-tests \
  CPPFLAGS="-I${OPENSSL_PREFIX}/include -I${BDB_PREFIX}/include" \
  LDFLAGS="-L${OPENSSL_PREFIX}/lib -L${BDB_PREFIX}/lib"
make -j2
make check
python3 -m unittest discover -s test/build -p 'test_*.py'
```

`OPENSSL_PREFIX` и `BDB_PREFIX` — пути к подготовленным OpenSSL 1.1.1w и BDB 4.8 в собственном окружении builder; не произвольные библиотеки с тем же именем.

**Приёмка:** BDB 4.8 проходит, неподходящая версия отвергается, helper tests проходят; новый configure log и linkage не ссылаются на соседний checkout. Чистая сборка не требует сохранённых локальных Makefile/config.status.

### D. Подтвердить subsidy и peer upgrade gate

**Файлы:** `src/test/main_tests.cpp`, `src/test/net_tests.cpp`; создать `test/functional/p2p_fork_min_protocol.py`; зарегистрировать его в `test/functional/test_runner.py`. Читать `src/validation.cpp`, `src/version.h`, `src/net_processing.cpp`, `src/chainparams.cpp`, `test/functional/test_framework/messages.py`.

**Вход:** новый tests-enabled бинарник. **Выход:** unit boundary coverage и сетевой functional regression.

- [ ] Проверить существующие утверждения для mainnet и testnet: PoW boundary, 1 699 999/1 700 000, 2 499 999/2 500 000, 3 499 999/3 500 000, 4 499 999/4 500 000, 5 499 999/5 500 000, 6 499 999/6 500 000 и 10 000 000.
- [ ] Сохранить отдельные проверки interval-based subsidy для regtest/других интервалов. Проверить сумму эмиссии по существующим ожиданиям и допустимые суммы, не менять `MAX_MONEY` на основании одного сравнения total supply.
- [ ] Дополнить проверки реального пути `CheckReward`: блок с допустимой наградой принимается, завышенная награда отвергается с `bad-cb-amount`/`bad-cs-amount`. Использовать контролируемый block fixture для обеих сторон границы; не добавлять production override высот консенсуса ради тестирования.
- [ ] В P2P test установить существующие параметры `-forkminpeerheight=2` и `-forkminpeerversion=70018` на regtest. Подключить peer 70017 до высоты 2; после достижения высоты 2 проверить его отключение, отказ новому 70017 и успешную работу peer 70018.
- [ ] Проверить изменение параметров на height 3 / version 70019, перезапуск и поведение после отката ниже порога: функция использует текущую высоту, а не необратимо сохранённую активацию.
- [ ] Задавать версию каждого synthetic peer явно: framework сейчас использует `MY_VERSION=70017`; простая глобальная замена уничтожит old-peer сценарий.

Пример граничного утверждения unit-теста с default аргументами:

```cpp
BOOST_CHECK(!ShouldDisconnectPeerForForkMinProtocol(70017, 1699999));
BOOST_CHECK(ShouldDisconnectPeerForForkMinProtocol(70017, 1700000));
BOOST_CHECK(!ShouldDisconnectPeerForForkMinProtocol(70018, 1700000));
```

```bash
src/test/test_zerohour --run_test=main_tests,net_tests --log_level=test_suite
PYTHONDONTWRITEBYTECODE=1 python3 test/functional/test_runner.py \
  p2p_fork_min_protocol.py --jobs=1
```

**Приёмка:** проходят как чистые функции, так и handshake/disconnect и блоковая проверка награды. Отключение P2P gate не отменяет subsidy validation. Протокол peer сам по себе не считается доказательством его правил консенсуса.

### E. Проверить EVM, PoS и совместимость кошельков

**Файлы:** существующие `test/functional/qtum_callcontract.py`, `qtum_createcontract.py`, `qtum_evm_revert.py`, `qtum_dgp.py`, `qtum_pos.py`, `wallet_encryption.py`, `wallet_backup.py`; создать `docs/release/wallet-compatibility.md`; при найденном пробеле расширять соответствующий тест, не создавать дубли.

**Вход:** сборка C и unit coverage B/D. **Выход:** результаты критических functional сценариев и матрица legacy-wallet compatibility.

- [ ] Выполнить создание/вызов контракта, revert, чтение после повторного RPC и рестарта; сверить roots, balances, gas/refunds и receipts с fixture ожиданиями.
- [ ] Проверить PoS: создание и принятие stake block, maturity, некорректную награду, повторное использование stake и согласованность двух узлов.
- [ ] Выполнить wallet encryption/backup. Дополнить сценарии неверного пароля, unlock/lock, рестарта и восстановления backup, если существующие assertions их не покрывают.
- [ ] Проверить копии зашифрованных BDB 4.8 кошельков от каждой реально поддерживаемой предыдущей версии: открыть, сверить адреса/баланс, разблокировать, подписать проверочное сообщение, заблокировать, перезапустить и повторить чтение.
- [ ] Зафиксировать версии старых wallet fixtures и их происхождение. Если копии отсутствуют, оставить gate «не проверено», не заменять его новым кошельком текущей версии.
- [ ] Проверять возможность открытия старым бинарником только на дополнительной копии и только в рамках заявленной backward compatibility. Не рекомендовать downgrade mainnet-узла через consensus activation.

```bash
PYTHONDONTWRITEBYTECODE=1 python3 test/functional/test_runner.py \
  qtum_callcontract.py qtum_createcontract.py qtum_evm_revert.py \
  qtum_dgp.py qtum_pos.py wallet_encryption.py wallet_backup.py --jobs=2
```

**Приёмка:** каждое критическое действие имеет проверенный результат, а не только успешный запуск процесса. Все изменения поведения EVM/PoS, обнаруженные проверками, отделены от разрешённых build/test исправлений.

### F. Ввести повторяемую матрицу платформ

**Файлы:** создать `.github/workflows/modern-build.yml`, `contrib/devtools/verify-modern-build.sh`; дополнить `docs/release/modern-build-verification.md`. CI включать в существующий механизм репозитория, если к моменту исполнения он появится.

**Вход:** stages B–E. **Выход:** один SHA, проверяемый на всех заявленных платформах, и сохраняемые logs/checksums.

- [ ] Создать общий verification script с `set -euo pipefail`, проверкой наличия `src/test/test_zerohour`, запуском `make check` и functional runner; отсутствие тестового бинарника должно давать ошибку, а не пропуск.
- [ ] Добавить Linux Ubuntu 24.04 headless и Qt jobs; BDB/OpenSSL брать из собственного `depends` prefix. Отдельно проверять сборку без dependency cache.
- [ ] Добавить Windows cross-build job и отдельный native Windows runtime gate. Не считать успешный MinGW link выполнением Windows tests.
- [ ] Добавить macOS ARM64 native build, unit/functional runtime и GUI smoke на реальном Apple Silicon runner.
- [ ] Формировать cache key из target, compiler/SDK, содержимого package recipes и patches; не восстанавливать неподтверждённый cache между несовместимыми targets.
- [ ] Записывать команды, exit codes и число тестов. Внешние actions закреплять на проверенные SHA при реализации; не угадывать action versions в этом плане.
- [ ] Выполнить полный обычный и расширенный functional набор; фиксировать список выбранных тестов и причины любых skips. Mandatory consensus/wallet сценарии не могут быть пропущены в release gate.

```bash
make check
PYTHONDONTWRITEBYTECODE=1 python3 test/functional/test_runner.py --jobs=2
PYTHONDONTWRITEBYTECODE=1 python3 test/functional/test_runner.py --extended --jobs=2
```

| Target | Build | Unit | Functional | GUI | Legacy wallet |
| --- | --- | --- | --- | --- | --- |
| Ubuntu 24.04 x86_64 headless | Обязательно | Обязательно | Обязательно | Не применяется | Обязательно |
| Ubuntu 24.04 x86_64 Qt | Обязательно | Обязательно | Критический набор | Обязательно | Обязательно |
| Windows x86_64 | Обязательно | На Windows | На Windows | Обязательно | Обязательно |
| macOS arm64 | Обязательно | На ARM64 Mac | На ARM64 Mac | Обязательно | Обязательно |

**Приёмка:** CI не бывает зелёным из-за `--disable-tests`; опубликованный verification report различает build-only и runtime evidence. Для недоступного runner gate остаётся открытым с конкретной причиной.

### G. Проверить Qt и подготовить macOS release artifacts

**Файлы:** `doc/build-osx-arm64.md`, `contrib/macdeploy/macdeployqtplus`, `Makefile.am` — только при воспроизведённой ошибке упаковки; создать `contrib/devtools/verify-macos-release.sh` и `docs/release/macos-release.md`.

**Вход:** тот же прошедший проверки SHA из F. **Выход:** проверенные app/DMG с traceable hashes и результатами подписи.

- [ ] Запустить Qt с временным regtest datadir: создать/открыть кошелёк, показать адрес, получить тестовые монеты, отправить между тестовыми кошельками, проверить restart и закрытие.
- [ ] Повторно проверить работу contract/token экранов и ошибки RPC, не трактовать запуск окна как GUI acceptance.
- [ ] Выполнить `make appbundle` и `make deploy`, сохранять результат отдельно от старого `bin-new`. Если нужен ручной обход packaging, превратить его в воспроизводимый скрипт с объяснением ошибки исходного пути.
- [ ] Проверить через `otool -L` main executable, bundled Qt frameworks и plugins: отсутствуют ссылки на пользовательские build prefixes; запуск на чистом Mac без Homebrew проходит.
- [ ] Подписывать nested code изнутри наружу проверенной Developer ID identity, затем bundle; проверить hardened runtime/entitlements под используемый Qt. Не использовать ad-hoc как замену release signature.
- [ ] После предоставления signing identity и настроенного keychain profile выполнить notarization, staple и проверку Gatekeeper. Если учётные данные отсутствуют, подготовить неподписанный candidate и явно оставить signing gate открытым.
- [ ] Рассчитать SHA-256 финальных артефактов после всех изменений подписи/stapling, связать их с SHA и verification report. Не публиковать автоматически.

Команды проверки после упаковки и подписи:

```bash
codesign --verify --deep --strict --verbose=2 ZHCASH-Qt.app
codesign -dv --verbose=4 ZHCASH-Qt.app
spctl --assess --type execute --verbose=4 ZHCASH-Qt.app
xcrun stapler validate ZHCASH-Qt.app
hdiutil verify ZHCASH-Core.dmg
shasum -a 256 ZHCASH-Core.dmg
```

**Приёмка:** Developer ID identity, sealed resources и успешная проверка нотариального билета; DMG открывается на чистом ARM64 Mac, установленное приложение проходит реальные wallet/GUI сценарии. Проверка подписи не подменяет functional tests.

### H. Синхронизировать документацию и build hygiene

**Файлы:** `README.md`, `doc/build-osx-arm64.md`, `src/test/README.md`, `doc/release-notes.md`, `.gitignore`, `docs/release/modern-build-verification.md`.

**Вход:** реальные команды и результаты B–G. **Выход:** точные инструкции и отделение исходников от локальных артефактов.

- [ ] Исправить названия unit executable в `src/test/README.md`: фактическая цель — `test_zerohour`, а не унаследованное `test_bitcoin`; GUI target сверить с Makefile перед документированием.
- [ ] Убрать default BDB bypass из всех поддерживаемых build recipes. На чистой машине повторить README-команды для каждого заявленного target.
- [ ] Зафиксировать реальные dependency versions, назначение Qt `-no-openssl` и отсутствие проверки обновления библиотек этим аудитом. Не обещать современные версии там, где используются legacy pins.
- [ ] Отдельным изменением уточнить ignore rules для `.DS_Store`, локальных build logs, временных verification outputs. Проверить, что они не скрывают новые source/test файлы.
- [ ] Инвентаризировать tracked `autom4te.cache` и `depends/built`; сформулировать политику доставки cache и его проверки. Не удалять tracked cache автоматически: README сейчас заявляет его повторное использование.
- [ ] Описать mandatory upgrade, параметры peer gate и границы rollback: восстановление backup/build artifact не означает безопасное возвращение к прежним правилам сети.
- [ ] В README обновлять только реально закрытые findings, сохраняя дату исходного аудита и ссылку на свежий report.

```bash
git diff --check
git status --short
```

**Приёмка:** инструкции повторяемы, имена файлов корректны, статусы не преувеличивают проверенность. Исправления документации не маскируют открытые release blockers.

### I. Финальная проверка готовности

**Файлы:** `docs/release/modern-build-verification.md`, `docs/release/macos-release.md`, `doc/release-notes.md`; создать `docs/release/modern-build-release-checklist.md`.

**Вход:** все этапы и финальный source SHA. **Выход:** reviewable candidate и однозначное решение «готов» либо перечень незакрытых gates.

- [ ] Проверить состав diff: нет случайных consensus/EVM/serialization изменений, соседних checkout-файлов и чувствительных данных.
- [ ] После последнего изменения кода повторить затронутые tests и полный release gate на том же SHA; не переносить старые passing результаты на новые бинарники.
- [ ] Проверить historical block/chainstate compatibility на выделенной копии snapshot с известными heights/hashes/state roots. Сверить результаты до и после рестарта/reindex по поддерживаемому сценарию.
- [ ] Проверить синхронизацию и работу peers в выделенном сетевом окружении, затем mainnet sync выделенного узла при наличии ресурсов; записать start/end heights/hashes и версии peers. Regtest не закрывает этот пункт.
- [ ] Проверить installed packages на целевых ОС, legacy wallets, checksums и принадлежность бинарников проверенному build.
- [ ] Записать оставшиеся ограничения по платформам, fixtures, signing и внешним ресурсам. Непроверенные обязательные пункты удерживают release gate открытым.
- [ ] Подготовить release notes и операторский upgrade checklist. Публикацию выполнять отдельным действием после принятия конкретных артефактов.

## 4. Критерии завершения

- [ ] Missing EVM test source восстановлен содержательно; tests-enabled clean build проходит.
- [ ] Все unit и обязательные functional проверки реально выполнены на release SHA.
- [ ] Subsidy boundaries, block reward rejection и реальный P2P gate подтверждены.
- [ ] EVM, PoS, исторические данные и legacy encrypted BDB wallets проверены.
- [ ] Build не зависит от соседнего checkout; BDB 4.8 проверяется, provenance dependencies сохранён.
- [ ] Linux/Windows/macOS имеют раздельные build/runtime результаты без скрытых skips.
- [ ] Qt и пакеты проверены на целевых ОС; macOS release подписан и нотариально заверен.
- [ ] Документация, release notes и checksum manifest относятся к тем же артефактам.

## 5. Что не входит в этот цикл

Массовый рефакторинг validation/wallet, смена build system на CMake, миграция wallet DB, перенос на OpenSSL 3/Qt 6, изменение токеномики и новые правила EVM/PoS. Для каждого такого направления нужен самостоятельный план совместимости после закрытия текущих release gates.
