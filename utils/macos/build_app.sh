#!/bin/bash

set -e

USAGE="Usage: $0 <clang|gcc> <debug|release> <bzr_repo_directory>"
USE_ASAN="OFF"

if [ -z "$3" ]; then
   echo $USAGE
   exit 1
else
   SOURCE_DIR=$3
   case "$2" in
   debug|Debug)
      TYPE="Debug"
      if [ "$1" == "clang" ]; then
         USE_ASAN="ON"
         # Necessary to avoid linking errors later on
         ASANLIB=$(echo "int main(void){return 0;}" | xcrun clang -fsanitize=address \
         -xc -o/dev/null -v - 2>&1 |   tr ' ' '\n' | grep libclang_rt.asan_osx_dynamic.dylib)
         mkdir -p "@rpath"
         ln -fs "$ASANLIB" "@rpath/"
      fi
      ;;
   release|Release)
      TYPE="Release"
      ;;
   *)
      echo $USAGE
      exit 1
      ;;
   esac
   case "$1" in
   clang)
      C_COMPILER="clang"
      CXX_COMPILER="clang++"
      COMPILER=$(clang --version | grep "clang")
      ;;
   gcc)
      C_COMPILER="gcc-7"
      CXX_COMPILER="g++-7"
      COMPILER=$(gcc-7 --version | grep "GCC")
      ;;
   *)
      echo $USAGE
      exit 1
      ;;
   esac
fi

# Check if the SDK for the minimum build target is available.
# If not, use the one for the installed macOS Version
OSX_MIN_VERSION="10.7"
SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_MIN_VERSION.sdk"
if [ ! -d "$SDK_DIRECTORY" ]; then
   OSX_VERSION=$(sw_vers -productVersion | cut -d . -f 1,2)
   OSX_MIN_VERSION=$OSX_VERSION
   SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_VERSION.sdk"
fi

REVISION=`bzr revno $SOURCE_DIR`
DESTINATION="WidelandsRelease"

if [[ -f $SOURCE_DIR/WL_RELEASE ]]; then
   WLVERSION="$(cat $SOURCE_DIR/WL_RELEASE)"
else
   WLVERSION="r$REVISION"
fi

echo ""
echo "   Source:      $SOURCE_DIR"
echo "   Version:     $WLVERSION"
echo "   Destination: $DESTINATION"
echo "   Type:        $TYPE"
echo "   macOS:       $OSX_MIN_VERSION"
echo "   Compiler:    $COMPILER"
echo ""

function MakeDMG {
   # Sanity check: Make sure Widelands is there.
   test -f $DESTINATION/Widelands.app/Contents/MacOS/widelands

   find $DESTINATION -name ".?*" -exec rm -v {} \;
   UP=$(dirname $DESTINATION)

   echo "Copying COPYING"
   cp $SOURCE_DIR/COPYING  $DESTINATION/COPYING.txt

   echo "Creating DMG ..."
   if [ "$TYPE" == "Release" ]; then
      hdiutil create -fs HFS+ -volname "Widelands $WLVERSION" -srcfolder "$DESTINATION" "$UP/widelands_64bit_$WLVERSION.dmg"
   elif [ "$TYPE" == "Debug" ]; then
      hdiutil create -fs HFS+ -volname "Widelands $WLVERSION" -srcfolder "$DESTINATION" "$UP/widelands_64bit_${WLVERSION}_${TYPE}.dmg"
   fi
}

function CopyLibrary {
   path=$1; shift
   cp $path "$DESTINATION/Widelands.app/Contents/MacOS/"
   chmod 644 "$DESTINATION/Widelands.app/Contents/MacOS/$(basename ${path})"
}

