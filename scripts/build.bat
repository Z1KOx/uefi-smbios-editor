@echo off
setlocal EnableExtensions

for %%I in ("%~dp0..") do set "PROJECT=%%~fI"

set "EDK2=C:\edk2"
set "SRC=%PROJECT%\src"
set "EDK2_PKG=%EDK2%\UefiSmbiosEditorPkg"
set "OUT=%PROJECT%\build"

set "PKG_NAME=UefiSmbiosEditor"
set "EFI_NAME=uefismbios"
set "TOOLCHAIN=VS2026"
set "ARCH=X64"
set "BUILDTARGET=DEBUG"

if not exist "%EDK2%\edksetup.bat" (
    echo ERROR: EDK2 not found at %EDK2%
    pause
    exit /b 1
)

for %%F in ("%SRC%\main.c" "%SRC%\UefiSmbiosEditor.inf" "%SRC%\UefiSmbiosEditor.dsc") do (
    if not exist %%F (
        echo ERROR: Missing file: %%~F
        echo.
        echo If UefiSmbiosEditor.dsc is still in the project root,
        echo move it into src\ first.
        pause
        exit /b 1
    )
)

echo [1/4] Copying project files into the EDK2 tree...

if exist "%SRC%\Main.c" ren "%SRC%\Main.c" main.c >nul 2>&1

robocopy "%SRC%" "%EDK2_PKG%" /MIR /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 (
    echo ERROR: Could not copy files into %EDK2_PKG%
    pause
    exit /b 1
)

if exist "%EDK2_PKG%\Main.c" ren "%EDK2_PKG%\Main.c" main.c >nul 2>&1

echo [2/4] Initializing EDK2...

cd /d "%EDK2%"
set "WORKSPACE=%EDK2%"

if exist "%EDK2%\BaseTools\Bin\Win32\GenFw.exe" (
    set "EDK_TOOLS_BIN=%EDK2%\BaseTools\Bin\Win32"
)

call edksetup.bat

if not exist "%EDK2%\BaseTools\Bin\Win32\GenFw.exe" (
    echo BaseTools binaries are missing - building them once...
    call edksetup.bat Rebuild %TOOLCHAIN%
)

if not exist "%EDK2%\BaseTools\Bin\Win32\GenFw.exe" (
    echo ERROR: BaseTools binaries are still missing after rebuild.
    pause
    exit /b 1
)

echo [3/4] Building %PKG_NAME%...

call build ^
    -p UefiSmbiosEditorPkg\UefiSmbiosEditor.dsc ^
    -a %ARCH% ^
    -b %BUILDTARGET% ^
    -t %TOOLCHAIN%

if errorlevel 1 (
    echo ERROR: Build failed.
    pause
    exit /b 1
)

echo [4/4] Collecting %EFI_NAME%.efi...

set "BUILT_EFI=%EDK2%\Build\%PKG_NAME%\%BUILDTARGET%_%TOOLCHAIN%\%ARCH%\%EFI_NAME%.efi"

if not exist "%BUILT_EFI%" (
    set "BUILT_EFI="
    for /r "%EDK2%\Build\%PKG_NAME%" %%F in (*.efi) do (
        if /i "%%~nxF"=="%EFI_NAME%.efi" (
            if not defined BUILT_EFI set "BUILT_EFI=%%F"
        )
    )
)

if not defined BUILT_EFI (
    echo ERROR: %EFI_NAME%.efi was not produced by the build.
    echo Expected location:
    echo %EDK2%\Build\%PKG_NAME%\%BUILDTARGET%_%TOOLCHAIN%\%ARCH%
    pause
    exit /b 1
)

if not exist "%OUT%" mkdir "%OUT%"

del /q "%OUT%\UefiSmbiosEditor.efi" >nul 2>&1
copy /Y "%BUILT_EFI%" "%OUT%\%EFI_NAME%.efi" >nul

if errorlevel 1 (
    echo ERROR: Could not copy the EFI file to %OUT%.
    pause
    exit /b 1
)

echo.
echo ==========================================
echo  BUILD SUCCEEDED
echo ==========================================
echo  EFI file:   %OUT%\%EFI_NAME%.efi
echo  Built from: %BUILT_EFI%
echo ==========================================

pause
endlocal