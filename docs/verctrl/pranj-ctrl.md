# Branch Rules

This document defines how branches, commits, and common Git operations are handled in the uEDP repository.

## 1. Current Branch Overview

| Branch | Purpose | Direct push | Notes |
| -------- | --------- | ------------- | ------- |
| `main` | Stable, releasable code | Not allowed | Changes come only through pull request |
| `release` | Release candidates and release fixes | Not allowed | Changes come only through pull request |
| `feat` | Integration branch for features | Not allowed | Feature branches merge here |
| `fix` | Integration branch for bug fixes | Not allowed | Fix branches merge here |
| `doc` | Integration branch for documentation | Not allowed | Documentation branches merge here |
| `chore` | Integration branch for maintenance | Not allowed | Build, tooling, and housekeeping |
| `<type>/<name>` | Short-lived work branch | Allowed (owner only) | Deleted after merge, except for `app/<name>` |
| `app/<name>` | Application-specific branch | Allowed (owner only) | Long-lived, used for application-specific development which rely on testobj for delta test and regression testing. |

Merge direction before halfway of v1.1.7:

Changes are merged directly from work branches into `feat`, `fix`, `doc`, or `chore`. Before or after the main release like v1.1.6, v1.1.7, all changes must be merged into `release` first, then merged into `main` after the release is complete.

However, after v1.1.7, we will create a new branch `sub-feat/v1.1.7` to contain all features for v1.1.7, and merge it into `feat` only when all features are complete and tested. Any new features from v1.1.8 and beyond will follow the same pattern, creating a new branch for each version. The pattern for `release` and `main` will remain the same, where all changes must be merged into `release` first, then merged into `main` after the release is complete.

## 2. Branch Naming

Format:

```example
sub-<type>/<short-description>
```

- `<type>` is one of `feat`, `fix`, `doc`, `chore`, `release`, `struct`.
- `<short-description>` is lowercase, words separated by hyphens, 3 to 5 words.
- Include the version or component when relevant: `sub-feat/testobj-v2-negative-cases`.

Examples:

```examples
sub-feat/testobj-v2-negative-cases
sub-fix/fcr-queue-full-severity
sub-doc/psetbl-pubsub-section
sub-chore/update-kconfig-docker
```

Do not use:

- Personal names in branch names (`shanghuang/test`)
- Dates or ticket-only names without a description (`feat/123`)
- Uppercase letters, spaces, or underscores

This example only applied to naming but not for real developing experience. We consider that sub branch name will only used for versioning, not for any objects. Therefore, `sub-feat/v1.1.7` or `sub-feat/v1.1.6-bf4` is a better naming convention for versioning, `sub-feat/testobj-v2-negative-cases` is not allowed for using in versioning. The reason is that we want to keep the versioning branch name simple and clear, and avoid dividing the versioning branch into too many sub-branches. The versioning branch should be used for all features and fixes related to that version, and any task branches should be created from the versioning branch.

## 3. Splitting Branches by Feature Version

Use a version suffix to separate increments of the same feature.

```examples
sub-feat/testobj-v0      <- first version, merged to sub-feat when stable
sub-feat/testobj-v1      <- next version, branched from updated sub-feat
sub-feat/testobj-v2      <- next version
```

For work inside one version, branch from the version branch:

```examples
sub-feat/testobj-v2                  <- version branch (long-lived for this version)
sub-feat/testobj-v2-negative-cases   <- short-lived task branch
sub-feat/testobj-v2-gda-matrix       <- short-lived task branch
```

Rules:

1. Create task branches from the version branch, not from `main`.
2. Open a pull request for each task branch into its version branch.
3. Keep each task branch small enough to review in one sitting.
4. Delete task branches after they are merged.
5. Open a pull request from the version branch into `sub-feat` only when the version is complete.

