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

## 14. Git Submodule Management Procedure

### 14.1. Purpose

This document defines a single, practical procedure for introducing and managing Git submodules in the superproject, including:

- converting an existing tracked directory into a submodule;
- switching between branches that use the same submodule;
- handling local modifications safely;
- merging and cherry-picking branches that change a submodule pin;
- reviewing pull requests that contain submodule changes;
- handling the temporary migration period where some branches still contain regular files;
- verifying the final state after migration.

`pltf` is used as the example submodule path. The same rules apply to other submodules.

---

### 14.2. Git Model and Terminology

A superproject does not store the files of a submodule directly. It stores a **gitlink** that points to one commit in the submodule repository.

| Representation | Meaning |
| --- | --- |
| `100644` | Normal tracked file |
| `040000 tree` | Normal tracked directory/tree in a commit |
| `160000 commit` | Gitlink pointing to a submodule commit |
| **Pin** | The submodule commit recorded by the superproject |

Example:

```text
feat
└── pltf 160000 -> commit A

reft
└── pltf 160000 -> commit B
```

Commit A and commit B may differ. Each superproject branch records the PLTF revision required by that branch.

#### 14.2.1. Core invariant

For every branch that uses `pltf` as a submodule:

1. `pltf` is stored as mode `160000`.
2. `.gitmodules` contains the correct path and repository URL.
3. The pinned PLTF commit exists on an accessible remote.
4. Before build, test, merge, or PR verification, the checked-out PLTF commit matches the superproject pin.

---

### 14.3. Target State

The long-term target is that every branch containing `pltf` represents it consistently as a submodule.

```text
branch A
└── pltf 160000 -> PLTF commit X

branch B
└── pltf 160000 -> PLTF commit Y
```

Avoid keeping this mixed structure permanently:

```text
branch A                     branch B
└── pltf 160000              └── pltf/
                                 ├── file1 100644
                                 └── file2 100644
```

The mixed state is supported only during migration because switching between a regular tree and a gitlink can leave working-tree content that blocks checkout.

---

### 14.4. Convert an Existing Tracked Directory to a Submodule

#### 14.4.1. Preconditions

Start from the branch to be migrated:

```bash
git checkout <target-branch>
git status
```

The superproject should not contain unrelated modifications.

Check the current `pltf` representation:

```bash
git ls-tree HEAD -- pltf
git ls-files --stage "pltf/*"
```

Before migration, the expected state is a normal tree/files, for example:

```text
040000 tree <tree-sha>    pltf
```

and individual index entries such as:

```text
100644 <blob-sha> 0    pltf/README.md
100644 <blob-sha> 0    pltf/uedp.py
```

Do **not** run `git submodule add ... pltf` while these files are still tracked. Otherwise Git may report:

```text
fatal: 'pltf' already exists in the index
```

#### 14.4.2. Confirm the PLTF revision before conversion

Before replacing the tracked directory, identify the PLTF commit that contains the intended content and make sure that commit exists on the PLTF remote.

Prefer performing this verification in a separate PLTF clone if the current `pltf/` path is still a normal superproject directory.

Useful checks:

```bash
git -C <PLTF_CLONE> fetch --all
git -C <PLTF_CLONE> branch -r --contains <PLTF_COMMIT>
git -C <PLTF_CLONE> rev-parse <PLTF_COMMIT>^{tree}
git rev-parse <target-branch>:pltf
```

If the selected PLTF commit does not represent the intended directory content, stop. Choosing an incorrect pin would change project content during migration.

#### 14.4.3. Optional safety copy

For a migration PR, keeping a temporary copy outside the superproject simplifies final content comparison:

```bash
cp -r pltf ../pltf-pre-migration-<target-branch>
```

On PowerShell:

```powershell
Copy-Item -Recurse pltf ../pltf-pre-migration-<target-branch>
```

Keep this copy only until the migration has been verified.

#### 14.4.4. Remove the regular files

