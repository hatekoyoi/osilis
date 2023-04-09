#!/bin/bash

cd $HOME/github/osilis/kernel
source $HOME/osbook/devenv/buildenv.sh
clang++ $CPPFLAGS -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 -c main.cpp
ld.lld $LDFLAGS --entry KernelMain -z norelro --image-base 0x100000 --static -z separate-code -o kernel.elf main.o

cd $HOME/edk2
source edksetup.sh
build
$HOME/osbook/devenv/run_qemu.sh Build/OsilisLoaderX64/DEBUG_CLANG38/X64/Loader.efi $HOME/github/osilis/kernel/kernel.elf
