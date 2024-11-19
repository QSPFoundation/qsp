# QSP

This repository contains source code of the QSP game engine.

## Useful links

### Documentation

* Docs: https://github.com/QSPFoundation/qspfoundation.github.io

### Players

* Classic QSP player: https://github.com/QSPFoundation/qspgui
* QSpider: https://github.com/QSPFoundation/qspider

### Dev tools

* QGen: https://github.com/QSPFoundation/qgen
* TXT2GAM: https://github.com/QSPFoundation/txt2gam
* VSCode extension: https://github.com/QSPFoundation/Qsp.FSharp.VsCode
* CLI tools: https://github.com/QSPFoundation/qsp-cli
* Test engine: https://github.com/QSPFoundation/qsp-test-engine
* Code analyzer: https://github.com/QSPFoundation/Qsp.FSharp

## Linux & MacOS build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Windows build

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 -DCMAKE_INSTALL_PREFIX=out
cmake --build build --target install --config Release
```

## Support us

Please consider supporting our development on:
* Buy me a coffee: https://buymeacoffee.com/varg
* Ethereum/EVM: 0x4537B99e27deD6C8459C1eFCdE0E7fa48357e44D
* [![PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=RB8B6EQW4FW6N)

## TODO

* A tool to automatically migrate older games (e.g. QSP 5.7) to the latest version of the engine
* Extra tests (see [qsp-wasm-engine](https://github.com/QSPFoundation/qsp-wasm-engine/tree/main/tests))
* Update the website
* Multi-user games (MUD-style games)
* New documentation & document new features
* Update bindings
* Implement a new way to communicate with GUI (more specific callbacks)
* Move the whole global state into a context parameter
* ???

## Chat group

https://discord.gg/6gWVYUtUGZ