Remove the old representation:

```bash
git rm -r pltf
```

Git will report the existing `pltf/*` files as deleted. This is **expected**.

The deletion means that the superproject stops tracking the files individually. It does not mean that the files are deleted from the standalone PLTF repository.

#### 14.4.5. Add the submodule

```bash
git submodule add <PLTF_REPOSITORY_URL> pltf
```

If a valid local submodule repository already exists and intentionally needs to be reused, `-f` may be used only after the old tracked entries have been removed:

```bash
git submodule add -f <PLTF_REPOSITORY_URL> pltf
```

Do not use `-f` as a substitute for fixing an existing `100644` representation.

Select the required PLTF commit:

```bash
git -C pltf fetch --all
git -C pltf checkout <PLTF_COMMIT>
git -C pltf rev-parse HEAD
```

A detached HEAD is acceptable here because this is only selecting the pin. Do not develop or create new PLTF commits while detached.

Stage the submodule metadata:

```bash
git add .gitmodules pltf
```

#### 14.4.6. Verify before committing

Check the gitlink:

```bash
git ls-files --stage pltf
git ls-tree HEAD -- pltf
```

Before the new commit is created, the index should show:

```text
160000 <PLTF_COMMIT> 0    pltf
```

If a safety copy was created, compare the actual contents:

```bash
diff -r -x .git ../pltf-pre-migration-<target-branch> pltf
```

No output means the content is equivalent.

Check the staged changes:

```bash
git status --short
```

The migration should contain only the structural changes required for the conversion, such as:

- `.gitmodules` added or updated;
- old `pltf/*` entries removed from the superproject;
- `pltf` added as mode `160000`.

#### 14.4.7. Commit the migration

Example:

```bash
git commit -m "chore(pltf): convert pltf to submodule at <short-sha>"
```

A result such as the following is normal:

```text
59 files changed, 1 insertion(+), 4859 deletions(-)
create mode 160000 pltf
delete mode 100644 pltf/README.md
...
```

Do **not** restore those deleted `100644` entries after the conversion. They are intentionally replaced by the `160000` gitlink.

Conceptually:

```text
Before
Superproject
└── pltf/
    ├── file1 100644
    ├── file2 100644
    └── ...

After
Superproject
└── pltf 160000
        |
        +--> PLTF repository @ selected commit
```

---

### 14.5. Post-Migration Verification

Run:

```bash
git status
git diff HEAD
git ls-tree HEAD -- pltf
git -C pltf rev-parse HEAD
git submodule status --recursive
git config -f .gitmodules --get-regexp '^submodule\..*'
```

Expected results:

- `git status` reports a clean working tree.
- `git diff HEAD` produces no output.
- `git ls-tree HEAD -- pltf` reports `160000 commit`.
- `git -C pltf rev-parse HEAD` matches the pin recorded by the superproject.
- `.gitmodules` contains the correct path and URL.
- `git submodule status --recursive` reports no abnormal prefix.

Prefix interpretation:

| Prefix | Meaning |
| --- | --- |
| none / space | Checked-out commit matches the recorded pin |
| `+` | Submodule is checked out at a different commit |
| `-` | Submodule is not initialized |
| `U` | Submodule conflict |

To verify that the files still exist inside PLTF:

```bash
git -C pltf ls-tree -r --name-only HEAD
```

---

### 14.6. Normal Branch Switching After Both Branches Are Submodulized

Once both source and target branches use `pltf` as mode `160000`, branch switching becomes simple.

Before switching:

```bash
git status
git -C pltf status
```

If both repositories are clean:

```bash
git switch <target-branch>
git submodule sync --recursive
git submodule update --init --recursive
git submodule status --recursive
```

Equivalent `git checkout <target-branch>` may be used instead of `git switch`.

#### 14.6.1. Why `submodule update` is required

Switching the superproject branch changes the expected gitlink but does not guarantee that the PLTF working tree is already checked out at that revision.

