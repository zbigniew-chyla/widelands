set -ex

# Some of these commands fail transiently. We keep retrying them until they
# succeed.
until sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y; do sleep 10; done
until sudo add-apt-repository ppa:zoogie/sdl2-snapshots -y; do sleep 10; done
until sudo apt-get update -qq --force-yes -y; do sleep 10; done
until sudo apt-get install -qq --force-yes -y g++-5; do sleep 10; done
export CXX="g++-5" CC="gcc-5";

until sudo apt-get install -qq --force-yes -y \
   cmake \
   libboost-all-dev \
   libglew-dev \
   libicu-dev \
   libpng-dev \
   libxml2-dev \
   libyajl-dev \
   zlib1g-dev \
   libsdl2-dev \
   libsdl2-image-dev \
   libsdl2-mixer-dev \
   libsdl2-net-dev \
   libsdl2-ttf-dev \
; do sleep 10; done
