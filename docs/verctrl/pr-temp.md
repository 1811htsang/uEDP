# PR-[PR_ID] - [TASK/ISSUE_ID] - [DDMMYY]

<!--
Example:
# PR-07 - 2158 - 061126

PR_ID:
- Sequential project PR number: PR-07, PR-08, PR-09, ...

TASK/ISSUE_ID:
- Related issue, task, ticket, or work-item ID.
- Use N/A if no tracked task exists.

DDMMYY:
- Date of PR creation.
-->

## Summary

<!--
Briefly describe:
- What this PR changes.
- Why the change is needed.
- Expected outcome after merging.
-->

This pull request introduces [brief description of the change and its purpose].

---

## Related Issue or Task

<!--
Reference the related issue, task, ticket, requirement, or work item.
-->

- Issue/Task: [ID or N/A]
- Related requirement/document: [reference or N/A]
- Target release/version: [version or N/A]

---

## Content

This pull request contains the following changes:

### [Change Category 1]

<!--
Examples:
- Feature Implementation
- Bug Fix
- Refactoring
- Build System
- Documentation
- Configuration
- Testing
-->

- [Change 1]
- [Change 2]
- [Change 3]

### [Change Category 2]

- [Change 1]
- [Change 2]

<!-- Add or remove subsections depending on the PR content. -->

---

## Change Type

<!-- Check all that apply. -->

- [ ] `feat` - New feature or functionality
- [ ] `fix` - Bug fix
- [ ] `reft` - Code refactoring without intended behavior change
- [ ] `docs` - Documentation change
- [ ] `chore` - Maintenance, configuration, tooling, or housekeeping
- [ ] `release` - Release preparation or stabilization
- [ ] `hotfix` - Fix for an already released version
- [ ] Other: [description]

---

## Source and Target Branch

- Source branch: `<source-branch>`
- Target branch: `<target-branch>`

<!--
Examples:

Development:
feat/<task> -> feat/integration
fix/<task>  -> integration

Release stabilization:
fix/<task> -> release/<version>

Released software:
hotfix/<task> -> main
-->

---

## Test Plan

<!--
Describe how the changes were verified.
Include commands, environments, test cases, or manual verification when applicable.
-->

### Test Environment

- Platform/OS: [e.g. Linux / Windows / N/A]
- Build/Configuration: [configuration or N/A]
- Version/Branch: [version or branch]

### Verification

- [ ] Project builds successfully
- [ ] Existing tests pass
- [ ] New/modified functionality has been tested
- [ ] Regression impact has been considered
- [ ] Documentation has been verified, if applicable

### Test Results

<!--
Provide a short summary or evidence.
Example:
- `cmake --build build`: PASS
- Unit tests: 128/128 PASS
- Manual PSE publish/subscribe test: PASS
-->

[TBD]

---

## Impact Assessment

<!--
Describe affected components and compatibility impact.
-->

- Affected component(s): [component(s) or N/A]
- API/interface change: Yes / No
- Configuration change: Yes / No
- Backward compatibility impact: Yes / No
- Documentation update required: Yes / No

### Breaking Changes

None.

<!--
If applicable, replace "None." with:
- Description of breaking change
- Migration/update instructions
-->

---

## Checklist

### Code

- [ ] Code follows project coding conventions
- [ ] No unnecessary/debug code is included
- [ ] Changes are limited to the intended scope
- [ ] New code has appropriate error handling

### Testing

- [ ] Relevant tests have been executed
- [ ] Existing functionality is not unintentionally affected
- [ ] Bug fixes include verification of the original defect, if applicable

### Documentation

- [ ] Documentation has been updated where required
- [ ] New behavior/API/configuration is documented
- [ ] Comments and examples are updated where applicable

### Git / PR

- [ ] Source branch follows the project branching convention
- [ ] Commit messages are clear and meaningful
- [ ] PR contains no unrelated changes
- [ ] Target branch is correct
- [ ] PR is ready for review

---

## Additional Notes

<!--
Optional:
- Known limitations
- Follow-up tasks
- Reviewer notes
- Migration considerations
- Dependencies on another PR
-->

None.