Example:

```text
feat -> pltf@AAA
reft -> pltf@BBB
```

After switching from `feat` to `reft`, run:

```bash
git submodule update --init --recursive
```

so that `pltf` is moved from `AAA` to the `BBB` pin required by `reft`.

---

### 14.7. Handling Local PLTF Modifications

Before switching the superproject branch, always inspect PLTF:

```bash
git -C pltf status --short --branch
git -C pltf log --oneline HEAD --not --remotes
```

#### 14.7.1. Modifications must be kept

Do not use `--force`.

Create or switch to a named PLTF branch, then commit and push the work:

```bash
git -C pltf switch -c <work-branch>
git -C pltf add -A
git -C pltf commit -m "<message>"
git -C pltf push -u origin <work-branch>
```

Alternatively, for temporary local preservation:

```bash
git -C pltf stash push -u -m "before superproject branch switch"
```

A stash is local-only. If the work must be durable or shared, push it to the PLTF remote.

#### 14.7.2. Modifications are no longer needed

If the local PLTF changes are intentionally disposable, the target pin may be enforced with:

```bash
git submodule update --init --recursive --force
```

`--force` should **not** be the default command. Use it only when the local submodule checkout may safely be replaced.

If an explicit cleanup is required, inspect first:

```bash
git -C pltf status
```

Then, only when the data is known to be disposable:

```bash
git -C pltf reset --hard
git -C pltf clean -fd
git submodule update --init --recursive --force
```

`reset --hard` and `clean -fd` permanently discard local work and untracked files. They must never be used blindly in the standard procedure.

---

### 14.8. Temporary Procedure Across the Gitlink Boundary

This section applies only during migration when one branch uses `pltf` as a submodule while another still stores `pltf` as regular files.

Example:

```text
migrated branch
└── pltf 160000

legacy branch
└── pltf/ 040000 tree
```

This mixed state is the main source of errors such as:

```text
The following untracked working tree files would be overwritten by checkout
```

#### 14.8.1. Before switching

Inspect and protect PLTF work:

```bash
git status
git -C pltf status --short --branch
git -C pltf log --oneline HEAD --not --remotes
```

Commit/push or stash anything that must be preserved.

Confirm how the target branch represents `pltf`:

```bash
git ls-tree <target-branch> -- pltf
```

Interpretation:

```text
160000 commit ... pltf    -> target uses a submodule
040000 tree ... pltf      -> target uses regular files
```

#### 14.8.2. From a submodule branch to a regular-tree branch

Do not force the branch switch if Git reports files that would be overwritten.

After PLTF work has been safely preserved, deinitialize the submodule working tree:

```bash
git submodule deinit -f -- pltf
```

If the physical folder remains and is known to contain no unsaved work, remove or move it outside the superproject.

PowerShell example:

```powershell
Move-Item pltf ../pltf-leftover
```

or, only when deletion is safe:

```powershell
Remove-Item -Recurse -Force pltf
```

Then switch:

```bash
git switch <regular-tree-branch>
git restore --source=HEAD --staged --worktree -- pltf
```

The target branch must not accidentally commit residual submodule content.

#### 14.8.3. From a regular-tree branch to a submodule branch

Ensure the regular-tree branch is clean first:

```bash
git status
```

Move or remove the existing `pltf/` working directory only after confirming that its contents are safely tracked or backed up.

Then:

```bash
git switch <submodule-branch>
git submodule sync --recursive
git submodule update --init --recursive
git submodule status --recursive
```

Do not use `git clean -f` or `git reset --hard` as a generic solution to crossing this boundary.

#### 14.8.4. Migration recommendation

This entire section should become unnecessary once every long-lived branch represents `pltf` as mode `160000`. Therefore, migrating the remaining branches is preferred over maintaining a complicated permanent boundary-switch procedure.

---

### 14.9. Merge Procedure

#### 14.9.1. Preconditions

Before merging branches that both use PLTF:

