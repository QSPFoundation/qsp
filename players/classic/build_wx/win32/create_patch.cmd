SET BUILD_PATH=%~dp0\..
SET WX_PATH=%BUILD_PATH%\wxWidgets

cd /d "%WX_PATH%"
svn diff > "%BUILD_PATH%\wxPatch.diff"
pause
