#!/bin/bash

set -x
set -e

TP_DIR=$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)

echo "building hiredis"
cd $TP_DIR/hiredis
make