1. Both branches should represent `pltf` as mode `160000`.
2. Both branches should have compatible `.gitmodules` entries.
3. Any repository-specific branch mapping file, such as `.gitsp-submodules.json`, must be reviewed separately from the gitlink.
4. All PLTF commits that may become the resulting pin must already exist on the PLTF remote.

Inspect the pin difference before merge:

```bash
git diff --submodule=log <target>...<source>
```

#### 14.9.2. Case A: same PLTF pin

```text
target -> pltf@AAA
source -> pltf@AAA
```

No gitlink resolution is required. Resolve only accompanying configuration files if they differ.

#### 14.9.3. Case B: one branch advances the other

Example:

```text
base   -> AAA
target -> AAA
source -> BBB
```

If `BBB` is the intended compatible PLTF revision, the resulting superproject may pin `BBB`.

Prefer reviewing a deliberate pin update rather than allowing an incidental pin change to be hidden inside unrelated changes.

#### 14.9.4. Case C: both branches move PLTF independently

Example:

```text
base   -> AAA
target -> BBB
source -> CCC
```

Do not arbitrarily choose one gitlink.

Resolve the actual PLTF changes in the PLTF repository first:

```bash
cd pltf
git fetch --all
git switch <integration-branch>
git merge <other-pltf-branch>
# resolve conflicts and test
git push
cd ..
```

Then point the superproject at the resulting PLTF integration commit:

```bash
git add pltf
```

Update any associated mapping/configuration file as required and verify:

```bash
git submodule update --init --recursive
git submodule status --recursive
```

---

### 14.10. Cherry-Pick Rules

A cherry-pick that modifies `pltf` may also change the gitlink.

Before cherry-picking:

```bash
git show --submodule=log <commit>
```

If the commit changes the PLTF pin:

1. confirm that the referenced PLTF commit exists on the remote;
2. cherry-pick the commit;
3. synchronize and update the submodule;
4. verify the resulting pin.

```bash
git cherry-pick <commit>
git submodule sync --recursive
git submodule update --init --recursive
git submodule status --recursive
```

Avoid cherry-picking structural migration commits into branches that are intentionally not ready for submodules.

---

### 14.11. Pull Request Policy

A PR that changes the `pltf` gitlink should clearly state:

- old PLTF commit;
- new PLTF commit;
- PLTF branch/tag when applicable;
- reason for the pin update;
- verification performed.

Review the difference with:

```bash
git diff --submodule=log <target>...<source>
```

For a pin-only update, keep the PR focused. Do not mix unrelated superproject modifications unless they are inseparable from the PLTF update.

For a migration PR, mass deletion of the former `pltf/*` files is expected. Reviewers should verify:

```text
old representation: 040000 tree / individual 100644 files
new representation: 160000 gitlink
```

and verify that the selected PLTF commit contains the intended content.

CI should initialize submodules from a clean checkout:

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

CI credentials must be able to access the PLTF repository and the pinned commit.

---

### 14.12. `.gitsp-submodules.json` and Branch Mapping

If the project uses `.gitsp-submodules.json`, treat it as **branch-mapping metadata**, not as the authoritative commit pin.

Example:

```json
{
  "feat": {
    "pltf": "main"
  },
  "reft": {
    "pltf": "main"
  }
}
```

Important distinction:

```text
.gitsp-submodules.json -> desired PLTF branch relationship
superproject gitlink   -> exact PLTF commit used by this superproject commit
```

The gitlink remains the source of truth for reproducible checkout.

After any tool-driven branch switch, always verify:

```bash
git submodule status --recursive
```

A branch mapping alone does not guarantee that the checked-out PLTF commit matches the gitlink.

If `.gitsp-submodules.json` causes merge conflicts, resolve the combined mapping deliberately and separately from the gitlink decision.

---

### 14.13. Standard Operational Checklist

