@echo off
setlocal EnableExtensions

for %%I in ("%~dp0..") do set "PROJECT=%%~fI"

set "EDK2=C:\edk2"
set "QEMU=C:\Program Files\qemu\qemu-system-x86_64.exe"

set "FV=%EDK2%\Build\OvmfX64\DEBUG_VS2026\FV"
set "OVMF=%FV%\OVMF_CODE.fd"
set "VARS=%PROJECT%\build\OVMF_VARS.fd"
set "EFI=%PROJECT%\build\smeditor.efi"
set "SHARE=%PROJECT%\build\esp"

if not exist "%QEMU%" (
    echo ERROR: QEMU not found: %QEMU%
    pause
    exit /b 1
)

"%QEMU%" -machine help 2>&1 | findstr /i /c:"pc-q35-9.0" >nul
if errorlevel 1 (
    echo ERROR: This QEMU build does not provide pc-q35-9.0.
    echo Run this command to list available machine types:
    echo   "%QEMU%" -machine help
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

copy /Y "%EFI%" "%SHARE%\smeditor.efi" >nul

>"%SHARE%\startup.nsh" echo fs0:\smeditor.efi /apply

if not exist "%VARS%" copy /Y "%FV%\OVMF_VARS.fd" "%VARS%" >nul

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
echo   smeditor.efi
echo.

cd /d "%PROJECT%"

set ARGS=-machine pc-q35-9.0 -cpu Nehalem -smp 12,sockets=1,cores=6,threads=2 -m 80G
set ARGS=%ARGS% -drive "if=pflash,format=raw,unit=0,readonly=on,file=%OVMF%"
set ARGS=%ARGS% -drive "if=pflash,format=raw,unit=1,file=%VARS%"
set ARGS=%ARGS% -smbios type=0,vendor="American Megatrends International,, LLC.",version="1811",date="08/15/2024",release=18.11,uefi=on
set ARGS=%ARGS% -smbios type=1,manufacturer="ASUS",product="TUF GAMING B650-PLUS WIFI",version="Rev 1.xx",serial="M80TUF650000001",uuid=7c4a2f91-8b36-4d52-a913-6e8f20b751c4,sku="TUF-B650-PLUS-WIFI",family="TUF Gaming Desktop"
set ARGS=%ARGS% -smbios type=2,manufacturer="ASUSTeK COMPUTER INC.",product="TUF GAMING B650-PLUS WIFI",version="Rev 1.xx",serial="MB-B650-001",asset="ASUS-MB-001",location="Mainboard"
set ARGS=%ARGS% -smbios type=3,manufacturer="Fractal Design",version="Pop Air Black",serial="FD-CASE-001",asset="CASE-001",sku="FD-C-POA1A-01"
set ARGS=%ARGS% -smbios type=4,sock_pfx="AM5",manufacturer="Advanced Micro Devices,, Inc.",version="AMD Ryzen 5 7600 6-Core Processor",serial="AMD-7600-001",asset="CPU-001",part="100-100001015BOX"
set ARGS=%ARGS% -smbios type=17,loc_pfx="DIMM",bank="BANK",manufacturer="Corsair",serial="COR-RAM",asset="RAM",part="CMK32GX5M2B6000C36",speed=6000
set ARGS=%ARGS% -smbios type=11,value="ASUS TUF Gaming",value="AMD Ryzen Gaming System",value="MSI GeForce RTX 4070 SUPER",value="Samsung 990 PRO 1TB",value="Corsair Vengeance DDR5"
set ARGS=%ARGS% -drive "file=fat:rw:%SHARE%,format=raw"
set ARGS=%ARGS% -debugcon file:build\qemu-ovmf-debug.log
set ARGS=%ARGS% -global isa-debugcon.iobase=0x402
set ARGS=%ARGS% -net none

"%QEMU%" %ARGS%

if errorlevel 1 (
    echo QEMU exited with an error.
    pause
)

endlocal