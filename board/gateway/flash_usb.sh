#!/usr/bin/env sh
set -e
cd ..
scons -u -j$(nproc)

printf %b 'from python import Panda\nfor serial in Panda.list(): Panda(serial).flash(fn="'$realpath obj/panda_gateway.bin.signed'")' | PYTHONPATH=.. python3
