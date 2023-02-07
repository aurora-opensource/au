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

function wrap_bazel () {
  COMMAND="$1"; shift
  TARGET="$1"; shift

  bazel \
    --nohome_rc \
    "$COMMAND" \
    --ui_event_filters=-info,-stdout,-stderr \
    --noshow_progress \
    "$TARGET" \
    -- \
    "$@"
}

function make_command_from_bazel_run () {
  TARGET="$1"; shift
  USER_MESSAGE="Building tool.  If slow, you can Ctrl-C and run: bazel --nohome_rc build $TARGET"

  # Write message, then run _building_ command.
  # When done: back up; then, write spaces; then, back up again.
  echo -n "$USER_MESSAGE" >&2
  wrap_bazel build "$TARGET"
  echo -n "$USER_MESSAGE" | sed 's/./\x08 \x08/g' >&2

  # Run _real_ command.
  wrap_bazel run "$TARGET" "$@"
}
