#! /bin/sh

# this script requires xcpretty https://github.com/xcpretty/xcpretty

BASEDIR=$(dirname $0)

cd $BASEDIR/..

if [ -d build-mac ]; then
  sudo rm -f -R build-mac
fi

#---------------------------------------------------------------------------------------------------------
#variables

IPLUG2_ROOT=../iPlug2
XCCONFIG=$IPLUG2_ROOT/../common-mac.xcconfig
SCRIPTS=$IPLUG2_ROOT/Scripts

# CODESIGN disabled by default. 
CODESIGN=1

# macOS codesigning/notarization
NOTARIZE_BUNDLE_ID=com.AOFX.Straudio
NOTARIZE_BUNDLE_ID_DEMO=com.AOFX.Straudio.DEMO
APP_SPECIFIC_ID=TODO
APP_SPECIFIC_PWD=TODO

# AAX/PACE wraptool codesigning
ILOK_ID=TODO
ILOK_PWD=TODO
WRAP_GUID=TODO

# Check for required environment variables
if [ -z "$APPLE_EMAIL" ] || [ -z "$APPLE_PASSWORD" ] || [ -z "$APPLE_TEAM_ID" ]; then
  echo "ERROR: Required environment variables are missing."
  echo "Please set APPLE_EMAIL, APPLE_PASSWORD, and APPLE_TEAM_ID before running this script."
  exit 1
fi

# Notify the user about the variables being used
echo "Using the following credentials for notarization:"
echo "Email: $APPLE_EMAIL"
echo "Team ID: $APPLE_TEAM_ID"

DEMO=0
if [ "$1" == "demo" ]; then
  DEMO=1
fi

BUILD_INSTALLER=1
if [ "$2" == "zip" ]; then
  BUILD_INSTALLER=0
fi

