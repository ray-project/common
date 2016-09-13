#!/usr/bin/env bash

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)

pushd "$ROOT_DIR/thirdparty"
  bash ./build-thirdparty.sh
popd
