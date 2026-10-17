# Tests of qsp

The tests are scenario files (`engine/*.qspt`) executed by a small runner
(`runner/qsp_test_runner.c`) against the public API of the library
(the default binding). Most of them are ported from
[qsp-wasm-engine](https://github.com/QSPFoundation/qsp-wasm-engine/tree/main/tests/engine);
where the expectation differs from the original test, the original is
commented in place (`wasm: ...`) with the reason. The runner and the format
come from the tests of qsp-legacy.

## Building and running

```sh
cmake -B build -DQSP_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Every `.qspt` file is registered as a separate CTest test (`engine.<name>`).
The runner can also be called directly: `build/tests/qsp-tests FILE.qspt...`.
It prints the failed checks with the events recorded so far, and the
summary `N passed, M failed, K skipped`; the exit code is non-zero if
anything failed. The runner writes its files (`@save-file`) to the current
directory.

The tests require the default binding (`BUILD_JAVA=OFF`).

## Format of `.qspt` files

A file is a sequence of tests. Lines starting with `//` and empty lines
between commands are ignored. Every command starts with `@` at the start
of the line. Commands marked as *block* take the following lines up to the
next `@`-line as their text; `@@` at the start of a block line stands for a
literal `@`, trailing empty lines and `//` lines are not part of the block.

The engine is reinitialized (`QSPTerminate`/`QSPInit`) for every test;
the game is built from the definitions and loaded right before the first
action of the test.

### Test and game definition

| Command | Meaning |
|---|---|
| `@test NAME` | starts a new test |
| `@src` *(block)* | code of location `test`; the game also gets an empty first location `start` (the same wrapper as `runTestFile()` in qsp-wasm-engine) |
| `@game` *(block)* | locations in the qsps format: `# name`, code lines, a line starting with `--` |
| `@module NAME` *(block)* | a separate game in the qsps format, loaded when the game calls `INCLIB 'NAME'` / `OPENQST 'NAME'` |
| `@desc LOC` *(block)* | base description of location `LOC` |
| `@action LOC \| NAME [\| IMAGE]` *(block)* | base action of location `LOC`, the block is its code |
| `@skip REASON` | the test is skipped; the reason says why (e.g. `ENGINE BUG: ...`) |

### Replies of the "user"

| Command | Meaning |
|---|---|
| `@reply input TEXT` | queues a reply to `INPUT` (escapes `\n`, `\r`, `\\`) |
| `@reply version TEXT` | queues a reply to the version callback (`$QSPVER`) |
| `@reply menu N` | queues the item index chosen in `MENU` (`-1` closes the menu) |
| `@reply isplay 0/1` | queues a reply to the "is playing" callback |
| `@reply mscount N` | value returned by `MSECSCOUNT` |
| `@watch-msg NAME` | every `MSG` callback also records the event `read NAME => VALUE`, the variable is read with `QSPGetVarValue` while the callback runs (`$NAME`, `#NAME`, `%NAME` give the typed view) |

### Actions

| Command | API call |
|---|---|
| `@run [LOC]` | `QSPExecLocationCode` (default location `test`) |
| `@exec CODE` | `QSPExecString` |
| `@counter` | `QSPExecCounter` |
| `@set-input TEXT` | `QSPSetInputStrText` |
| `@user-input` | `QSPExecUserInput` |
| `@select-act N` / `@exec-act` | `QSPSetSelActionIndex` / `QSPExecuteSelActionCode` |
| `@select-obj N` | `QSPSetSelObjectIndex` |
| `@restart` | `QSPRestartGame` |
| `@save NAME` / `@load-save NAME` | `QSPSaveGameAsData` / `QSPOpenSavedGameFromData` into/from an in-memory slot |
| `@save-file FILE` / `@load-file FILE` | the same with a file in the current directory (the file is written only when the save succeeds) |
| `@debug 0/1` | `QSPEnableDebugMode` |
| `@clear-events` | forgets the recorded events |

`SAVEGAME` and `OPENGAME` of the game use the in-memory slots too. A save
cancelled by `$ONGSAVE` (it moved to another location) is recorded as the
event `save_cancelled`.

An error of the engine is recorded as the event `error NAME` and must be
consumed with `@expect error`; an unconsumed error fails the test at the
next action or at the end of the test. `NAME` is the `QSP_ERR_*` name
without the prefix.

### Checks

| Command | Checks |
|---|---|
| `@expect EXPR => VALUE` | value of a QSP expression, see below |
| `@expect main TEXT` / `@expect stat TEXT` | main / additional description (escaped) |
| `@expect acts [a; b\|img]` / `@expect objs [...]` | current actions / objects; an object shows ` => TITLE` when its title differs from the name |
| `@expect selact N` / `@expect selobj N` | selected action / object |
| `@expect curloc NAME` | current location (`$CURLOC`) |
| `@expect error NAME [(location 'L', line N, local N, action A) LINE]` | the pending error, by name or with its position: `line` is the top-level line, `local` the line of the actual code, `LINE` its text (a prefix of the description is enough) |
| `@expect event TEXT` / `@expect no-event PREFIX` | an event was / no event starting with `PREFIX` was recorded |
| `@expect count N PREFIX` | number of events starting with `PREFIX` |
| `@expect events` *(block)* | the exact list of recorded events |

`@expect EXPR => VALUE` evaluates `QSPT_EXPECT_RESULT = (EXPR)` with
`QSPExecString` and compares the stored value (then the variable is
removed). `VALUE` is a QSP literal: a number, a string in single quotes
(`''` is a quote, `\n`, `\r`, `\\` are escapes), a tuple `[1, 'a']` or the
undefined value `?`. The type has to match: a name without a prefix gives
the value as it is, so `x => ?` checks that `x` has no value.
`readVariable('x')` of qsp-wasm-engine reads the numeric view of a
variable, it's written here as `#x => 0`; `readVariable('$x')` is
`$x => ''`.

### Events

Callbacks are recorded with the names used by qsp-wasm-engine:
`main_changed [TEXT]`, `stats_changed [TEXT]`, `actions_changed [...]`,
`objects_changed [...]`, `msg TEXT`, `view [FILE]`, `system_cmd TEXT`,
`user_input [TEXT]`, `input TEXT`, `version [PARAM]`, `menu [...]`,
`wait N`, `timer N`, `play_file FILE VOLUME`, `is_play FILE`,
`close_file [FILE]`, `panel_visibility TYPE 0/1`, `open_game FILE IS_NEW`,
`save_game [FILE]`, `load_save [FILE]`, `debug LOC:LINE:ACT CODE`,
`error NAME`. An argument in brackets is omitted when it's empty.
`panel_visibility` passes the `QSP_WIN_*` flags. The refresh callback
records the windows marked by `QSPGetWindowsChangedState`, or every window
when the refresh is forced, like `onRefresh()` of qsp-wasm-engine.
