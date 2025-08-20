# Upgrading Au

Every new release of Au improves safety, ergonomics, or both.  It's vitally important to stay up to
date.  Upgrading gets harder the longer you wait, because more breaking changes accumulate with
respect to the latest release.  Fortunately, Au has historically had only 2 to 3 significant
releases per year, so staying up to date should not be very onerous.

Au strives to enable simple, minimal upgrades.  That doesn't mean we don't have breaking changes.
It means whenever there _is_ a breaking change, we provide a syntax that works in both versions.
This lets you put your syntax updates in separate commits, so the upgrade commit does one thing
only.

Additionally, starting with Au 0.5.0, we provide new tools to get _ahead_ of the game, and become
compatible with changes we know we'll make in the _next_ release.  These "future proof" release
artifacts empower you to make upgrades as painless as possible.  We'll explain below how to use
them.

## Direct versus incremental

If your current release was the latest when the new release came out, then you can skip this
section: a direct upgrade is already as "incremental" as can be.  Otherwise, you've got a choice:

- **Direct:** Upgrade directly to the latest release, ignoring any intermediate releases.
- **Incremental:** Upgrade stepwise, one release at a time, until you reach the latest release.

The direct approach can be less work overall, if it works well.  The downside is that it can become
overwhelming if there are too many failures.  In that case, the incremental approach lets you deal
with the minimum number of breaking changes at one time.

A good compromise can be to start by trying the direct approach, and switch to the incremental
approach if things become overwhelming.

!!! tip
    The rest of the guide covers upgrading from one specific version to another specific version.
    Therefore, if you choose the incremental approach, you would apply the instructions repeatedly,
    once for each "step".

## Basic upgrade process

Suppose you are on version `A`, and upgrading to version `B`.  Here are the recommended steps to take:

1. Using your installation method, switch Au from version `A` to version `B`.

    - This depends greatly on whatever method of installation you used.  See our [installation
      guide](../install.md) for details.

2. Run all builds and tests for your project, fixing all failures as you go.

    - Read the release notes for every version _after_ `A`, up to and including `B`.  Any potential
      breaking changes will be listed prominently at the top.  If the change needs special guidance
      for how to resolve it, we will include it there too.

    - Continue to iterate until your full project builds and all tests pass.

    - _**Optional:** If release `A` has future proof artifacts, you can use them to make this step more
      incremental._  (The first release with future proof artifacts is 0.5.0.  Not every subsequent
      release will necessarily have these.  Check your release notes.)

        - For every future proof release artifact (`A-future-1`, `A-future-2`, etc.), use it instead
          of release `B`, and fix the errors you find.  Then switch to the next future proof artifact,
          and repeat.

        - When your project is simultaneously clean under all future proof artifacts, switch back to
          release `B`: you should find far fewer errors, if any at all.

    - At the end of this step, your project will build cleanly under release `B`.  You can certainly
      land this commit as is, combining the upgrade with syntax updates, but we recommend a cleaner
      approach: make well scoped syntax update commits, followed by a final simple commit to upgrade
      the library.

3. **Optional:** Make commits for syntax updates.

    - A good first step is to temporarily revert to the previous version ("version `A`"), but keep
      all of the fixes.  If the library builds, then you can put all of your syntax updates into
      commits that land before the upgrade.  However, _any breakages will have to be included in the
      same commit that updates the library_.  We strive to avoid ever forcing users to do this, but
      it may happen.

    - Group the commits logically to keep them single purpose.  In particular:

        - Assuming they go through code review, the review will be easier if _each commit makes only
          one kind of syntax change_.

        - If a syntax change has very many instances, it may even be worth breaking it up into
          smaller commits, perhaps grouping them by areas of the codebase.

4. Land the commit that updates the library.

    - If you followed step 3, this will usually be a very simple commit that just changes the Au
      artifact you are using.

    - If you opted out of step 3 --- or, if you encountered incompatible changes --- then it will
      also include syntax updates.

At this point, you are on the new version of Au.  Read the rest of the release notes, and find out
about all the goodies you just unlocked!

### Future proof releases

The flaw with the above approach is that the breaking change updates happen _when the new release is
already ready_, which creates time pressure.  Even if you break up the syntax updates nicely into
smaller commits, it's still a lot of work all at once.

Starting with Au 0.5.0, we provide new tools --- _future proof release artifacts_ --- to make that
upgrade process more incremental, and easier to do at your leisure.  Each future proof artifact is
a copy of the release, but with _one specific future breaking change_ pulled forward and applied.
You can use it to find all of the callsites that you'll need to update when that breaking change
lands to the main branch.  Crucially, _those updates are also compatible with the current release_,
so you can make them immediately.

We also provide an "overall" future proof release that contains _all_ individual future proof
changes, combined.

Here are the detailed steps for how to use all of these.  Assume that you have just finished
upgrading to version `B`:

1. In your local client, replace your version of Au (version `B`) with the first future proof
   artifact --- say, `B-future-1`.

    - The number after `-future-` --- here, `1` --- indicates which
      [issue](https://github.com/aurora-opensource/au/issues) this artifact is future-proofing for.

2. Run all builds and tests for your project, fixing all failures as you go.

    - The release notes will tell you how to resolve the issues for that specific future proof
      artifact.  They will provide a syntax that is compatible with both the current and the future
      proof release.

3. Revert your local client back to the "current" release, `B`, and land the syntax changes you have
   made --- whether as one commit, or several.

4. Repeat steps 1-3 for every future proof artifact.

5. Make a "long-lived pull request" whose _only change_ is to update the Au artifact from version
   `B` to `B-future`: an artifact that contains _all_ future proof changes.

    - This is _not for landing_; it is a tool to keep your project up to date.

6. From time to time, merge the latest version of mainline into this pull request, to catch any new
   inconsistencies as they pop up.

    - This works best if your project automatically builds and runs all tests on pull requests: you
      can lean on your Continuous Integration (CI) to find the issues.  If not, you'll have to build
      and test manually.

Note that these steps can take place over as long a time as you like.  In this way, you can prepare
your project for future releases incrementally, and at your own pace. When the next release comes
out, upgrading will be as painless as possible: you should only have to deal with whatever breaking
changes we couldn't anticipate ahead of time.
