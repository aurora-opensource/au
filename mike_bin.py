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

import re
import subprocess
import sys

from unittest import mock
import mike.driver
import mike.mkdocs_utils


def override_version():
    """Retrieve the version in a way that's agnostic to the name of the command.

    Since the `mkdocs` executable isn't installed on our machine, but instead is
    run through bazel, it turns out that _our_ mkdocs executable thinks it has a
    different name (since we use a wrapper script).  But mike assumes both that
    we can call `mkdocs`, _and_ that it thinks its own name is `mkdocs` when we
    do.
    """
    output = subprocess.run(
        ["mkdocs", "--version"],
        check=True,
        stdout=subprocess.PIPE,
        universal_newlines=True,
    ).stdout.rstrip()

    # Original line:
    #
    #    m = re.search('^mkdocs, version (\\S*)', output)
    #
    # Changed version:
    m = re.search("^\\S+, version (\\S*)", output)

    return m.group(1)


if __name__ == "__main__":
    with mock.patch("mike.mkdocs_utils.version", override_version):
        sys.exit(mike.driver.main())
