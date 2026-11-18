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
    │ v1.1.5      └───┼────────────────────┘└─│─ docs ◄──── chore           │       ┌──►└──┴─► sub-feat/v?.?.? - single branch for        │
                      ▼                     ▲ │    └────────────────────────┼───────┘               │            specific version release │
│ v1.1.6-bf1 ◄────── hotfix ────────────────┘ │    only main dev release    └───────────────────────┘                                     │
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

Specifically for PR from any `sub-feat/` branch into `feat` branch, it is required to have at least 3 major changes to consider a PR before merging into `integration` branch with documentation for release.

### 11.5. When does feature ready for integration

In current implementation, `sub-feat/Rv1.1.8`, `sub-feat/Rv1.1.9`, and `sub-feat/Rv1.1.7-bf1` are used for feature development or applying new bugfixes for each release. However, it is not clear when a feature is considered ready for integration into the main feature branch (`feat`). Therefore, we will use the following rules to determine when a feature is ready for integration:

- The feature has been fully implemented and tested in the `sub-feat/` branch.
- The feature has been reviewed and approved by the necessary stakeholders.
- The feature is compatible with the current state of the main feature branch (`feat`) or required changes that are needed to be merged into the main feature branch (`feat`) have been identified and addressed.

## 12. Changes selection

### 12.1. Documentation update between development branches

When we have multiple branches for development among release, having only one branch for documentation can cause issues due to the fact that only current next release source code can be merged into the documentation branch. Therefore, we will need a rule to select which changes from docs branch to support documentation among multiple development branches. The following rules are proposed:

> using `git restore --source <branch> <file>` to restore the file from the documentation branch to the current working branch, then commit and push the changes to the current working branch. This will ensure that the documentation is up to date with the latest changes and also allow for the documentation to be updated without having to merge the entire documentation branch into the current working branch.

Along with the rules, commit message should be in the following format:

```examples
struct(docs > <file_name>): <short description>
```

Where `<file_name>` is the name of the file being updated, and `<short description>` is a brief summary of the changes made to the file. This will help to keep track of changes made to the documentation and ensure that they are properly documented in the commit history.

For example, current next release is v1.1.8, and we have two development branches `sub-feat/v1.1.8` and `sub-feat/v1.1.9`. Due to current development is prioritize for v1.1.8, the source code from `sub-feat/v1.1.8` will be allowed to be merged into the documentation branch. If we want to update the task list for v1.1.9, apply the rule above:

```bash
git restore --source doc docs/to-do.md
```

Then commit and push the changes to the `sub-feat/v1.1.9` branch with the following commit message:

```examples
struct(docs > to-do.md): update task list for v1.1.9
```

Follow the same rule for any other files in the documentation branch. This will ensure that the documentation is up to date with the latest changes and also allow for the documentation to be updated without having to merge the entire documentation branch into the current working branch.

Furthermore, if any errors raised from migration and integration process of submodulize, cherry-pick for current development branch is acceptable to support documentation update with out version control errors. However, it is important to ensure that the cherry-picked changes are properly documented and tracked in the commit history to avoid confusion and ensure that the documentation is accurate and up to date.

### 12.2. Code update between development branches

When we have a release that has been released and we are having branch `fix` to apply bug fixes for multiple release, it may create a situation where merging the `fix` branch into the `sub-feat/` branch for that release may cause redundant changes or conflicts. Therefore, we will need a rule to select which changes from `fix` branch to support code update among multiple development branches. The following rules are proposed:

> using `git cherry-pick <commit-sha>` to select the specific commit from the `fix` branch to be applied to the current working branch, then commit and push the changes to the current working branch. This will ensure that only the necessary changes are applied to the current working branch and also allow for the code to be updated without having to merge the entire `fix` branch into the current working branch.

Then add a commit message as follows:

```bash
git commit --allow-empty -m "sub-feat(dummy): add dummy commit to highlight new fix copied from branch fix"
```