| Operation | Before | Required after |
| --- | --- | --- |
| Switch between two submodule branches | `git status`; `git -C pltf status` | `git submodule sync --recursive`; `git submodule update --init --recursive`; status check |
| Discard obsolete PLTF modifications | Confirm changes are disposable | `git submodule update --init --recursive --force` |
| Merge | Inspect `git diff --submodule=log`; ensure PLTF pins exist remotely | Resolve pin intentionally; update; test |
| Cherry-pick pin change | Verify referenced PLTF commit exists remotely | Update and verify submodule |
| Update PLTF pin | PLTF commit already pushed | Focused PR and CI verification |
| Convert regular `pltf/` to submodule | Confirm correct PLTF commit and content | Verify `160000`, SHA match, clean tree |
| Cross gitlink/regular-tree boundary during migration | Preserve PLTF work first | Remove/move residual working tree safely and restore target representation |

---

### 14.14. Commands for Diagnosis

#### 14.14.1. Determine how the current branch stores PLTF

```bash
git ls-tree HEAD -- pltf
```

Expected submodule:

```text
160000 commit <sha>    pltf
```

Regular directory:

```text
040000 tree <sha>    pltf
```

#### 14.14.2. Determine which files the index tracks

```bash
git ls-files --stage pltf
git ls-files --stage "pltf/*"
```

#### 14.14.3. Show the pin recorded by the current superproject commit

```bash
git rev-parse HEAD:pltf
```

#### 14.14.4. Show the commit currently checked out inside PLTF

```bash
git -C pltf rev-parse HEAD
```

These two SHA values should match before build/test/PR verification.

#### 14.14.5. Inspect submodule differences between branches

```bash
git diff --submodule=log <target>...<source>
```

#### 14.14.6. Detect local PLTF work

```bash
git -C pltf status --short --branch
git -C pltf log --oneline HEAD --not --remotes
```

---

### 14.15. Practices to Avoid

Do not use the following as standard workflow:

#### 14.15.1. Force-checkout to bypass conflicting files

```bash
git checkout -f <branch>
```

This hides the underlying working-tree ownership problem and may discard useful changes.

#### 14.15.2. Restore old `pltf/*` files after conversion

After successfully converting to mode `160000`, restoring the former `100644` files defeats the migration.

#### 14.15.3. Blind cleanup

```bash
git reset --hard
git clean -fd
```

These commands can permanently delete work. Use them only after explicitly determining that the affected data is disposable.

#### 14.15.4. Blind submodule force update

```bash
git submodule update --init --recursive --force
```

Use `--force` only when local submodule modifications are intentionally no longer needed.

#### 14.15.5. Treat a branch mapping as the pin

A PLTF branch name in `.gitsp-submodules.json` does not replace the superproject gitlink. Reproducibility comes from the exact `160000` commit.

---

### 14.16. Final Recommended Workflow

#### 14.16.1. One-time migration

```text
regular pltf/ files
        |
        | verify matching PLTF commit
        v
remove old tracked files
        |
        v
add pltf submodule
        |
        v
checkout required PLTF commit
        |
        v
verify content + 160000 gitlink
        |
        v
commit + PR
```

#### 14.16.2. Daily development after migration

```bash
# Before switching
git status
git -C pltf status

# Switch superproject branch
git switch <target-branch>

# Synchronize submodule
git submodule sync --recursive
git submodule update --init --recursive

# Verify
git submodule status --recursive
```

If obsolete local PLTF modifications are intentionally discarded:

```bash
git submodule update --init --recursive --force
```

### 14.16.3. Final principle

A stable submodule workflow comes from keeping a clear ownership boundary:

```text
Superproject owns:
- .gitmodules
- pltf gitlink (160000)
- optional branch-mapping policy

PLTF repository owns:
- PLTF source files
- PLTF commits and branches
```

Once all relevant branches follow this structure, normal switching and merging no longer requires deleting and restoring dozens of `pltf` files. The superproject changes only the pin, while the submodule repository remains responsible for its own source and history.
