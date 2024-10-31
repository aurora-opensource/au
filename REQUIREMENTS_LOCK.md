# Updating `requirements_lock.txt`

This file explains how to update the `requirements_lock.txt` file, and how to test those updates.  (We
need to refresh it periodically.)

## Performing the update

Run this command:

```sh
bazel run //:requirements.update -- --upgrade
```

## Testing the update

The main thing the requirements file affects is the documentation website.  We mainly pay attention
to changes in the desired packages, listed in `requirements.in`; keeping track of changes in
automatically pulled dependencies would be overwhelming and untenable.

### Read the release notes

- Look at the diff for `requirements_lock.txt` to get the old and new version number for every
  package listed in `requirements.in`.
- Read the release notes for every release that occurred since the previous version.
- Look out for breaking changes, and figure out how to test any that you see.

### A/B compare the website

- Run `au-docs-serve` to generate the latest version of the doc website; load it in a new window.
- In another tab, open <https://aurora-opensource.github.io/au/main/>.
- Tab back and forth to look for any changes.
      - Page down once on each page, and repeat.
      - At the end of the page, click the link to the next page, and repeat.
- Keep track of whatever changes you see.

You don't need to go through the _entire_ website, but visit enough pages to be satisfied that it's
representative.

### Make sure `mike` still works

`mike` is how we generate our versioned documentation.

First, visit the source code for the version of `mike` in the new release.  (You can find this on
their [releases](https://github.com/jimporter/mike/releases) page; click the tag link on the left
side to browse the code.)  Double check the functions that we monkey patch in `mike_bin.py`, to make
sure their interfaces and/or implementations are still up to date.

Next, make sure that our monkey patched version of mike works.  Here's a suggested sequence.

```sh
# Create a new version of the docs, named "temp-test".
#
# NOTE: this will not deploy anything remotely.
bazel run //:mike -- deploy temp-test

# View the list of versions: make sure "temp-test" is in the list.
bazel run //:mike -- list

# Serve the documentation website.  (Open it in a browser; play around.)
bazel run //:mike -- serve

# Delete the testing version.
bazel run //:mike -- delete temp-test

# Nitpicky cleanup: don't want this in our public git history.
git branch -f gh-pages origin/gh-pages
```