This will create a new commit in the current working branch that highlights the new fix that has been copied from the `fix` branch. The `--allow-empty` flag allows for an empty commit to be created, which can be useful for highlighting changes that have been made without actually modifying any files.

## 13. Ticket append and update

When we have a ticket that has been created for a specific feature or bug fix but we need to move or hold the ticket for a later release, we will need a rule to append or update the ticket to reflect the changes made. The following rules are proposed:

> If the ticket is considered to be moved to a later release, we will need to update the current ticket with tag `DEPRECATED`. In new release section, this ticket will follow new ticket number for the new release but adding a reference to the old ticket number. This will ensure that the ticket is properly tracked and documented in the commit history.

For example, in v1.1.8, ticket PLTF-012, PLTF-014, PLTF-015, PLTF-016 are considered to be moved to v1.2.0. Therefore, we will update the current ticket with tag `DEPRECATED`, non-ticket and create new tickets for v1.2.0 with same epic and reference to the old ticket numbers. In v1.2.0, the new tickets can follow the format as below:

```md
# in v1.1.8
- [ ] @! DEPRECATED[epic=Rv1.1.8-i0/pltf.libunity-integration] PLTF-016 <+

# in v1.2.0
- [ ] @! DEPRECATED[epic=Rv1.1.8-i0/pltf.libunity-integration] PLTF-XXX (REF: Rv1.1.8/PLTF-016) <+
```

## 14. Submodulize and policy against checkout, PR

Since μEDP has been submodulized with PLTF, it is important to add new policy and guidelines for submodule usage, especially when it comes to checking out and creating pull requests.

### 14.1. Pinning rules

- `pltf` is not tracked the same way on every branch (snapshot 2026-11-18). Only `sub-feat/Rv1.1.8` has `pltf` as a submodule (gitlink, mode 160000) and a `.gitmodules` file. On `main`, `feat`, `fix`, `doc`, `chore`, `reft`, `sub-feat/Rv1.1.7-bf1`, and `sub-feat/Rv1.1.9`, `pltf` is tracked as regular files. `release` and `integration` have no `pltf` path.
- On a branch with a gitlink, the pinned commit is the PLTF version that the branch contains. Different branches may pin different commits.
- Never commit a gitlink that points to a PLTF commit that is not pushed to the PLTF remote.
- Do not use `git submodule update --remote` on `main`, `release`, or any branch used for build or release verification. These branches must follow the pinned commit.

### 14.2. Checkout and switch

1. Before switching μEDP branches, make sure `pltf` has no uncommitted or unpushed work: `git -C pltf status`. Commit or stash inside `pltf` first.
2. After any `git switch`, `git checkout`, `git pull`, or `git merge` that changes `pltf` or `.gitmodules`, run:

   ```bash
   git submodule sync --recursive
   git submodule update --init --recursive
   git submodule status --recursive
   ```

3. Do not build, test, or open a PR until `git submodule status --recursive` shows no `+` (checked-out commit differs from the pin), `-` (not initialized), or `U` (conflict) prefix.
4. Do not develop in detached HEAD inside `pltf`. Switch to or create a named branch in `pltf` before committing.

### 14.3. Merge between branches

1. Before merging, check whether the source branch changes the submodule pin: `git diff --submodule=log <target>...<source>`.
2. If the source and target pin different PLTF commits, resolve the gitlink conflict as follows:
   - `git -C pltf fetch --all`
   - Choose the integration commit. If neither pin is a descendant of the other, merge the PLTF branches inside `pltf` and push the result first.
   - Push the PLTF integration commit to the remote.
   - In μEDP, run `git add pltf`, then complete the merge.
3. A merge or cherry-pick that changes the `pltf` gitlink or `.gitmodules` is never merged directly, even when the change is small. It must go through a pull request (see section 11.4).
4. When a branch must not receive PLTF, cherry-pick the required commits instead of merging the whole branch, or add the submodule in a separate, isolated commit.
5. Cherry-picking a commit that changes the `pltf` gitlink requires approval. The PLTF commit must already exist on the remote. Run the submodule update and status commands afterwards.

