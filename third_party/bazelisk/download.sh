#!/bin/bash
# Copyright 2022 Aurora Operations, Inc.

set -eux

BAZELISK_V="1.16.0"
BAZELISK_TARBALL="v${BAZELISK_V}.tar.gz"
BAZELISK_DIR="bazelisk-${BAZELISK_V}"

mkdir -p tmp
pushd tmp
curl -LO "https://github.com/bazelbuild/bazelisk/archive/${BAZELISK_TARBALL}"
tar xf "${BAZELISK_TARBALL}"
popd

cp "tmp/${BAZELISK_DIR}"/LICENSE .
cp "tmp/${BAZELISK_DIR}"/bazelisk.py .

rm -rf tmp
