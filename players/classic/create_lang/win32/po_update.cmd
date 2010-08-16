dir /B ..\..\qspgui\src\*.cpp > files.txt
dir /B ..\..\qsp\src\*.c >> files.txt
xgettext -a --no-location -s --no-wrap -j -D..\..\qspgui\src -D..\..\qsp\src -ffiles.txt -o..\qspgui.po
del files.txt
