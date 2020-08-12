QSP
===

QSP Library

**Linux build:**
```
mkdir build && cd build
cmake -DBUILD_QSPGUI=ON -DBUILD_TXT2GAM=ON ..
make
```

**TODO:**
* Check build on Windows
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