### 14.4. Pull requests

- The PR description must state the PLTF pin change: old commit, new commit, and PLTF branch or tag.
- A PR that changes only the gitlink uses the commit message `chore(pltf)[<name>]: update pltf submodule to <short-sha>`. Do not mix a pin change with unrelated μEDP changes in the same PR.
- Review pin changes with `git diff --submodule=log <target>...<source>`.
- Once `release` or `main` track `pltf` as a gitlink, PRs into them must pin a PLTF commit that is part of an approved PLTF release or tag.
- CI must be able to fetch the pinned PLTF commit. Credentials must grant access to both μEDP and PLTF.

### 14.5. Checklist

| Action | Before | After |
| --- | --- | --- |
| Switch or checkout | `pltf` is clean and pushed | `submodule update --init --recursive`, then `submodule status` shows no `+`, `-`, or `U` |
| Merge | Know which side changes the pin | Gitlink resolved, PLTF integration commit pushed, `git add pltf` done |
| Update pin | PLTF commit pushed | Gitlink-only PR, CI verifies the pinned commit can be fetched |
| Cherry-pick a pin change | Approval given, PLTF commit on remote | Submodule update and status checked |
| Switch across the gitlink boundary (`sub-feat/Rv1.1.8` to or from a branch without the gitlink) | `pltf` has no uncommitted or unpushed work | No `pltf` change recorded on the target branch; `submodule status` checked where a gitlink exists |

### 14.6. Switching across the gitlink boundary

Observed behaviour when switching from `sub-feat/Rv1.1.8` to a branch such as `doc` or `feat`:

1. Git reports local changes inside `pltf` that must be committed in the PLTF repository. These changes belong to the current branch, not the target branch.
2. If those changes are discarded, Git then reports new `pltf` changes on the target branch and asks for a commit. The result is equivalent to applying PLTF changes to the target branch without a pull request.

This is not an allowed way to deliver PLTF changes. Rules:

1. Treat `pltf` changes found after a switch as work until proven otherwise. Check them before any discard: `git -C pltf status` and `git -C pltf log --oneline HEAD --not --remotes`.
2. If the work must be kept, commit it on a PLTF work branch and push it through the PLTF review flow, or stash it inside `pltf` using the naming convention in section 7.

    > This means that if any leftover changes in the submodule folder are found in the target branch, they must be committed or stashed in the PLTF repository before any actual changes from the remote `pltf` repository are applied to the target branch.

3. Never record a `pltf` change on the target branch with `git add pltf`, `git add -A`, or `git commit -a`. If `git status` shows `pltf` modified on a branch where `pltf` is regular files, stop and ask for help. Do not commit.
4. Do not discard `pltf` changes to make a switch pass until the procedure below has been followed through its step 2. If the procedure is unfamiliar, practice it on a clean clone first.
5. Adding or removing the `pltf` gitlink or `.gitmodules` is a structural change. Keep it in its own commit, and bring it into another branch only through a pull request (14.3.3) or a cherry-pick approved under 14.3.5.

#### Procedure: `sub-feat/Rv1.1.8` to `doc`

**Step 1. Record the state on the source branch.**

```bash
git switch sub-feat/Rv1.1.8
git status --short --branch
git submodule status pltf
git -C pltf status --short --branch
git -C pltf log --oneline HEAD --not --remotes
PINNED=$(git rev-parse HEAD:pltf)
echo "Pinned PLTF commit: $PINNED"
```

- `PINNED` is the gitlink on this branch. Keep the value; step 3 restores it.
- A `+` prefix in `git submodule status pltf` means the checked-out PLTF commit differs from the pin.
- The last command lists PLTF commits that are not on any remote. Each commit must either be pushed to an appropriate PLTF remote branch or intentionally preserved before leaving the source state.
- If `git -C pltf status` is empty and the log is empty, skip to step 4.

