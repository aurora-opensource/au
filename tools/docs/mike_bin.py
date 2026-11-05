# Copyright 2023 Aurora Operations, Inc.
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

import os
import pathlib
import subprocess
import sys

from unittest import mock
import mike.driver
import mike.mkdocs_utils
from mike.mkdocs_utils import docs_version_var


# We need to monkey patch any function in mike which calls `mkdocs` via its
# command line interface.  As of mike 2.2.0, there is only one: `build`.  (Mike
# used to shell out to `mkdocs --version`, but it now reads the version from
# `importlib.metadata` at import time, so there is nothing to patch.)


def override_build(config_file, version, *, quiet=False, output=None):
    """Override mike's native logic for building the docs."""
    # Original line:
    #    command = (
    #        ['mkdocs'] + (['--quiet'] if quiet else []) + ['build', '--clean'] +
    #        (['--config-file', config_file] if config_file else [])
    #    )
    # Changed version (reformatted with Black):
    command = (
        ["update_docs"]
        + (["--quiet"] if quiet else [])
        + ["build", "--clean"]
        + (["--config-file", config_file] if config_file else [])
    )

    env = os.environ.copy()
    env[docs_version_var] = version

    subprocess.run(command, check=True, env=env, stdout=output, stderr=output)


if __name__ == "__main__":
    # `update_docs` is our name for `mkdocs`.  It will be found in the same folder as this script.
    # We need to add this folder to the PATH.  (That is not the working directory: `bazel run` starts
    # us at the root of our runfiles, which is where `mkdocs` expects to find `mkdocs.yml`.)
    os.environ["PATH"] = f"{pathlib.Path(__file__).absolute().parent}:{os.environ['PATH']}"

    with mock.patch("mike.mkdocs_utils.build", override_build):
        sys.exit(mike.driver.main())
