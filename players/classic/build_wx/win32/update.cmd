SET WX_PATH=d:\wx\wxWidgets
SET WX_QSP_PATH=d:\wx\wx_qsp
SET MSYS=c:\msys
SET VC=c:\Program Files\Microsoft Visual Studio 9.0

SET QSP_PATH=%~dp0\..\..
rem --------------------------------------------------------------------------
svn up "%WX_PATH%"
rd /s /q "%WX_QSP_PATH%"
svn export "%WX_PATH%" "%WX_QSP_PATH%"
"%MSYS%\bin\patch" -i "%QSP_PATH%/build_wx/wxPatch.diff" -d "%WX_QSP_PATH%" -p 3 --binary
cd /d "%WX_QSP_PATH%\build\msw"
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
