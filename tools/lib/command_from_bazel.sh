#!/bin/bash
# Copyright 2022 Aurora Operations, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eou pipefail

# In case users haven't set up `direnv`.
export PATH="$PATH:./tools/bin"

function wrap_bazel () {
  COMMAND="$1"; shift
  TARGET="$1"; shift

  bazel_opts=(
    "--nohome_rc"
    "$COMMAND"
    "--run_under=cd $PWD &&"
    "$TARGET"
    "--"
  )

  bazel "${bazel_opts[@]}" "$@"
}

function make_command_from_bazel_run () {
  # We used to run the provided build command first, then do a bazel run on the
  # target.  This was done try to minimize bazel output to the console when
  # running a command.  If we allow the bazel output to go to the console as it
  # would normally, it allows the user to see progress and the like.  Keeping
  # this around in case we need to do fancy tricks to reduce bazel output again.
  BUILD_CMD="$1"; shift

  TARGET="$1"; shift

  wrap_bazel run "$TARGET" "$@"
}
