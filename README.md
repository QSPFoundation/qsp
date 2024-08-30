# QSP

## Support us

Please consider supporting our development on:
* Buy me a coffee: https://buymeacoffee.com/varg

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

## TODO

* Update bindings
* Implement a new way to communicate with GUI (more specific callbacks)
* Move the whole global state into a context parameter
* Move the QSP player into a separate repository
* Document new features
* ???

## Chat group

https://discord.gg/6gWVYUtUGZ