**Step 2. Keep the PLTF work and push it. Do not discard it.**

```bash
git -C pltf switch -c pacup/<short-description>
git -C pltf add -A
git -C pltf diff --cached --stat
git -C pltf commit -m "feat(pltf)[<name>]: <short subject>"
git -C pltf rev-parse HEAD
git -C pltf push -u origin pacup/<short-description>
git -C pltf branch -r --contains HEAD
```

- `switch -c` keeps uncommitted changes and makes the new branch point at the current PLTF commit. If `pltf` is detached, the commits are reachable only through this branch.
- Review `git -C pltf diff --cached --stat` before committing. Unstage unrelated or generated files with `git -C pltf restore --staged <file>`.
- Use the commit format from section 4. Commit only the PLTF change, not the μEDP gitlink.
- Write down the SHA printed by `git -C pltf rev-parse HEAD`. Step 6 needs it.
- The last command must list `origin/pacup/<short-description>`. Do not continue until it does, because the work must not exist only on this machine.
- Open the PLTF review through the PLTF flow. Do not merge it into a μEDP branch.
- A stash keeps the work on this machine only, using the section 7 naming (`git -C pltf stash push -u -m "1534 - 011026"`). It is not enough for step 6. Push the branch first.
- Finish every `git -C pltf` check before step 5. After the switch to `doc`, `pltf` is a regular path of μEDP and `git -C pltf` no longer targets the PLTF repository.
- Check ignored files too: `git -C pltf status --ignored --short`. Ignored outputs are not pushed; copy any you need before step 5.

**Step 3. Restore the pinned PLTF commit on the source branch.**

> **What this step does:** puts the `pltf` checkout back to the commit that `sub-feat/Rv1.1.8` pins, so the μEDP branch is clean before you leave it. Nothing is committed here.

```bash
git submodule update --init pltf
git status --short
git submodule status pltf
```

- `git submodule status pltf` must show no `+` and no `-` prefix.
- `git status --short` must not list `pltf`. The pin in μEDP is unchanged, so do not stage or commit anything here.
- If the update cannot find the pinned commit, stop and go back to step 1. Do not try to force the pin.

**Step 4. Confirm the target branch stores `pltf` as regular files.**

> **What this step does:** checks the target branch before switching. If `doc` still had a gitlink (`160000`), the switch would change the pin rather than the files, and the rest of this procedure would not apply. Stop and ask for help in that case.

```bash
git ls-tree doc -- pltf
git cat-file -e doc:.gitmodules 2>/dev/null && echo "doc has .gitmodules" || echo "doc has no .gitmodules"
```

- The first command should show a `040000 tree` entry, not `160000 commit`. That confirms the switch changes the path from gitlink to regular files.

**Step 5. Switch and check the target branch.**

> **What this step does:** performs the switch. This is the first point where Git compares `pltf` on both branches. Any `pltf` leftover shown here should already be saved by step 2. If a change is unexpected, stop here and do not run step 6.

```bash
git switch doc
git status --short --branch
git status --short -- pltf
```

- Do not add `-f`, `--force`, or `--discard-changes` to the switch. If it refuses, read the message and go back to step 1.
- If `git status --short -- pltf` is empty, the procedure is finished for this step.
- If it shows `M` or `??` entries under `pltf/`, do not stage or commit them. Go to step 6.

**Step 6. Only when step 5 shows `pltf` leftovers on `doc`: move them aside, then restore the `doc` files.**

> **What this step does:** moves the leftover PLTF folder out of μEDP and restores the `doc` files in its place. Nothing is deleted. It runs only when the PLTF work is on `origin`, not in a local commit or a stash.

Precondition: check the pushed branch with the SHA from step 2. Replace `<PLTF_WORK_SHA>` with that value. `--git-dir` is used because `pltf/` is no longer a PLTF repository after the switch to `doc`.

```bash
git --git-dir=.git/modules/pltf branch -r --contains <PLTF_WORK_SHA>
```

