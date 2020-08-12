QSP
===

QSP Library

**Linux build:**
```
mkdir build && cd build
cmake -DBUILD_QSPGUI=ON -DBUILD_TXT2GAM=ON ..
make
```

**Windows build:**
```
mkdir build
cd build
cmake.exe -G "Visual Studio 15 2017" -A Win32 -DBUILD_QSPGUI=ON -DBUILD_TXT2GAM=ON ..
MSBuild.exe ALL_BUILD.vcxproj
```

**TODO:**
* Put all the related files (DLLs/localization) into the output directory
* Test locals implementation
* Test loop implementation
* Update bindings
* Implement new way to communicate with GUI (more specific callbacks)
* Move global state into context parameter
* Make sure qspgui works with the latest wxWidgets fw
* Split QSP library and QSP player/editor into multiple repositories
* Build web player
* Build web editor
* Build new QGen
* Document new features
* Improve TXT2GAM to support UTF-8
* Improve TXT2GAM to join multiple text files (you can specify a file that contains a list of files to be joined)
* ???
