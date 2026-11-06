# Branch Rules

This document defines how branches, commits, and common Git operations are handled in the uEDP repository.

## 1. Branch Overview

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
