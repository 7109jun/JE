@echo off
setlocal
set "JE_EXE=%~dp0je.exe"
if not exist "%JE_EXE%" (
    echo je.exe was not found next to this installer.
    exit /b 1
)
reg add "HKCU\Software\Classes\.jxj" /ve /d "JE.File" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\JE.File" /ve /d "JE Executable File" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\JE.File\shell\open\command" /ve /d "\"%JE_EXE%\" \"%%1\"" /f >nul || exit /b 1
reg add "HKCU\Software\Classes\JE.File\DefaultIcon" /ve /d "\"%JE_EXE%\",0" /f >nul || exit /b 1
assoc .jxj=JE.File >nul 2>&1
ftype JE.File="%JE_EXE%" "%%1" >nul 2>&1
echo .jxj files are now associated with JE.
echo Double-clicking a .jxj file will run it with je.exe.
endlocal
