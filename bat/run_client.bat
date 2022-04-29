@echo off

tasklist /fi "ImageName eq Client_CCVDE.exe" /fo csv 2>NUL | find /I "Client_CCVDE.exe">NUL

if "%ERRORLEVEL%"=="0" (
    start init-error.vbs
) else (
    cd ..
    start Client_CCVDE.exe
)