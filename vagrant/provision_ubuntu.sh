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
sudo apt-get -y -q install gawk pkg-config
sudo apt-get -y -q install tree expect patch
sudo apt-get -y -q install flex bison byacc
sudo apt-get -y -q install genisoimage syslinux syslinux-utils
sudo apt-get -y -q install qemu qemu-system-x86 virtualbox vagrant
sudo apt-get -y -q install virtualbox-dkms virtualbox-qt xserver-xorg xinit

# Get the Genode toolchain setup
mkdir scratch
cd scratch
wget --progress=bar:force --output-document genode-toolchain-17.05-x86_64.tar.bz2 https://downloads.sourceforge.net/project/genode/genode-toolchain/17.05/genode-toolchain-17.05-x86_64.tar.xz?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fgenode%2Ffiles%2Fgenode-toolchain%2F17.05%2F&ts=1511114714&use_mirror=svwh

# Use the day-to-day setup base
git clone https://github.com/ohpauleez/genode.git genode-ohpauleez
cd genode-ohpauleez
git checkout townsend
./tool/ports/prepare_port nova # NOVA microhypervisor - supports VirtualBox
#./tool/ports/prepare_port foc # Fiasco.OC L4 kernel
#./tool/ports/prepare_port sel4 # seL4 kernel
./tool/ports/prepare_port x86emu

./tool/ports/prepare_port dde_linux # Linux devices and kernel
./tool/ports/prepare_port dde_rump # NetBSD RUMP kernel components/devices
./tool/ports/prepare_port dde_ipxe # Open source boot firmware

# OpenBSD DDE has a malformed gzip archive
#./tool/ports/prepare_port dde_bsd # OpenBSD devices and kernel

#TODO I don't know how to do these with the new build system
#./tool/ports/prepare_port
#cd ../libports PKG=x86emu
#make prepare
#cd ../ports
#make prepare
#cd ../..

# Prepare the tools and build dir
sudo tar xPfj ../genode-toolchain-17.05-x86_64.tar.bz2
./tool/create_builddir nova_x86_64 BUILD_DIR=build.nova64

##  Continuing the build yourself
## -------------------------------------
## You'll now need to edit your build.nova64 to turn on all REPOSITORY settings
## you want.
## vagrant ssh
## cd scratch/genode-ohpauleez
## vim build.nova64/etc/build.conf
##
## Uncomment all repos; :wq
## Then from scratch/genode-ohpauleez/build.nova64 ; make run/turmvilla
## The image will be at: build.nova64/var/run/turmvilla.iso
##
## Copy the image out: cp build.nova64/var/run/turmvilla.iso /vagrant
## Logout of the vagrant image and halt it.
##
## Optionally above, you can build the very base image with run/vbox_linux
##  - that image would be at: build.nova64/var/run/vbox_linux.iso

