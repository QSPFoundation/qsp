rd /s /q _debug
rd /s /q _release
xcopy misc\win32 _debug /E /I
xcopy misc\win32 _release /E /I
xcopy misc\common _debug /E /I
xcopy misc\common _release /E /I
rem ---------------------------------
rd /s /q qsp\.objs
del qsp\win32_qsp.layout
del qsp\win32_qsp.depend
rd /s /q qspgui\.objs
del qspgui\win32_qspgui.layout
rem ---------------------------------
del win32_vs.ncb
del /a:h win32_vs.suo
del qsp\*.user
del qspgui\*.user
