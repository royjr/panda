#!/usr/bin/env sh
set -e

cd ..
GATEWAY=1 scons -u -j$(nproc)
cd gateway

../../tests/gateway/enter_canloader_gateway.py ../obj/panda_gateway.bin.signed