The output must list `origin/pacup/<short-description>`. If it is empty, stop. A stash-only state is not enough.

```bash
mv pltf ../pltf-leftover-<date>
git status --short -- pltf
git restore --source=HEAD --staged --worktree -- pltf
git status --short -- pltf
```

- `mv` keeps the old folder outside μEDP instead of deleting it. After moving the submodule working tree outside the superproject, do not rely on the moved working tree's `.git` indirection. Use the parent's `.git/modules/pltf` metadata or the already pushed remote branch for verification.
- `git --git-dir=.git/modules/pltf` reads PLTF metadata that stays in μEDP's `.git`, so the check still works after the move.
- After `git restore`, `git status --short -- pltf` must be empty.
- Keep `../pltf-leftover-<date>` until the PLTF review of step 2 is complete, then delete it.
- If `git restore` fails or reports a path that is not under `pltf/`, stop and ask for help. Do not use `git clean -f` or `git reset --hard`.

**Step 7. Return to the source branch.**

```bash
git switch sub-feat/Rv1.1.8
git submodule sync --recursive
git submodule update --init --recursive
git submodule status --recursive
git rev-parse HEAD:pltf
git -C pltf rev-parse HEAD
```

- The last two SHAs must match. A `+` in `git submodule status` means the pin and the checkout still differ.
- If `git submodule update` refuses because untracked files under `pltf/` would be overwritten, back them up and move them aside. Do not use `-f` to force the checkout.

### 14.7. gitsp notes for submodules

- The "Submodule & ràng buộc" tab stores its mapping in `.gitsp-submodules.json` at the repo root, in the form `{"<main-branch>": {"<submodule-path>": "<submodule-branch>"}}`. Commit this file with μEDP. Saving a rule only changes the working tree.
- gitsp reads the mapping from the current checkout before switching. The target branch's mapping is therefore whatever the current branch's file says. Keep the file identical across branches that use it, or switch only from a branch whose file already contains the target mapping.
- "Chuyển branch theo ràng buộc" requires a rule for every submodule listed on the current checkout, and refuses tracked changes in μEDP or in any submodule. It switches the main repo first, then each submodule to its required branch. It does not run `git submodule update --init`, does not read the target branch's gitlink, and only handles submodules listed before the switch. Use it only when the current and target branches both have `pltf` as a gitlink. For switches across the gitlink boundary, follow 14.2 and 14.6 manually.
- A gitsp branch rule is a branch name, not a commit pin. After switching, the submodule commit can still differ from the gitlink. Run `git submodule status --recursive` and check for `+` (see 14.2).
- An empty submodule list on a branch means that branch has no gitlink. It does not mean `pltf` is absent.

### 14.8. Submodule invariant

For every μEDP commit that represents `pltf` as a gitlink:

1. `HEAD:pltf` shall be a mode `160000` gitlink.
2. The referenced PLTF commit shall exist on an accessible PLTF remote.
3. The checked-out PLTF repository shall match the gitlink before build, test, merge, or PR verification.
4. `.gitmodules` and the gitlink shall be changed only through reviewed μEDP commits.

### 14.9. Migration `pltf` as gitlink on every branch

Target state: every branch that contains `pltf` stores it as a gitlink (mode `160000`) with a matching `.gitmodules` entry. No branch stores `pltf` as regular files (`040000 tree`). Only then do the rules in 14.2 to 14.8 and the gitsp rules in 14.7 apply the same way on every branch.

Current state (snapshot 2026-11-18, from `origin`):

| Branch | `pltf` on origin | `.gitmodules` on origin |
| --- | --- | --- |
| `sub-feat/Rv1.1.8` | gitlink `160000`, pin `225af111` | yes |
| `main`, `release`, `feat`, `fix`, `doc`, `chore`, `reft`, `integration`, `sub-feat/Rv1.1.7-bf1`, `sub-feat/Rv1.1.9` | regular tree `040000` | no |

Blockers, to be resolved before any branch is migrated:

