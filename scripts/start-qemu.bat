@echo off
setlocal EnableExtensions

for %%I in ("%~dp0..") do set "PROJECT=%%~fI"

set "EDK2=C:\edk2"
set "QEMU=C:\Program Files\qemu\qemu-system-x86_64.exe"

set "OVMF=%EDK2%\Build\OvmfX64\DEBUG_VS2026\FV\OVMF.fd"
set "EFI=%PROJECT%\build\uefismbios.efi"
set "SHARE=C:\uefi-test"

if not exist "%QEMU%" (
    echo ERROR: QEMU not found: %QEMU%
    pause
    exit /b 1
)

if not exist "%OVMF%" (
    echo ERROR: OVMF firmware not found: %OVMF%
    echo.
    echo Build it once from the VS prompt:
    echo   cd /d C:\edk2
    echo   call edksetup.bat
    echo   build -p OvmfPkg\OvmfPkgX64.dsc -a X64 -t VS2026 -b DEBUG
    echo.
    echo If OpenSSL or submodule errors appear, run this first:
    echo   git submodule update --init --recursive
    pause
    exit /b 1
)

if not exist "%EFI%" (
    echo ERROR: %EFI% not found.
    echo Run scripts\build.bat first.
    pause
    exit /b 1
)

if not exist "%SHARE%" mkdir "%SHARE%"

del /q "%SHARE%\*.efi" >nul 2>&1

copy /Y "%EFI%" "%SHARE%\uefismbios.efi" >nul

if errorlevel 1 (
    echo ERROR: Could not copy the EFI file to %SHARE%.
    pause
    exit /b 1
)

echo EFI:   %EFI%
echo OVMF:  %OVMF%
echo Share: %SHARE%
echo.
echo In the UEFI Shell run:
echo   fs0:
echo   uefismbios.efi
echo.

cd /d "%PROJECT%"

"%QEMU%" ^
    -machine q35 ^
    -m 512M ^
    -bios "%OVMF%" ^
    -drive file=fat:rw:%SHARE%,format=raw ^
    -debugcon file:qemu-ovmf-debug.log ^
    -global isa-debugcon.iobase=0x402 ^
    -net none

if errorlevel 1 (
    echo QEMU exited with an error.
    pause
)

endlocal