function MakeAppPackage {
   echo "Making $DESTINATION/Widelands.app now."
   rm -Rf $DESTINATION/

   mkdir $DESTINATION/
   mkdir $DESTINATION/Widelands.app/
   mkdir $DESTINATION/Widelands.app/Contents/
   mkdir $DESTINATION/Widelands.app/Contents/Resources/
   mkdir $DESTINATION/Widelands.app/Contents/MacOS/
   cp $SOURCE_DIR/data/images/logos/widelands.icns $DESTINATION/Widelands.app/Contents/Resources/widelands.icns
   ln -s /Applications $DESTINATION/Applications

   cat > $DESTINATION/Widelands.app/Contents/Info.plist << EOF
{
   CFBundleName = widelands;
   CFBundleDisplayName = Widelands;
   CFBundleIdentifier = "org.widelands.wl";
   CFBundleVersion = "$WLVERSION";
   CFBundleShortVersionString = "$WLVERSION";
   CFBundleInfoDictionaryVersion = "6.0";
   CFBundlePackageType = APPL;
   CFBundleSignature = wdld;
   CFBundleExecutable = widelands;
   CFBundleIconFile = "widelands.icns";
}
EOF

   echo "Copying data files ..."
   rsync -Ca $SOURCE_DIR/data $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Copying locales ..."
   rsync -Ca locale $DESTINATION/Widelands.app/Contents/MacOS/data/

   echo "Copying binary ..."
   cp -a src/widelands $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Stripping binary ..."
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands

   echo "Copying dynamic libraries for SDL_image ... "
   CopyLibrary "$(brew --prefix libpng)/lib/libpng.dylib"
   CopyLibrary "$(brew --prefix jpeg)/lib/libjpeg.dylib"

   echo "Copying dynamic libraries for SDL_mixer ... "
   CopyLibrary $(brew --prefix libogg)/lib/libogg.dylib
   CopyLibrary $(brew --prefix libvorbis)/lib/libvorbis.dylib
   CopyLibrary $(brew --prefix libvorbis)/lib/libvorbisfile.dylib

   $SOURCE_DIR/utils/macos/fix_dependencies.py \
   $DESTINATION/Widelands.app/Contents/MacOS/widelands \
   $DESTINATION/Widelands.app/Contents/MacOS/*.dylib
}

function BuildWidelands() {
   PREFIX_PATH="$(brew --prefix libpng)"
   PREFIX_PATH+=";$(brew --prefix jpeg)"
   PREFIX_PATH+=";$(brew --prefix libpng)"
   PREFIX_PATH+=";$(brew --prefix python)"
   PREFIX_PATH+=";$(brew --prefix zlib)"
   PREFIX_PATH+=";/usr/local"
   PREFIX_PATH+=";/usr/local/Homebrew"

   export SDL2DIR="$(brew --prefix sdl2)"
   export SDL2IMAGEDIR="$(brew --prefix sdl2_image)"
   export SDL2MIXERDIR="$(brew --prefix sdl2_mixer)"
   export SDL2TTFDIR="$(brew --prefix sdl2_ttf)"
   export BOOST_ROOT="$(brew --prefix boost)"
   
   # Not needed for CMake 3.12 or above
   # see cmake --help-policy CMP0074
   #export ICU_ROOT="$(brew --prefix icu4c)"

   cmake $SOURCE_DIR -G Ninja \
      -DCMAKE_C_COMPILER:FILEPATH="$(brew --prefix ccache)/libexec/$C_COMPILER" \
      -DCMAKE_CXX_COMPILER:FILEPATH="$(brew --prefix ccache)/libexec/$CXX_COMPILER" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="$OSX_MIN_VERSION" \
      -DCMAKE_OSX_SYSROOT:PATH="$SDK_DIRECTORY" \
      -DCMAKE_INSTALL_PREFIX:PATH="$DESTINATION/Widelands.app/Contents/MacOS" \
      -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64" \
      -DCMAKE_BUILD_TYPE:STRING="$TYPE" \
      -DGLEW_INCLUDE_DIR:PATH="$(brew --prefix glew)/include" \
      -DGLEW_LIBRARY:PATH="$(brew --prefix glew)/lib/libGLEW.dylib" \
      -DCMAKE_PREFIX_PATH:PATH="${PREFIX_PATH}" \
      -DOPTION_ASAN="$USE_ASAN"
   ninja

   echo "Done building."
}

BuildWidelands
MakeAppPackage
MakeDMG