`feat` is currently contains features ranging from v1.1.6-bf3 to v1.1.7. Therefore, any release merging from branch `feat` will contain features that are not fully tested. To avoid this, we will create a new branch `sub-feat/v1.1.7` to contain all features for v1.1.7, and merge it into `feat` only when all features are complete and tested. Any new features from v1.1.8 and beyond will follow the same pattern, creating a new branch for each version.

## 4. Commit Messages

Format:

```example
<type>(<scope>)[<name>]: <short subject>
```

- `<type>`: `feat`, `fix`, `docs`, `chore`, `struct`, `test`.
- `<scope>`: the module or document, such as `fcr`, `psetbl`, `testobj-v2`, `to-do`.
- `<name>`: the name of the person or team responsible for the change.
- Subject: imperative mood, lowercase start, no trailing period.

Examples from the current history:

```examples
feat(testobj-v2)[shanghuang]: add negative test cases for FCR and GDP handling
docs(psetbl)[minh]: add Pub-Sub Engine section with signal-task relationship examples
fix(fcr)[dat]: update action for TASK message queue full
```

Keep one logical change per commit. Commit early on a work branch and clean up the history before opening the pull request.

## 5. Cherry-Pick

Use cherry-pick to copy a specific commit to another branch, such as moving a bug fix from `feat` to `release`.

Steps:

```bash
git switch <target-branch>
git pull --ff-only
git cherry-pick -x <commit-sha>
```

- `-x` appends the original commit hash to the message, which helps traceability.
- For a range, use `git cherry-pick <first-sha>^..<last-sha>`.
- On conflict, resolve the files, then run `git add <file>` and `git cherry-pick --continue`.
- To cancel, run `git cherry-pick --abort`.
- Push the target branch through a pull request. Do not push cherry-picked commits directly to `main` or `release`.

## 6. Undo a Commit

### 6.1 Commit not pushed yet

Undo the commit and keep the changes staged:

```bash
git reset --soft HEAD~1
```

Undo the commit and keep the changes unstaged:

```bash
git reset HEAD~1
```

Discard the commit and its changes. This cannot be recovered easily, so confirm first:

```bash
git reset --hard HEAD~1
```

### 6.2 Commit already pushed

Do not rewrite shared history. Create a new commit that reverts the change:

```bash
git revert <commit-sha>
git push
```

### 6.3 Fix only the last commit message

```bash
git commit --amend -m "<type>(<scope>): <new subject>"
```

Amend only commits that are not pushed, or that were pushed to your own work branch and have not been used by anyone else.

## 7. Stash Changes

Save uncommitted work to switch branches without committing:

```bash
git stash push -u -m "<short description>"
```

- `-u` includes untracked files.
- List stashes: `git stash list`
- Restore and remove the latest stash: `git stash pop`
- Restore a specific stash and keep it: `git stash apply stash@{<n>}`
- Create a branch from a stash: `git stash branch <new-branch> stash@{<n>}`
- Delete a stash after it is no longer needed: `git stash drop stash@{<n>}`

Stashes are local only. Do not rely on them as backup for work you care about; commit to a work branch instead.

Naming convention for stashes is belong to local, therefore, it is impossible to control preference of stash name. To avoid confusion, we will use the following naming convention for stashes:

```examples
1534 - 011026
```

where 1534 is the time in 24-hour format, and 011026 is the date in MMDDYY format. This will help to identify the stashes easily.

## 8. Update a Branch (Rebase and Force Update)

Before rewriting a branch, fetch the latest state:

```bash
git fetch origin
```

This is mandatory before rebasing or force-pushing, to avoid overwriting someone else's work or accidentally creating a merge commit that breaks the history.

Update a work branch on top of its base:

```bash
git rebase origin/<base-branch>
```

Force update after a rebase or amend. Always use `--force-with-lease`:

```bash
git push --force-with-lease origin <your-branch>
```

Rules for force updates:

