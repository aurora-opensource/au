import sys

import mkdocs.__main__


def main(argv):
    mkdocs.__main__.cli(argv[1:])


if __name__ == "__main__":
    sys.exit(main(sys.argv))
