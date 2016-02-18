#!/bin/sh

# Get the latest packages
sudo apk update

# Install all deps needed to build Genode and effectively work on its source
sudo apk add coreutils
sudo apk add build-base
sudo apk add make
sudo apk add autoconf
sudo apk add byacc
sudo apk add flex
sudo apk add tcl
sudo apk add tree
sudo apk add expect
sudo apk add gcc
sudo apk add sdl
sudo apk add git
sudo apk add curl
sudo apk add wget
sudo apk add libxml2

# Get GNU tar
mkdir /home/vagrant/scratch
cd /home/vagrant/scratch
wget http://ftp.gnu.org/gnu/tar/tar-1.28.tar.gz
tar xvzf tar-1.28.tar.gz
cd tar-1.28
./configure
make
sudo make install
cd ..

# Get the Genode toolchain setup
wget --quiet --output-document genode-toolchain-15.05-x86_64.tar.bz2 http://downloads.sourceforge.net/project/genode/genode-toolchain/15.05/genode-toolchain-15.05-x86_64.tar.bz2?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fgenode%2Ffiles%2Fgenode-toolchain%2F15.05%2F&ts=1455810960&use_mirror=iweb
sudo tar xPfj genode-toolchain-15.05-x86_64.tar.bz2

# Get N. Feske's Genode patchset, which will give use the day-to-day setup base
git clone https://github.com/nfeske/genode.git genode-nfeske
cd genode-nfeske
git checkout turmvilla
cd repos/base-nova
make prepare PKG=x86emu
cd ../..
./tool/create_builddir nova_x86_64 BUILD_DIR=build.nova64

##  Continuing the build yourself
## -------------------------------------
## You'll now need to edit your build.nova64 to turn on all REPOSITORY settings
## you want.  The more the merrier in my opinion :)
## vagrant ssh
## vi scratch/genode-nfeske/build.nova64/etc/build.conf
##
## Uncomment all repos from libports to gems; :wq
## Then from scratch/genode-nfeske/build.nova64 ; make run/vbox_linux

