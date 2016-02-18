#!/bin/sh

# Get the latest packages
sudo apt-get update

# Install all deps needed to build Genode and effectively work on its source
sudo apt-get -y -q install build-essential
sudo apt-get -y -q install make
sudo apt-get -y -q install software-properties-common
sudo apt-get -y -q install gcc
sudo apt-get -y -q install g++
sudo apt-get -y -q install gcc-4.9-multilib g++-4.9-multilib
sudo apt-get -y -q install autoconf
sudo apt-get -y -q install autoconf2.64
sudo apt-get -y -q install autogen
sudo apt-get -y -q install tcl
sudo apt-get -y -q install libsdl2-2.0-0 libsdl2-dev
sudo apt-get -y -q install git subversion curl wget gperf
sudo apt-get -y -q install libxml2 libxml2-dev libxml2-utils
sudo apt-get -y -q install xsltproc
sudo apt-get -y -q install yasm iasl unzip
sudo apt-get -y -q install tree
sudo apt-get -y -q install expect
sudo apt-get -y -q install flex bison byacc

# Get the Genode toolchain setup
mkdir scratch
cd scratch
wget --progress=bar:force --output-document genode-toolchain-15.05-x86_64.tar.bz2 http://downloads.sourceforge.net/project/genode/genode-toolchain/15.05/genode-toolchain-15.05-x86_64.tar.bz2?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fgenode%2Ffiles%2Fgenode-toolchain%2F15.05%2F&ts=1455810960&use_mirror=iweb
sudo tar xPfj genode-toolchain-15.05-x86_64.tar.bz2

# Get N. Feske's Genode patchset, which will give use the day-to-day setup base
git clone https://github.com/ohpauleez/genode.git genode-ohpauleez
cd genode-ohpauleez
git checkout turmvilla
cd repos/base-nova
make prepare PKG=x86emu
cd ../dde_linux
make prepare
cd ../dde_rump
make prepare
cd ../libports
make prepare
cd ../ports
make prepare
cd ../..
./tool/create_builddir nova_x86_64 BUILD_DIR=build.nova64

##  Continuing the build yourself
## -------------------------------------
## You'll now need to edit your build.nova64 to turn on all REPOSITORY settings
## you want.
## vagrant ssh
## echo "You may need to run the 'tar' command for the toolchain"
## echo "You may also have to patch up and prepare libports/ports again; Subversion URLs must be HTTPS in here"
## cd scratch/genode-ohpauleez
## vi build.nova64/etc/build.conf
##
## Uncomment all repos from libports to gems; :wq
## Then from scratch/genode-ohpauleez/build.nova64 ; make run/vbox_linux