1. Only force-push your own work branch.
2. Never force-push `main`, `release`, `feat`, `fix`, `doc`, or `chore`.
3. Never use `--force` without `--force-with-lease`. It can overwrite commits someone else pushed.
4. Before force-pushing, confirm no one else has commits on the branch (`git log origin/<your-branch>..HEAD` and check the remote log).
5. If a teammate is working on the same branch, coordinate before rewriting it.

## 9. Pull Requests

- Open a pull request for every merge into an integration branch or `main`.
- Target branch must match the rule in section 1.
- Use the pull request template: see [pull-request-template.md](pull-request-template.md).
- Keep the title in commit message format: `<type>(<scope>): <subject>`.
- Delete the source branch after merge.

## 10. Conflicts when fetching

When fetching, if you see the branch is spilting into two branches or having merge commits, it means that your local branch is not up to date with the remote branch. To resolve this, you can use the following command to reset your local branch to match the remote branch:

```bash
git reset --hard origin/<branch-name>
```

or you can reset the local branch to before the merge commit then fetch the remote branch again:

```bash
git reset --hard <commit-sha>
git fetch origin
```

## 11. Toward new branch model from v1.1.7 release

### 11.1. Current situation for branch `fix`

Currently, we have a branch `fix` which is used for bug fixes. However, since the release of v1.1.4, we have not been using this branch for bug fixes. Instead, we have been merging bug fixes directly into the `feat` branch. This has caused some confusion and issues with our release process as the `fix` branch is not being used as intended.

To address this, it is encouraged to use the `fix` branch for bug fixes, and to merge bug fixes into the `fix` branch first before merging into `feat`. This will help to keep the release process organized and ensure that bug fixes are properly tracked and tested before being included in a release.

### 11.2. Recommendation for branch `fix` and branch `feat`

Furthermore, it should make up a table to ensure the usage of each branch is clear and consistent. The following rules are proposed:

| What changes | Branch | Rule |
| --- | --- | --- |
| Add new capability/behavior | feat | feat/... |
| Existing behavior is incorrect from requirement | fix | fix/... |
| Crash / exception / incorrect output | fix | fix/... |
| Regression | fix | fix/... |
| Test failed due to implementation defect | fix | fix/... |
| Cleanup code, behavior is not changed | refactor | refactor/... |
| Documentation only | doc | doc/... |
| Maintenance/tool/config dependency | chore | chore/... |

