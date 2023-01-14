#!/bin/bash
# Copyright 2022 Aurora Operations, Inc.

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
  echo -n "$USER_MESSAGE"
  wrap_bazel build "$TARGET"
  echo -n "$USER_MESSAGE" | sed 's/./\x08 \x08/g'

  # Run _real_ command.
  wrap_bazel run "$TARGET" "$@"
}