1. **Pin per branch is undecided.** The `pltf` tree on `origin/main`, `origin/release`, and `origin/feat` is the same tree (`806d8f10`). It does not match the tree of any commit in the local PLTF repository (`.git/modules/pltf`, checked with `log --all`). A gitlink must pin a PLTF commit whose tree equals the content the branch carries today. Otherwise the migration silently changes μEDP content. If no PLTF commit matches, stop and decide with the PLTF owners which commit to pin.
2. **URL is not confirmed for all branches.** The only `.gitmodules` today (on `sub-feat/Rv1.1.8`) uses `https://github.com/The-Personal-Toolsetation/pltf.git`. Confirm this URL before copying it to other branches.
3. **Fresh clones change behaviour.** On a migrated branch, `pltf/` is empty after `git clone` until `git submodule update --init --recursive` runs (14.2, 14.8). Tell the team and CI before the first migration PR lands.

Migration rules:

1. Migrate one branch per PR. Each PR is a single commit: `.gitmodules` plus the gitlink, nothing else. Do not mix it with other changes (14.4).
2. Do not push directly to protected branches. Use the normal PR flow (section 9).
3. Do not rewrite history. Migration is a new commit on top of the existing branch.
4. Until every branch that contains `pltf` is migrated, do not merge or switch between a migrated branch and an unmigrated one without the procedure in 14.6. The mismatch between a tree and a gitlink is the source of the switch problem described in 14.6.
5. The migration order is an open decision. Record it here before starting.

Procedure for one branch (run on a work branch created from the target branch):

```bash
# 1. Confirm the PLTF commit C has the same tree as the branch's pltf tree.
git rev-parse <branch>:pltf
git -C pltf rev-parse C^{tree}          # must print the same SHA

# 2. Work branch from the target branch (naming: section 2).
git switch -c sub-chore/pltf-gitlink-<branch> <branch>

# 3. Remove pltf from the index and keep a copy of the folder for comparison.
git rm -r --cached pltf
mv pltf ../pltf-pre-migration-<branch>

# 4. Add the gitlink and .gitmodules, then check out C.
git update-index --add --cacheinfo 160000,C,pltf
git config -f .gitmodules submodule.pltf.path pltf
git config -f .gitmodules submodule.pltf.url https://github.com/The-Personal-Toolsetation/pltf.git
git add .gitmodules
git submodule update --init pltf

# 5. Verify before committing.
git ls-tree HEAD -- pltf                 # 160000 commit C
git submodule status pltf                # no + or - prefix
diff -r -x .git ../pltf-pre-migration-<branch> pltf   # must print nothing
git status --short                       # only .gitmodules and pltf

# 6. Commit only the migration, then open a PR.
git commit -m "chore(pltf)[<name>]: track pltf as submodule gitlink at <short-sha>"
```

- Step 3 moves the folder aside because `submodule update` refuses to write into a path with untracked files (section 6).
- Step 5 `diff -r` is the proof that no μEDP content changed. If it prints anything, stop, do not commit, and check the pin in step 1.
- The PR description must state: the old tree SHA, the new pin C, and the result of step 5.
- After the PR merges, delete `../pltf-pre-migration-<branch>` only once the branch is verified on a clean clone.

Post-migration invariants, checked for each branch:

- `git ls-tree <branch> -- pltf` shows `160000 commit`, and `git cat-file -e <branch>:.gitmodules` succeeds.
- No branch shows `pltf` as `040000 tree`.
- After `git submodule update --init --recursive`, `git submodule status --recursive` shows no `+`, `-`, or `U` prefix.

gitsp impact (open items, not implemented):

- gitsp's "Chuyển branch theo ràng buộc" does not run `git submodule update` after switching, and it does not read the target gitlink (14.7). Once every branch is a gitlink, add the update step to the switch flow, and check `git submodule status` afterwards.
- `.gitsp-submodules.json` must hold the same rules on every branch that uses it (14.7).
