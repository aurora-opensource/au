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

function wrap_bazel() {
  COMMAND="$1"
  shift

  TARGET="$1"
  shift

  bazel_opts_startup=(
    "--nohome_rc"
    "--quiet"
  )

  bazel_opts=(
    "--ui_event_filters=-info,-stdout,-stderr"
    "--noshow_progress"
    "--run_under=cd $PWD &&"
    "$TARGET"
  )

  bazel "${bazel_opts_startup[@]}" "$COMMAND" "${bazel_opts[@]}" -- "$@"
}

function make_command_from_bazel_run() {
  BUILD_CMD="$1"
  shift

  TARGET="$1"
  shift

  USER_MESSAGE="Building tool.  If curious as to what's taking so long, you can Ctrl-C and run: bazel --nohome_rc $BUILD_CMD $TARGET"

  # Write message, then run _building_ command.
  # When done: back up; then, write spaces; then, back up again.
  echo -n "$USER_MESSAGE" >&2
  wrap_bazel "$BUILD_CMD" "$TARGET"
  echo -n "$USER_MESSAGE" | sed 's/./\x08 \x08/g' >&2

  # Run _real_ command.
  if [ "$BUILD_CMD" == "fetch" ]; then
    # With bzlmod, bazel fetch with a specific file in a module extension
    # doesn't actually pull anything down, and query doesn't actually find it.
    # With current versions of bazel, it looks like bazel build of the target
    # file works, so try that.  Leaving this here for now in case I need to come
    # back for it.
    echo "ERROR: fetch with bzlmod may not work as expected..." >&2
    exit 1

    # A target that is simply fetched is a pre-built target.  Bazel 6.0.0 will complain if we use
    # `bazel run`, because `bazel run` uses `bazel build` under the hood, and there is nothing to
    # build.  So in this case, we simply execute the target directly.
    QUERY_RESULT="$(bazel --nohome_rc query --noshow_progress $TARGET --output=location)"
    "${QUERY_RESULT%%:*}" "$@"
  fi

  wrap_bazel run "$TARGET" "$@"
}
