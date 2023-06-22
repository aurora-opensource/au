# How to cut a new release

## Preparing the release

These are the steps to take before you actually cut the release.

### Check Aurora's code

Create a draft PR which updates Aurora's internal code to the candidate release.  Make sure all of
the builds pass.  If any fail, check whether the best fix lies with Aurora's internal code, or with
Au.

GitHub will automatically generate a [tarball](https://github.com/aurora-opensource/au/tarball/main)
of the latest state of `main`.  The `strip_prefix` for this release will typically be
`aurora-opensource-au-XXXXXXX`, where `XXXXXXX` is the first 7 characters of the most recent commit.
Of course, you can also unpack it (`tar -xvzf`) to obtain the `strip_prefix` authoritatively.

### Gather information for release notes

First, go to the [latest release](https://github.com/aurora-opensource/au/releases/latest).  Click
the list of "commits to main since this release", found near the top.  Read through the commits, and
keep track of the main changes as you go.  Use the following categories.

- User-facing library changes
- New units
- Tooling updates
- Documentation updates
- Repo updates

Any empty section can be omitted.

## Cutting the release

### Pick a version number

We try to follow [semantic versioning](https://semver.org/).  Since we are currently in major
version zero (0.y.z), incompatible changes don't force a major version upgrade.

### Fill out release notes template

The first line should be the tag name.

The second line should be blank.

```
0.3.1  <--- NOTE: update this!

Release Notes
=============

User-facing library changes
---------------------------

New units
---------

Tooling updates
---------------

Documentation updates
---------------------

Repo updates
------------

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

### Create the tag

Use the command below, replacing `0.3.1` with the version to create.

```sh
# Remember to update the tag number!
git tag -a 0.3.1
```

Copy/paste the above message, and format it as needed.  Then, push the tag to
GitHub.

```sh
# Remember to update the tag number!
git push origin 0.3.1
```

### Upload the artifact

Manually uploading releases helps future-proof us against previous breakages.  See:
https://github.blog/2023-02-21-update-on-the-future-stability-of-source-code-archives-and-hashes/

On the [tags page](https://github.com/aurora-opensource/au/tags), click the `.tar.gz` link to
download the tarball.

### Create the release

On the tags page, click the three-dots menu, and select "Create release".

- Use the version number as the title.
- Copy the body of the release notes into the Release Notes text box.
- Attach the tarball downloaded in the previous step.
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
