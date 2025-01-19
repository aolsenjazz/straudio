@echo off

REM - CALL "$(SolutionDir)scripts\postbuild-win.bat" "$(TargetExt)" "$(BINARY_NAME)" "$(Platform)" "$(TargetPath)" "$(VST3_32_PATH)" "$(VST3_64_PATH)" "$(AAX_32_PATH)" "$(AAX_64_PATH)" "$(CLAP_PATH)" "$(BUILD_DIR)" "$(VST_ICON)" "$(AAX_ICON)" "$(CREATE_BUNDLE_SCRIPT)" "$(ICUDAT_PATH)"

set FORMAT=%1
set NAME=%2
set PLATFORM=%3
set BUILT_BINARY=%4
set VST3_32_PATH=%5
set VST3_64_PATH=%6
shift
shift
shift
shift
shift
shift
set AAX_32_PATH=%1
set AAX_64_PATH=%2
set CLAP_PATH=%3
set BUILD_DIR=%4
set VST_ICON=%5
set AAX_ICON=%6
set CREATE_BUNDLE_SCRIPT=%7
set ICUDAT_PATH=%8

echo POSTBUILD SCRIPT VARIABLES -----------------------------------------------------
echo FORMAT %FORMAT% 
echo NAME %NAME% 
echo PLATFORM %PLATFORM% 
echo BUILT_BINARY %BUILT_BINARY% 
echo VST3_32_PATH %VST3_32_PATH% 
echo VST3_64_PATH %VST3_64_PATH% 
echo CLAP_PATH %CLAP_PATH% 
echo BUILD_DIR %BUILD_DIR%
echo VST_ICON %VST_ICON% 
echo AAX_ICON %AAX_ICON% 
echo CREATE_BUNDLE_SCRIPT %CREATE_BUNDLE_SCRIPT%
echo ICUDAT_PATH %ICUDAT_PATH%
echo END POSTBUILD SCRIPT VARIABLES -----------------------------------------------------

if %PLATFORM% == "Win32" (
  if exist "%ICUDAT_PATH%" (
    echo copying icudtl.dat file next to built binary: %BUILT_BINARY%
    for %%F in (%BUILT_BINARY%) do (
      copy /y %ICUDAT_PATH% "%%~dpF"
    )
  ) else (
    echo icudtl.dat not found at %ICUDAT_PATH%, skipping...
  )

  if %FORMAT% == ".exe" (
    echo copying exe to build dir: %BUILD_DIR%\%NAME%_%PLATFORM%.exe
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%_%PLATFORM%.exe
    if exist "%ICUDAT_PATH%" (
      echo copying dat file to build dir: %BUILD_DIR%
      copy /y %ICUDAT_PATH% %BUILD_DIR%
    )
  )

  if %FORMAT% == ".dll" (
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%_%PLATFORM%.dll
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %BUILD_DIR%
    )
  )
  
  if %FORMAT% == ".vst3" (
    echo copying 32bit binary to VST3 BUNDLE ..
    call %CREATE_BUNDLE_SCRIPT% %BUILD_DIR%\%NAME%.vst3 %VST_ICON% %FORMAT%
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%.vst3\Contents\x86-win
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %BUILD_DIR%\%NAME%.vst3\Contents\x86-win
    )
    if exist %VST3_32_PATH% ( 
      echo copying VST3 bundle to 32bit VST3 Plugins folder ...
      call %CREATE_BUNDLE_SCRIPT% %VST3_32_PATH%\%NAME%.vst3 %VST_ICON% %FORMAT%
      xcopy /E /H /Y %BUILD_DIR%\%NAME%.vst3\Contents\*  %VST3_32_PATH%\%NAME%.vst3\Contents\
    )
  )
  
  if %FORMAT% == ".aaxplugin" (
    echo copying 32bit binary to AAX BUNDLE ..
    call %CREATE_BUNDLE_SCRIPT% %BUILD_DIR%\%NAME%.aaxplugin %AAX_ICON% %FORMAT%
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%.aaxplugin\Contents\Win32
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %BUILD_DIR%\%NAME%.aaxplugin\Contents\Win32
    )
    echo copying 32bit bundle to 32bit AAX Plugins folder ... 
    call %CREATE_BUNDLE_SCRIPT% %BUILD_DIR%\%NAME%.aaxplugin %AAX_ICON% %FORMAT%
    xcopy /E /H /Y %BUILD_DIR%\%NAME%.aaxplugin\Contents\* %AAX_32_PATH%\%NAME%.aaxplugin\Contents\
  )
)

if %PLATFORM% == "x64" (
  if exist "%ICUDAT_PATH%" (
    echo copying icudtl.dat file next to built binary: %BUILT_BINARY%
    for %%F in (%BUILT_BINARY%) do (
      copy /y %ICUDAT_PATH% "%%~dpF"
    )
  ) else (
    echo icudtl.dat not found at %ICUDAT_PATH%, skipping...
  )

  if not exist "%ProgramFiles(x86)%" (
    echo "This batch script fails on 32 bit windows... edit accordingly"
  )

  if %FORMAT% == ".exe" (
    echo copying exe to build dir: %BUILD_DIR%\%NAME%_%PLATFORM%.exe
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%_%PLATFORM%.exe
    if exist "%ICUDAT_PATH%" (
      echo copying dat file to build dir: %BUILD_DIR%
      copy /y %ICUDAT_PATH% %BUILD_DIR%
    )
  )

  if %FORMAT% == ".dll" (
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%_%PLATFORM%.dll
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %BUILD_DIR%
    )
  )
  
  if %FORMAT% == ".vst3" (
    echo copying 64bit binary to VST3 BUNDLE ...
    call %CREATE_BUNDLE_SCRIPT% %BUILD_DIR%\%NAME%.vst3 %VST_ICON% %FORMAT%
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%.vst3\Contents\x86_64-win
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %BUILD_DIR%\%NAME%.vst3\Contents\x86_64-win
    )
    if exist %VST3_64_PATH% (
      echo copying VST3 bundle to 64bit VST3 Plugins folder ...
      call %CREATE_BUNDLE_SCRIPT% %VST3_64_PATH%\%NAME%.vst3 %VST_ICON% %FORMAT%
      xcopy /E /H /Y %BUILD_DIR%\%NAME%.vst3\Contents\*  %VST3_64_PATH%\%NAME%.vst3\Contents\
    )
  )
  
  if %FORMAT% == ".aaxplugin" (
    echo copying 64bit binary to AAX BUNDLE ...
    call %CREATE_BUNDLE_SCRIPT% %BUILD_DIR%\%NAME%.aaxplugin %AAX_ICON% %FORMAT%
    copy /y %BUILT_BINARY% %BUILD_DIR%\%NAME%.aaxplugin\Contents\x64
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %BUILD_DIR%\%NAME%.aaxplugin\Contents\x64
    )
    echo copying 64bit bundle to 64bit AAX Plugins folder ... 
    call %CREATE_BUNDLE_SCRIPT% %BUILD_DIR%\%NAME%.aaxplugin %AAX_ICON% %FORMAT%
    xcopy /E /H /Y %BUILD_DIR%\%NAME%.aaxplugin\Contents\* %AAX_64_PATH%\%NAME%.aaxplugin\Contents\
  )
  
  if %FORMAT% == ".clap" (
    echo copying binary to CLAP Plugins folder ... 
    copy /y %BUILT_BINARY% %CLAP_PATH%
    if exist "%ICUDAT_PATH%" (
      copy /y %ICUDAT_PATH% %CLAP_PATH%
    )
  )
)
