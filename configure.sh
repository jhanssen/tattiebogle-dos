#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cmake -G "Watcom WMake" -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/cmake/wc-toolchain.cmake" "${SCRIPT_DIR}" "$@"
