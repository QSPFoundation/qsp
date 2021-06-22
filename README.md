# QSP

## Linux build

```bash
mkdir build && cd build
cmake -DBUILD_QSPGUI=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Windows build

```bash
mkdir build
cd build
cmake -G "Visual Studio 15 2017" -A Win32 -DBUILD_QSPGUI=ON -DCMAKE_INSTALL_PREFIX=out ..
cmake --build . --target install --config Release
```

## TODO

* Test locals implementation
* Test loop implementation
* Update bindings
* Implement a new way to communicate with GUI (more specific callbacks)
* Move the whole global state into a context parameter
* Move a QSP player into a separate repository
* Document new features
* ???

## Chat group

https://discord.gg/vyyyTrd
