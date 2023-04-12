#!/bin/bash

cd $HOME/github/osilis/kernel
source $HOME/osbook/devenv/buildenv.sh
make

cd $HOME/edk2
source edksetup.sh
build
$HOME/osbook/devenv/run_qemu.sh Build/OsilisLoaderX64/DEBUG_CLANG38/X64/Loader.efi $HOME/github/osilis/kernel/kernel.elf
