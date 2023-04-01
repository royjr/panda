#!/usr/bin/env sh
set -e

DFU_UTIL="dfu-util"

cd ..
GATEWAY=1 scons -u -j$(nproc)
# cd gateway

# Recovers panda from DFU mode only, use flash.sh after power cycling panda
# printf %b 'from python import Panda\nfor serial in Panda.list(): Panda(serial).reset(enter_bootstub=True); Panda(serial).reset(enter_bootloader=True)' | PYTHONPATH=.. python3
PYTHONPATH=.. python3 -c "from python import Panda; from time import sleep; Panda().reset(enter_bootstub=True); Panda().reset(enter_bootloader=True); sleep(2)" || true


$DFU_UTIL -d 0483:df11 -a 0 -s 0x08004000 -D obj/panda_gateway.bin.signed
$DFU_UTIL -d 0483:df11 -a 0 -s 0x08000000:leave -D obj/bootstub.panda_gateway.bin

