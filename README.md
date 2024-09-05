# QSP

## Linux build

```bash
mkdir build && cd build
cmake -DBUILD_QSPGUI=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

## Windows build

```bash
mkdir build
cd build
cmake -G "Visual Studio 15 2017" -A Win32 -DBUILD_QSPGUI=ON -DCMAKE_INSTALL_PREFIX=out ..
cmake --build . --target install --config Release
```

## Support us

Please consider supporting our development on:
* Buy me a coffee: https://buymeacoffee.com/varg
* Ethereum/EVM: 0x4537B99e27deD6C8459C1eFCdE0E7fa48357e44D
* [![PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=6NR6JYRHXJHRE)

## TODO

* A tool to automatically migrate older games (e.g. QSP 5.7) to the latest version of the engine
* Extra tests (see [qsp-wasm-engine](https://github.com/QSPFoundation/qsp-wasm-engine/tree/main/tests))
* Update the website
* Multi-user games (MUD-style games)
* New documentation & document new features
* Update bindings
* Implement a new way to communicate with GUI (more specific callbacks)
* Move the whole global state into a context parameter
* Move the QSP player into a separate repository
* ???

## Chat group

https://discord.gg/6gWVYUtUGZ
