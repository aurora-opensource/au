# How to cut a new release

## Preparing the release

These are the steps to take before you actually cut the release.

### Check all commented-out test cases

These are test cases that we can't test automatically, usually because the intended behavior is
a compiler error.  Grep the codebase for `uncomment`, and test every such case individually to make
sure they still display the desired behavior.

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

### Update the CMake version number

Edit the `CMakeLists.txt` file in the root folder, updating the version number in the `project`
command to the number chosen above.

Also update the version number in the `HOMEPAGE_URL` parameter, because we link to the docs for the
latest release in our CMake project definition.  (True, this URL won't exist until you complete the
remaining steps in this guide, but the danger of getting it wrong is pretty small.)

Make a PR with these changes and land it before creating the tag.

### Fill out release notes template

The first line should be the tag name.

The second line should be blank.

```
0.3.1  <--- NOTE: update this!

Release Notes
=============

Upgrading from (a.b.x)
----------------------

User-facing library changes
---------------------------

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

Closed Issues
-------------

Here are all of the issues that were closed between these releases.  (Note that
the resolution is at the level of days, so some of these issues might show up in
the notes for more than one release.)

NOTE: change dates below!

https://github.com/aurora-opensource/au/issues?q=is%3Aissue+closed%3A2022-12-20..2023-03-18

Contributors
------------

Thanks to those who authored or reviewed PRs, or filed or participated in
Issues!  Alphabetically:

- @chiphogg
- ...
```

### Prepare the release branch

First, make sure the "final commit" (which updates the CMake variables) has already landed, and is
currently checked out.  This will be the "base" commit for the release branch, which we'll create
and push to GitHub.

```sh
# Remember to update the version number!
git switch --create release-0.3.1
git push origin release-0.3.1
```

Branches named similarly to `release-0.3.1` are protected in the Au repo, so we will need to make
PRs for the final changes for the release.

### PR: Update links

Several C++ files in the repository link to the documentation website, but they link to the version
at `main`.  This version will change over time in ways that we can't predict.  It's important for
users who use a release to have permanent links.  Therefore, the first PR for the release branch is
to find every link to `main` in C++ files, and replace it with a link to the release version.

```
Find this:
https://aurora-opensource.github.io/au/main/...
                                       ^^^^

Replace with this (remember to update the version number!):
https://aurora-opensource.github.io/au/0.3.1/...
                                       ^^^^^
```

We do this in a PR on the release branch in order to avoid churn commits on the main branch.

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

### Download all artifacts

Manually uploading releases helps future-proof us against known failure modes.  See:
https://github.blog/2023-02-21-update-on-the-future-stability-of-source-code-archives-and-hashes/

On the [tags page](https://github.com/aurora-opensource/au/tags), click the `.tar.gz` link to
download every tarball.  This will always include the release tarball, and may also include one or
more future-proof tarballs.

### Create the release

On the tags page, click the three-dots menu, and select "Create release".

- Use the version number as the title.
- Copy the body of the release notes into the Release Notes text box.
    - For every future-proof release, _include specific migration instructions_: tell users what
      syntax will be compatible with both the current and future releases.
- Attach all tarballs --- the release tarball, and every future-proof release tarball --- that you
  downloaded in the previous steps.
    - Add a table with the name of each artifact (`0.3.1`, `0.3.1-future-122`, etc.) and its SHA256
      checksum.  (This both saves users the effort of downloading and hashing the files themselves,
      and also gives them an authoritatively correct value.)
- Click the `Publish release` button.

### Regenerate the doc website

First, create the version of the doc website corresponding to this release.

```sh
bazel run //:mike -- deploy --push 0.3.1
```

Next, we need to make sure the manifest for `au.hh` and `au_noio.hh` on the `main` doc website
includes this latest version tag.  Check out the `main` branch, and run a manual deploy to the
`main` doc website:

```sh
git checkout main
bazel run //:mike -- deploy --push main
```
