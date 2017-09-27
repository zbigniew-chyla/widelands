set -ex

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  #Install requested compiler version for linux

  if [ "$CXX" = "g++" ]; then
    sudo apt-get install -qq g++-$GCC_VERSION;
    export CXX="g++-$GCC_VERSION" CC="gcc-$GCC_VERSION";
  fi
  if [ "$CXX" = "clang++" ]; then
    sudo apt-get install -qq clang-$CLANG_VERSION;
    export CXX="clang++-$CLANG_VERSION" CC="clang-$CLANG_VERSION";
  fi
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  # Install osx dependencies
  # boost and cmake are preinstalled :)
  brew install gettext glew icu4c sdl2 sdl2_image sdl2_mixer sdl2_ttf zlib
  # brew doesn't add a link by default
  brew link --force gettext
  brew link --force icu4c
fi
#Build for Coverity by commands in .travis.yml