This is also in line with Conventional Commits where fix = patch a bug, while feat = introduce new feature. [[conventionalcommits.org]](https://www.conventionalcommits.org/en/v1.0.0/?ref=refind)

However, it should be noted that `feat` and `fix` branches are closely related to the type of change being made. Therefore, when encountering a bug that requires actions, we should ask:

> If we make this change, is the current software violating any requirements or expected behavior?
>
> YES → fix
> NO, the change is adding new capability → feat

For example below:

```examples
Requirement: 

Feature A shall set signal X = 0 when system enters state YX.
Actual: 
  X remains 1. 
Change: 
  Correct condition to set X = 0.
```

In this case, it is a fix, even though the developer has to write new logic.

In contrast:

```examples
New requirement: 

Feature A shall support a new NG state. 
Current implementation: 
  No NG state. 
Change: 
  Implement NG state.
```

In this case, it is a feat.

> This is important because "I am adding new code" **does not mean** feat.

### 11.3. Release-Integration branch model

Currently, we have a branch model where `feat`, `fix`, `doc`, and `chore` branches are long-lived integration branches. However, this can lead to confusion and issues with the release process as these branches can become cluttered with changes that are not yet ready for release or may not be relevant to the current release like currently, `feat` branch contains features ranging from v1.1.6-bf3 to v1.1.7. Therefore, any release merging from branch `feat` will contain features that are not fully tested.

To address this, v1.1.7 release to v1.1.8 will apply a new changes to the branch model with `main`, `integration`, `release`, `docs` as follow:

```asciidocs
                     integration              ┌───────────────────────────────────────────────────────────────────────────────────────────┐
│ Rv1.1.8-i0 ◄───────    release ◄──┐         │ release                                                                 development phase │
│        -i1                        │         │ candidate bug   ┌─── reft - refactor ◄─────┐                                              │
▼        -i2        final release   ├─────────┼────────────────►│       │                  │                                              │
                  ┌─────────────────┼──────┐  │                 ├─── fix│ - bugfix   ◄─────┤                                              │
                  │                 │      │  │                 │    ▼  ▼                  │                                              │
    ▲ v1.1.7 ◄────┼─ main ◄───── release ◄─│┬─│─ integration ◄──┴─── feat - feature ────┬──┼─► app/testobj-v? - specific application      │
    │ v1.1.6      │   │                    ││ │                             ▲           │  │                    codebase for testing      │
    │ v1.1.5      └───┼────────────────────┘└─│─ docs ◄──── chore           │           └──┴─► sub-feat/v?.?.? - single branch for        │
                      ▼                     ▲ │                             │                       │            specific version release │
│ v1.1.6-bf1 ◄────── hotfix ────────────────┘ │                             └───────────────────────┘                                     │
│ v1.1.6-bf2                  if need         │                              only merge after stable                                      │
│ v1.1.6-bf3                                  └───────────────────────────────────────────────────────────────────────────────────────────┘
▼ v1.1.6-bf4                                                                                                                               
```

The branching model separates the software lifecycle into development, release preparation, and final release stages.

During the development phase, `feat` serves as the primary feature-development and integration stream, where application-specific branches such as `app/testobj-v?` can be used to develop and test a particular application codebase, while `sub-feat/v?.?.?` isolates development for a specific feature or version and is merged back into `feat` only after it reaches a stable state.

Supporting changes are separated by purpose:

- `reft` is used for refactoring that does not intentionally introduce new functionality
- `fix` is used for correcting bugs found during normal development
- `chore` is used for maintenance, tooling, configuration, or housekeeping changes
- `docs` contains documentation-related updates.

Stable development changes are consolidated into `integration`, which acts as the convergence point before a release candidate is created.

If a bug is discovered during release-candidate validation, the bugfix (`-bfX` suffix) is created from the affected `release`/`integration` baseline and merged back into the relevant development stream so that the correction is preserved in subsequent versions. Once the release candidate has been validated and considered stable, it progresses through `release` and is finally merged into `main`, where `main` represents the official released codebase.

Bugs discovered after the software has already been released are handled separately through a `hotfix` created from `main`; after verification, the `hotfix` is merged back into `main` for the corrective `release` and propagated back toward the active development or `integration` stream when required, preventing the same defect from reappearing in future releases.

Overall, this model keeps feature development (`feat`), development bug fixing (`fix`), refactoring (`reft`), maintenance (`chore`), documentation (`docs`), release stabilization (`integration`/`release`), and production correction (`hotfix`) logically separated while maintaining a controlled forward path from development to final release and a backward synchronization path for critical fixes.

### 11.4. When to use pull request and when to merge directly

In current implementation, pull request - PR is used for any changes required to be merged. Therefore, it is inconclusive to use PR for all changes, especially for small changes like typo fix or doc update. Therefore, we will use the following rules to determine when to use PR and when to merge directly:

| Change type | Merge method | Rule | Example |
| --- | --- | --- | --- |
| Small fix (e.g., typo, doc update) | Direct merge | For minor changes that don't require review | Fixing a typo in a comment or updating a README file, merge documents into sub feature branches |
| Medium fix (e.g., bug fix, small feature) | Pull request | For changes that require review and testing | Fixing a bug in the code or adding a small feature, merge into sub feature branches, usually for more significant changes from fix/reft into sub feature branches |
| Large fix (e.g., major feature, refactor) | Pull request | For changes that require review, testing, and approval | Adding a new feature or refactoring a large section of code, merge into sub feature branches, usually for more significant changes sub feature branches into main feature branches |
