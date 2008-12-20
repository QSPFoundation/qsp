SET MSYS=c:\msys
SET VC=c:\Program Files\Microsoft Visual Studio 9.0
rem --------------------------------------------------------------------------
d:
cd d:\wx
svn up wxWidgets
rd /s /q wx_qsp
svn export wxWidgets wx_qsp
"%MSYS%\bin\patch" -i d:\wx\wxPatch.diff -d d:\wx\wx_qsp -p 3 --binary
cd wx_qsp\build\msw
rem MinGW --------------------------------------------------------------------
SET PATH=%MSYS%\bin;%PATH%
mingw32-make -f makefile.gcc BUILD=release UNICODE=1 RUNTIME_LIBS=static
rem VC -----------------------------------------------------------------------
call "%VC%\Common7\Tools\vsvars32.bat"
nmake -f makefile.vc BUILD=debug UNICODE=1 RUNTIME_LIBS=static
nmake -f makefile.vc BUILD=release UNICODE=1 RUNTIME_LIBS=static
nmake -f makefile.vc BUILD=release UNICODE=0 RUNTIME_LIBS=static
rem --------------------------------------------------------------------------
pause
