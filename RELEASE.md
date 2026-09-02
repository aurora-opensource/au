# How to cut a new release

## Preparing the release

These are the steps to take before you actually cut the release.

### Check all commented-out test cases

These are test cases that we can't test automatically, usually because the intended behavior is
a compiler error.  Grep the codebase for `uncomment`, and test every such case individually to make
sure they still display the desired behavior.

### Check known "problem" compilers

Some compilers in our "Assumed Support" tier (see [supported compilers]) are known to be prone to
problems or errors.  We can't keep these passing on every _commit_, but if we check in on them at
_release_ time, we can keep builds passing for more users.  To be added to this list, a compiler
must (a) be requested by an actual user, and (b) fail in a way that we can't capture on our CI.

For each link below, make sure the build completes successfully.  If it doesn't, fix the build if
possible.

- [gcc 5.3](https://godbolt.org/z/EoEaK3aWs)
    - Issues seen:
        - functions giving different results in `constexpr` vs. runtime
        - incorrectly restricting `friend` template declarations
        - incorrectly failing to apply an implicit constructor

[supported compilers]: https://aurora-opensource.github.io/au/main/supported-compilers/

### Check Aurora's code

Create a draft PR which updates Aurora's internal code to the candidate release.  Make sure all of
the builds pass.  If any fail, check whether the best fix lies with Aurora's internal code, or with
Au.

GitHub will automatically generate a [tarball](https://github.com/aurora-opensource/au/tarball/main)
of the latest state of `main`.  The `strip_prefix` for this release will typically be
`aurora-opensource-au-XXXXXXX`, where `XXXXXXX` is the first 7 characters of the most recent commit.
Of course, you can also unpack it (`tar -xvzf`) to obtain the `strip_prefix` authoritatively.

#### Check cumulative compile time impact

Set up a compile time measurement using Aurora's internal code as the client code.  These
measurements should automatically switch back and forth between the previous and new release, and
should cover at least a half-dozen Au-dependent targets, ideally diverse ones.

If there is a significant regression, root cause it and see if it can be fixed.  If not, mention it
in the release notes.

### Gather information for release notes

First, go to the [latest release](https://github.com/aurora-opensource/au/releases/latest).  Click
the list of "commits to main since this release", found near the top.  Read through the commits, and
keep track of the main changes as you go.  Use the following categories.

- Upgrading from (a.b.x)
- User-facing library changes
    - If the compilation speed has been significantly impacted, mention this here.
- New units and constants
- Tooling updates
- Documentation updates
- Repo updates

Any empty section can be omitted.

## Cutting the release

### Pick a version number

We try to follow [semantic versioning](https://semver.org/).  Since we are currently in major
version zero (0.y.z), incompatible changes don't force a major version upgrade.

### Update the version number

The version number lives in exactly one place: the `AU_VERSION_MAJOR`, `AU_VERSION_MINOR`, and
`AU_VERSION_PATCH` macros in `au/version.hh`.  Edit those three macros to the number chosen above.
Everything else is derived from them:

- The C++ `AU_VERSION` macro (which downstream users read to detect the library and its version).
- The CMake `project(... VERSION ...)` and `HOMEPAGE_URL`, which the root `CMakeLists.txt` parses
  out of `au/version.hh`.  (The `HOMEPAGE_URL` points at the docs for this release, which won't
  exist until you complete the remaining steps in this guide, but the danger of getting it wrong is
  pretty small.)

**Where this commit lands depends on the release type:**

- **Minor or major release** (e.g. `0.6.0`): make a PR that bumps `au/version.hh` and land it on
  `main` _before_ creating the tag.  This is the "final commit", and it becomes the base commit for
  the release branch (see "Prepare the release branch" below).  `main` then reports this version
  until the _next_ release bumps it again, even as new (unreleased) changes land on top; this is
  expected, and matches how the version macros are documented to behave.
- **Patch release** (`0.5.1` and later): patch releases are cherry-picked from `main` onto the
  pre-existing release branch (e.g. `release-0.5.0`).  Bump `au/version.hh` in a commit _on the
  release branch_ alongside the cherry-picked fix(es), and tag that.  **Then also bump
  `au/version.hh` to the same patch version on `main`** (in an ordinary PR): since the patch is made
  _entirely_ of cherry-picks from `main`, `main` already contains everything the patch does, so it
  should advertise (at least) that version.  The one exception: if `main` has already advanced to a
  _higher_ version than the patch --- e.g. a newer minor release has since landed --- leave `main`
  alone, because it already reports a version greater than the patch.

### Fill out release notes template

The first line should be the tag name.

The second line should be blank.

Crucially, if you have future proof releases, then the notes for every one _must include specific
migration instructions_: tell users what syntax will be compatible with both the current and future
releases.

```
0.3.1  <--- NOTE: update this!

Release Notes
=============

Upgrading from (a.b.x)
----------------------

User-facing library changes
---------------------------

Compile time impact
-------------------

New units and constants
-----------------------

Tooling updates
---------------

Documentation updates
---------------------

Repo updates
------------

Future-proofing releases
------------------------

Artifacts and SHA256 sums
-------------------------

Closed Issues
-------------

Here are all of the issues that were closed since the last release.

NOTE: change dates and milestones below!  The end date should be the date you
are tagging the release.  Excluding the previous releases' milestones keeps
issues from reappearing in more than one set of release notes.

https://github.com/aurora-opensource/au/issues?q=is%3Aissue%20closed%3A2022-12-20..2023-03-18%20-milestone%3A0.3.0

Contributors
------------

Thanks to those who authored or reviewed PRs, or filed or participated in
Issues!  Alphabetically:

- @chiphogg
- ...
```

### Prepare the release branch

First, make sure the "final commit" (which updates the version in `au/version.hh`) has already
landed, and is currently checked out.  This will be the "base" commit for the release branch, which
we'll create and push to GitHub.

```sh
# Remember to update the version number!
git switch --create release-0.3.1
git push origin release-0.3.1
```

Branches named similarly to `release-0.3.1` are protected in the Au repo, so we will need to make
PRs for the final changes for the release.

#### Tag the mainline commit as `-base`

The base commit is the last commit on `main` that is part of this release, so we tag it to record
that fact.  This is an annotated tag, like every other tag in this guide.

```sh
# Remember to update the version number!
git tag --annotate 0.3.1-base
git push origin 0.3.1-base
```

Use a message of this form (first line is the tag name, then a blank line, then the body):

```
0.3.1-base

This is the mainline commit that is equivalent to the 0.3.1 release.

The only difference is that links in the C++ code and comments will
point to `/main/`, not to `/0.3.1/`.
```

This tag is not just bookkeeping.  The single-file scripts derive their version identifier from
`git describe`, so this is what makes the `main` doc website's `au.hh` report
`Version identifier: 0.3.1-base` instead of something like `0.3.0-247-gabcd1234`.  Note that patch
releases don't get a `-base` tag: they are made entirely of cherry-picks, so there is no distinct
mainline commit that corresponds to them.

### PR: Update links

Several files in the repository link to the documentation website, but they link to the version at
`main`.  This version will change over time in ways that we can't predict.  It's important for users
who use a release to have permanent links.  Therefore, the first PR for the release branch is to
find every link to `main` in those files, and replace it with a link to the release version.

**Which files?**  The rule is: everything that ships to users as part of the release, plus
everything we publish to the _versioned_ doc site.  Concretely:

- **All C++ files.**  These links show up in comments and in `static_assert` messages, so users
  encounter them directly in compiler output.
- **Everything under `docs/`**, because `mkdocs` builds that directory into the versioned site.  A
  `/main/` link on a release doc site sends readers to unversioned docs that will drift.

Two deliberate exceptions:

- **The compiler-output transcripts in `docs/troubleshooting.md`.**  These are illustrative rather
  than verbatim --- the line numbers they quote already drift from release to release --- and users
  are far more likely to search them for the error text than for a URL.
- **Repo-root files** (`README.md`, `RELEASE.md`, `CONTRIBUTING.md`, `REQUIREMENTS_LOCK.md`,
  `examples/README.md`).  These are not published to any versioned site, and `RELEASE.md`'s links
  are part of these instructions.

```
Find this:
https://aurora-opensource.github.io/au/main/...
                                       ^^^^

Replace with this (remember to update the version number!):
https://aurora-opensource.github.io/au/0.3.1/...
                                       ^^^^^
```

We do this in a PR on the release branch in order to avoid churn commits on the main branch.

Since this can reflow lines, ensure that clang-format is up to date.  The following command may be
useful:

```sh
git ls-files '*.hh' '*.cc' | xargs tools/bin/clang-format --style=file -i
```

### Create the tag for the release

Once the above PR has landed, that commit _is_ the release, so it's time to tag it as such:

```sh
# Remember to update the version number!
git tag --annotate 0.3.1
```

Copy/paste the message you composed earlier, and format it as needed.  Then, push the tag to GitHub.

```sh
# Remember to update the version number!
git push origin 0.3.1
```

### Create future-proofing releases

These give users the opportunity to modernize their project's usage of Au incrementally and at their
own pace.  Project maintainers who take advantage of this should find that when the _next_ release
comes out, they can upgrade to it without breakages.

First, go through all issues slated for a future minor-or-larger release, and identify the ones that
are suitable for this approach.  Make a list.

#### Individual issues

Make a PR against the release branch that is, essentially, "our best guess for how we will implement
this feature".  Strive for something that we could simply cherry pick when it's time to land to the
main branch.  This doesn't mean we _commit_ to doing it _exactly_ this way, but it's a good guide.
In any case, the code must be high enough quality to pass code review.

**Important:** every PR must use the release **tag** as its base commit, not the HEAD of the release
branch.  Do **not** pull in any changes from the release branch later.

When each PR lands, tag the HEAD commit _of the PR's branch_ with a tag of the format
`0.3.1-future-NNN`, where `NNN` is the issue number that the PR future-proofs for, and where `0.3.1`
must again be replaced with the actual version number.

#### Final future-proof release

At this point, the release branch consists of the release tag commit itself, followed by a series of
commits that implement future-proofing changes.  We now need to tag the HEAD of the release branch
as the final future-proofing release.

```sh
# Remember to update the version number!
git fetch origin release-0.3.1:release-0.3.1
git switch release-0.3.1
git tag --annotate 0.3.1-future
```

Write a message of the form:

```
# Remember to update the version number!
Future-proofing release for 0.3.1, comprising:

# Add one line for every future-proofed issue:
- #NNN
- #MMM
# etc.
```

(Lines starting with `#` are instructions, and should not be included in the actual message.)

Finally, push the new tag to GitHub.

```sh
# Remember to update the version number!
git push origin 0.3.1-future
```

### Audit the tags before pushing

Tags are effectively immutable once published, so check them while they are still local.  This
script verifies the whole family at once: that every tag exists and is _annotated_ (a missing `-m`
silently produces a lightweight tag), that each is in the right place, and that the version macros
and doc links are what we expect.

```sh
# Remember to update the version number!
VER=0.3.1
for t in $VER-base $VER $VER-future-NNN $VER-future; do
  printf '%-20s ' "$t"
  git rev-parse -q --verify "$t" >/dev/null || { echo "*** MISSING ***"; continue; }
  git cat-file tag "$t" >/dev/null 2>&1 && k=annotated || k="*** LIGHTWEIGHT ***"
  c=$(git rev-parse "$t^{}")
  git merge-base --is-ancestor "$c" "release-$VER" && w="on release branch" || w="off-branch"
  echo "$k  $w  $(git log -1 --format='%h %s' $c)"
done
```

Expect the `-base` tag on `main`, the release tag and the final `-future` tag on the release branch,
and each `-future-NNN` tag **off-branch** with the release commit as its parent.  Also confirm that
`au/version.hh` reports the right version at every release-side tag.

### Download all artifacts

Manually uploading releases helps future-proof us against known failure modes.  See:
https://github.blog/2023-02-21-update-on-the-future-stability-of-source-code-archives-and-hashes/

On the [tags page](https://github.com/aurora-opensource/au/tags), click the `.tar.gz` link to
download every tarball.  This will always include the release tarball, and may also include one or
more future-proof tarballs.

Verify each one against its tag before you publish its checksum.  This catches a mis-clicked link or
a truncated download at the one moment when it is still cheap to fix:

```sh
# Remember to update the version number!
for t in 0.3.1 0.3.1-future-NNN 0.3.1-future; do
  rm -rf /tmp/au-verify && mkdir -p /tmp/au-verify
  tar -xzf ~/Downloads/au-$t.tar.gz -C /tmp/au-verify
  (cd /tmp/au-verify/au-$t && find . -type f | sort | xargs sha256sum) > /tmp/from-tarball.txt
  git archive --format=tar "$t" | tar -xf - -C /tmp/au-verify --one-top-level=from-git
  (cd /tmp/au-verify/from-git && find . -type f | sort | xargs sha256sum) > /tmp/from-git.txt
  printf '%-20s ' "$t"
  diff -q /tmp/from-tarball.txt /tmp/from-git.txt >/dev/null && echo OK || echo "*** DIFFERS ***"
done
```

### Create the release

On the tags page, click the three-dots menu, and select "Create release".

- Use the version number as the title.
- Copy the body of the release notes into the Release Notes text box.
- Attach all tarballs --- the release tarball, and every future-proof release tarball --- that you
  downloaded in the previous steps.
    - Add a table with the name of each artifact (`0.3.1`, `0.3.1-future-122`, etc.) and its SHA256
      checksum.  (This both saves users the effort of downloading and hashing the files themselves,
      and also gives them an authoritatively correct value.)
- Click the `Publish release` button.

### Regenerate the doc website

First, create the version of the doc website corresponding to this release.

**Check out the release tag first.**  `mike` publishes whatever is in your working tree.

```sh
# Remember to update the version number!
git switch --detach 0.3.1
bazel run //:mike -- deploy --push 0.3.1
```

Next, we need to make sure the manifest for `au.hh` and `au_noio.hh` on the `main` doc website
includes this latest version tag.  Check out the `main` branch, and run a manual deploy to the
`main` doc website:

```sh
git switch main
bazel run //:mike -- deploy --push main
```

Afterwards, confirm the version identifier on each site is what you expect:

```sh
curl -s https://aurora-opensource.github.io/au/0.3.1/au.hh | grep -m1 'Version identifier'
curl -s https://aurora-opensource.github.io/au/main/au.hh | grep -m1 'Version identifier'
```

The first should report the release version, and the second should report `0.3.1-base`.