VERSION=`echo | grep PLUG_VERSION_HEX config.h`
VERSION=${VERSION//\#define PLUG_VERSION_HEX }
VERSION=${VERSION//\'}
MAJOR_VERSION=$(($VERSION & 0xFFFF0000))
MAJOR_VERSION=$(($MAJOR_VERSION >> 16))
MINOR_VERSION=$(($VERSION & 0x0000FF00))
MINOR_VERSION=$(($MINOR_VERSION >> 8))
BUG_FIX=$(($VERSION & 0x000000FF))

FULL_VERSION=$MAJOR_VERSION"."$MINOR_VERSION"."$BUG_FIX

PLUGIN_NAME=`echo | grep BUNDLE_NAME config.h`
PLUGIN_NAME=${PLUGIN_NAME//\#define BUNDLE_NAME }
PLUGIN_NAME=${PLUGIN_NAME//\"}

ARCHIVE_NAME=$PLUGIN_NAME-mac

if [ $DEMO == 1 ]; then
  ARCHIVE_NAME=$ARCHIVE_NAME-demo
fi

# TODO: use get_archive_name script
# if [ $DEMO == 1 ]; then
#   ARCHIVE_NAME=`python3 ${SCRIPTS}/get_archive_name.py ${PLUGIN_NAME} mac demo`
# else
#   ARCHIVE_NAME=`python3 ${SCRIPTS}/get_archive_name.py ${PLUGIN_NAME} mac full`
# fi

VST3=`echo | grep VST3_PATH $XCCONFIG`
VST3=$HOME${VST3//\VST3_PATH = \$(HOME)}/$PLUGIN_NAME.vst3

AU=`echo | grep AU_PATH $XCCONFIG`
AU=$HOME${AU//\AU_PATH = \$(HOME)}/$PLUGIN_NAME.component

APP=`echo | grep APP_PATH $XCCONFIG`
APP=$HOME${APP//\APP_PATH = \$(HOME)}/$PLUGIN_NAME.app

# Dev build folder
AAX=`echo | grep AAX_PATH $XCCONFIG`
AAX=${AAX//\AAX_PATH = }/$PLUGIN_NAME.aaxplugin
AAX_FINAL="/Library/Application Support/Avid/Audio/Plug-Ins/$PLUGIN_NAME.aaxplugin"

PKG="build-mac/installer/$PLUGIN_NAME Installer.pkg"
PKG_US="build-mac/installer/$PLUGIN_NAME Installer.unsigned.pkg"

CERT_ID=`echo | grep CERTIFICATE_ID $XCCONFIG`
CERT_ID=${CERT_ID//\CERTIFICATE_ID = }
DEV_ID_APP_STR="Developer ID Application: ${CERT_ID}"
DEV_ID_INST_STR="Developer ID Installer: ${CERT_ID}"

echo $VST3
echo $AU
echo $APP
echo $AAX

if [ $DEMO == 1 ]; then
 echo "making $PLUGIN_NAME version $FULL_VERSION DEMO mac distribution..."
#   cp "resources/img/AboutBox_Demo.png" "resources/img/AboutBox.png"
else
 echo "making $PLUGIN_NAME version $FULL_VERSION mac distribution..."
#   cp "resources/img/AboutBox_Registered.png" "resources/img/AboutBox.png"
fi

sleep 2

echo "touching source to force recompile"
echo ""
touch *.cpp

#---------------------------------------------------------------------------------------------------------
#remove existing binaries

echo "remove existing binaries"
echo ""

if [ -d $APP ]; then
  sudo rm -f -R -f $APP
fi

if [ -d $AU ]; then
 sudo rm -f -R $AU
fi

if [ -d $VST3 ]; then
  sudo rm -f -R $VST3
fi

if [ -d "${AAX}" ]; then
  sudo rm -f -R "${AAX}"
fi

if [ -d "${AAX_FINAL}" ]; then
  sudo rm -f -R "${AAX_FINAL}"
fi

#---------------------------------------------------------------------------------------------------------
# build xcode project. Change target to build individual formats, or add to All target in the xcode project

xcodebuild -project ./projects/$PLUGIN_NAME-macOS.xcodeproj -xcconfig ./config/$PLUGIN_NAME-mac.xcconfig DEMO_VERSION=$DEMO -target "All" -jobs=1 -UseModernBuildSystem=YES -configuration Release | tee build-mac.log | xcpretty #&& exit ${PIPESTATUS[0]}

if [ "${PIPESTATUS[0]}" -ne "0" ]; then
  echo "ERROR: build failed, aborting"
  echo ""
  # cat build-mac.log
  exit 1
else
  rm build-mac.log
fi

#---------------------------------------------------------------------------------------------------------
# set bundle icons - http://www.hamsoftengineering.com/codeSharing/SetFileIcon/SetFileIcon.html

echo "setting icons"
echo ""

if [ -d $AU ]; then
  ./$SCRIPTS/SetFileIcon -image resources/$PLUGIN_NAME.icns -file $AU
fi

if [ -d $VST3 ]; then
  ./$SCRIPTS/SetFileIcon -image resources/$PLUGIN_NAME.icns -file $VST3
fi

if [ -d "${AAX}" ]; then
  ./$SCRIPTS/SetFileIcon -image resources/$PLUGIN_NAME.icns -file "${AAX}"
fi

#---------------------------------------------------------------------------------------------------------
#strip symbols from binaries

echo "stripping binaries"
echo ""

if [ -d $APP ]; then
  strip -x $APP/Contents/MacOS/$PLUGIN_NAME
fi

if [ -d $AU ]; then
  strip -x $AU/Contents/MacOS/$PLUGIN_NAME
fi

if [ -d $VST3 ]; then
  strip -x $VST3/Contents/MacOS/$PLUGIN_NAME
fi

if [ -d "${AAX}" ]; then
  strip -x "${AAX}/Contents/MacOS/$PLUGIN_NAME"
fi

if [ $CODESIGN == 1 ]; then
  #---------------------------------------------------------------------------------------------------------
  # code sign AAX binary with wraptool

  # echo "copying AAX ${PLUGIN_NAME} from 3PDev to main AAX folder"
  # sudo cp -p -R "${AAX}" "${AAX_FINAL}"
  # mkdir "${AAX_FINAL}/Contents/Factory Presets/"
  
  # echo "code sign AAX binary"
  # /Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool sign --verbose --account $ILOK_ID --password $ILOK_PWD --wcguid $WRAP_GUID --signid "${DEV_ID_APP_STR}" --in "${AAX_FINAL}" --out "${AAX_FINAL}"

  #---------------------------------------------------------------------------------------------------------

  #---------------------------------------------------------------------------------------------------------
  echo "code-sign binaries"
  echo ""

  codesign --force -s "${DEV_ID_APP_STR}" -v $APP --deep --strict --options=runtime #hardened runtime for app
  xattr -cr $AU 
  codesign --force -s "${DEV_ID_APP_STR}" -v $AU --deep --strict
  
  xattr -cr $VST3
  codesign --force -s "${DEV_ID_APP_STR}" -v $VST3 --deep --strict
  #---------------------------------------------------------------------------------------------------------
fi

if [ $BUILD_INSTALLER == 1 ]; then
  #---------------------------------------------------------------------------------------------------------
  # installer

  sudo rm -R -f build-mac/$PLUGIN_NAME-*.dmg

  echo "building installer"
  echo ""

  ./scripts/makeinstaller-mac.sh $FULL_VERSION

  if [ $CODESIGN == 1 ]; then
    echo "code-sign installer for Gatekeeper on macOS 10.8+"
    echo ""
    mv "${PKG}" "${PKG_US}"
    productsign --sign "${DEV_ID_INST_STR}" "${PKG_US}" "${PKG}"
    rm -R -f "${PKG_US}"
  fi

  #set installer icon
  ./$SCRIPTS/SetFileIcon -image resources/$PLUGIN_NAME.icns -file "${PKG}"

  #---------------------------------------------------------------------------------------------------------
  # make dmg, can use dmgcanvas http://www.araelium.com/dmgcanvas/ to make a nice dmg, fallback to hdiutil
  echo "building dmg"
  echo ""

  if [ -d installer/$PLUGIN_NAME.dmgCanvas ]; then
    dmgcanvas installer/$PLUGIN_NAME.dmgCanvas build-mac/$ARCHIVE_NAME.dmg
  else
    cp installer/changelog.txt build-mac/installer/
    cp installer/known-issues.txt build-mac/installer/
    cp "manual/$PLUGIN_NAME manual.pdf" build-mac/installer/
    hdiutil create build-mac/$ARCHIVE_NAME.dmg -format UDZO -srcfolder build-mac/installer/ -ov -anyowners -volname $PLUGIN_NAME
  fi

  sudo rm -R -f build-mac/installer/

  if [ $CODESIGN == 1 ]; then
    #---------------------------------------------------------------------------------------------------------
    #notarize dmg
    echo "notarizing"
    echo ""
    # you need to create an app-specific id/password https://support.apple.com/en-us/HT204397
    # arg 1 Set to the dmg path
    # arg 2 Set to a bundle ID (doesn't have to match your )
    # arg 3 Set to the app specific Apple ID username/email
    # arg 4 Set to the app specific Apple password  
    PWD=`pwd`
    
    # ALEX IF YOU FIND THIS I don't think I'm worried about demo versions right now.
    # Only updating the notarize script for non-demos
    if [ $DEMO == 1 ]; then
      ./$SCRIPTS/notarise.sh "${PWD}/build-mac" "${PWD}/build-mac/${ARCHIVE_NAME}.dmg" $NOTARIZE_BUNDLE_ID $APP_SPECIFIC_ID $APP_SPECIFIC_PWD
    else
      # Notarize the .dmg using xcrun notarytool
      xcrun notarytool submit "${PWD}/build-mac/${ARCHIVE_NAME}.dmg" \
        --apple-id "$APPLE_EMAIL" \
        --password "$APPLE_PASSWORD" \
        --team-id "$APPLE_TEAM_ID" \
        --wait
    fi

    if [ "${PIPESTATUS[0]}" -ne "0" ]; then
      echo "ERROR: notarize script failed, aborting"
      exit 1
    fi

  fi
else
  #---------------------------------------------------------------------------------------------------------
  # zip

  if [ -d build-mac/zip ]; then
    rm -R build-mac/zip
  fi

  mkdir -p build-mac/zip

  if [ -d $APP ]; then
    cp -R $APP build-mac/zip/$PLUGIN_NAME.app
  fi

  if [ -d $AU ]; then
    cp -R $AU build-mac/zip/$PLUGIN_NAME.component
  fi

  if [ -d $VST3 ]; then
    cp -R $VST3 build-mac/zip/$PLUGIN_NAME.vst3
  fi

  if [ -d "${AAX_FINAL}" ]; then
    cp -R $AAX_FINAL build-mac/zip/$PLUGIN_NAME.aaxplugin
  fi

  echo "zipping binaries..."
  echo ""
  ditto -c -k build-mac/zip build-mac/$ARCHIVE_NAME.zip
  rm -R build-mac/zip
fi

#---------------------------------------------------------------------------------------------------------
# dSYMs
sudo rm -R -f build-mac/*-dSYMs.zip

echo "packaging dSYMs"
echo ""
zip -r ./build-mac/$ARCHIVE_NAME-dSYMs.zip ./build-mac/*.dSYM

#---------------------------------------------------------------------------------------------------------

#if [ $DEMO == 1 ]
#then
#  git checkout installer/Straudio.iss
#  git checkout installer/Straudio.pkgproj
#  git checkout resources/img/AboutBox.png
#fi

echo "done!"
echo ""
