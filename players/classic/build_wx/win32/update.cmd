SET MSYS=c:\msys
SET VC=c:\Program Files\Microsoft Visual Studio 9.0

SET BUILD_PATH=%~dp0\..
SET WX_PATH=%BUILD_PATH%\wxWidgets

SET PATH=%PATH%;%MSYS%\bin
rem --------------------------------------------------------------------------
patch -i "%BUILD_PATH%\wxPatch.diff" -d "%WX_PATH%" -p 0
cd /d "%WX_PATH%\build\msw"
rem MinGW --------------------------------------------------------------------
mingw32-make -f makefile.gcc BUILD=release UNICODE=1 RUNTIME_LIBS=static
rem VC -----------------------------------------------------------------------
call "%VC%\Common7\Tools\vsvars32.bat"
nmake -f makefile.vc BUILD=debug UNICODE=1 RUNTIME_LIBS=static
nmake -f makefile.vc BUILD=release UNICODE=1 RUNTIME_LIBS=static
nmake -f makefile.vc BUILD=release UNICODE=0 RUNTIME_LIBS=static
rem --------------------------------------------------------------------------
pause
