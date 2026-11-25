@echo off
setlocal
cd /d "%~dp0"
set "PATH=%PATH%;%LOCALAPPDATA%\bin\NASM"

if "%~1"=="" (
    echo Uso: rg ^<archivo.rg^>
    echo.
    echo   rg tests\correctos\p4_if_else.rg
    echo   rg p4_if_else
    echo   rg p4
    exit /b 1
)

rem --- admite ruta completa, nombre sin extension, o prefijo ---
set "SRC=%~1"
if not exist "%SRC%" if exist "tests\correctos\%~1.rg" set "SRC=tests\correctos\%~1.rg"
if not exist "%SRC%" for %%f in ("tests\correctos\%~1*.rg") do set "SRC=%%f"
if not exist "%SRC%" for %%f in ("tests\errores-ejecucion\%~1*.rg") do set "SRC=%%f"
if not exist "%SRC%" for %%f in ("tests\errores-semanticos\%~1*.rg") do set "SRC=%%f"
if not exist "%SRC%" for %%f in ("tests\errores-lexicos-sintacticos\%~1*.rg") do set "SRC=%%f"

if not exist "%SRC%" (
    echo No encontre el archivo: %~1
    exit /b 1
)

echo === %SRC%
echo.

compilador.exe "%SRC%"
if errorlevel 1 (
    echo.
    echo [se detuvo aca: el compilador informo errores, no se genera ejecutable]
    exit /b 1
)

nasm -f win32 -o out\programa.obj out\programa.asm || exit /b 1
gcc -m32 -o out\programa.exe out\programa.obj || exit /b 1

echo.
echo --- salida del programa ---
out\programa.exe