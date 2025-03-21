@echo off

REM - batch file to build MSVS project and zip the resulting binaries
REM - updating version numbers requires python and python path added to %PATH% env variable 
REM - zipping requires 7zip in %ProgramFiles%\7-Zip\7z.exe
REM - building installer requires innotsetup in "%ProgramFiles(x86)%\Inno Setup 5\iscc"
REM - AAX codesigning requires wraptool tool added to %PATH% env variable and aax.key/.crt in .\..\..\..\Certificates\

if %1 == 1 (echo Making Straudio Windows DEMO VERSION distribution ...) else (echo Making Straudio Windows FULL VERSION distribution ...)

echo "touching source"

copy /b ..\*.cpp+,,

echo ------------------------------------------------------------------
echo Updating version numbers ...

call python prepare_resources-win.py %1
call python update_installer_version.py %1

cd ..\

echo "Restoring dependencies in projects/packages.config..."
msbuild -p:RestorePackagesConfig=true -t:restore

echo ------------------------------------------------------------------
echo Building ...

if exist "%ProgramFiles(x86)%" (goto 64-Bit) else (goto 32-Bit)

if not defined DevEnvDir (
:32-Bit
echo 32-Bit O/S detected
call "%ProgramFiles%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
goto END

:64-Bit
echo 64-Bit Host O/S detected
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_x64
goto END
:END
)


REM - set preprocessor macros like this, for instance to enable demo build:
if %1 == 1 (
set CMDLINE_DEFINES="DEMO_VERSION=1"
REM -copy ".\resources\img\AboutBox_Demo.png" ".\resources\img\AboutBox.png"
) else (
set CMDLINE_DEFINES="DEMO_VERSION=0"
REM -copy ".\resources\img\AboutBox_Registered.png" ".\resources\img\AboutBox.png"
)

REM - Could build individual targets like this:
REM - msbuild Straudio-app.vcxproj /p:configuration=release /p:platform=win32

echo Building 32 bit binaries...
msbuild Straudio.sln /p:configuration=release /p:platform=win32 /nologo /verbosity:minimal /fileLogger /m /flp:logfile=build-win.log;errorsonly 

echo Building 64 bit binaries...
msbuild Straudio.sln /p:configuration=release /p:platform=x64 /nologo /verbosity:minimal /fileLogger /m /flp:logfile=build-win.log;errorsonly;append

REM --echo Copying AAX Presets

REM --echo ------------------------------------------------------------------
REM --echo Code sign AAX binary...
REM --info at pace central, login via iLok license manager https://www.paceap.com/pace-central.html
REM --wraptool sign --verbose --account XXXXX --wcguid XXXXX --keyfile XXXXX.p12 --keypassword XXXXX --in .\build-win\aax\bin\Straudio.aaxplugin\Contents\Win32\Straudio.aaxplugin --out .\build-win\aax\bin\Straudio.aaxplugin\Contents\Win32\Straudio.aaxplugin
REM --wraptool sign --verbose --account XXXXX --wcguid XXXXX --keyfile XXXXX.p12 --keypassword XXXXX --in .\build-win\aax\bin\Straudio.aaxplugin\Contents\x64\Straudio.aaxplugin --out .\build-win\aax\bin\Straudio.aaxplugin\Contents\x64\Straudio.aaxplugin

REM - Make Installer (InnoSetup)

echo ------------------------------------------------------------------
echo Making Installer ...

if exist "%ProgramFiles(x86)%" (goto 64-Bit-is) else (goto 32-Bit-is)

:32-Bit-is
"%ProgramFiles%\Inno Setup 6\iscc" /Q ".\installer\Straudio.iss"
goto END-is

:64-Bit-is
"%ProgramFiles(x86)%\Inno Setup 6\iscc" /Q ".\installer\Straudio.iss"
goto END-is

:END-is

REM - Codesign Installer for Windows 8+
REM -"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe" sign /f "XXXXX.p12" /p XXXXX /d "Straudio Installer" ".\installer\Straudio Installer.exe"

REM -if %1 == 1 (
REM -copy ".\installer\Straudio Installer.exe" ".\installer\Straudio Demo Installer.exe"
REM -del ".\installer\Straudio Installer.exe"
REM -)
